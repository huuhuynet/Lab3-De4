
#include <WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <math.h>
#include <string.h>

// ================== KHAI BAO GPIO ==================

#define TRIG_PIN       5
#define ECHO_PIN       18
#define TEMP_PIN       4
#define PUMP_PIN       25
#define VALVE_PIN      26

// ================== WIFI ==================

const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ================== MQTT ==================

const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

// Topic MQTT
const char* topic_sensor = "lab3de4/aquarium/demo01/sensor";
const char* topic_mode   = "lab3de4/aquarium/demo01/mode/set";
const char* topic_pump   = "lab3de4/aquarium/demo01/pump/set";
const char* topic_valve  = "lab3de4/aquarium/demo01/valve/set";
const char* topic_state  = "lab3de4/aquarium/demo01/state";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// ================== CAM BIEN NHIET DO ==================

OneWire oneWire(TEMP_PIN);
DallasTemperature tempSensor(&oneWire);

// ================== CAU TRUC DU LIEU ==================

struct SensorData {
  float distance;
  float temperature;
};

// Trang thai thiet bi gui ve Website
struct DeviceState {
  bool autoMode;
  bool pumpOn;
  int pumpDuty;
  bool valveOpen;
};

// ================== FREERTOS QUEUE ==================

QueueHandle_t dataQueue;
QueueHandle_t dataAlert;
QueueHandle_t stateQueue;

// ================== BIEN DIEU KHIEN ==================

portMUX_TYPE commandMux = portMUX_INITIALIZER_UNLOCKED;

bool autoMode = true;
bool manualPump = false;
bool manualValve = false;

// ================== TRANG THAI MAY BOM ==================

enum AlertAction {
  ACTION_NONE,
  ACTION_PUMP_50,
  ACTION_PUMP_100,
  ACTION_DRAIN
};

AlertAction currentAction = ACTION_NONE;

unsigned long actionEndTime = 0;

// ================== CAU HINH PWM ==================

const int PWM_FREQ = 5000;
const int PWM_RESOLUTION = 8;
const int PWM_CHANNEL = 0;

void setupPumpPWM() {

#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttach(PUMP_PIN, PWM_FREQ, PWM_RESOLUTION);
#else
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(PUMP_PIN, PWM_CHANNEL);
#endif

}

void setPump(int duty) {

  duty = constrain(duty, 0, 255);

#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(PUMP_PIN, duty);
#else
  ledcWrite(PWM_CHANNEL, duty);
#endif

}

// ================== DOC KHOANG CACH ==================

float readDistance() {

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) {
    return -1;
  }

  // Chuyen khoang cach tu cm sang m
  float distance = duration * 0.0343 / 2.0 / 100.0;

  return distance;
}

// ================== MQTT CALLBACK ==================

void mqttCallback(char* topic, byte* payload, unsigned int length) {

  char message[32];

  if (length >= sizeof(message)) {
    return;
  }

  memcpy(message, payload, length);
  message[length] = '\0';

  portENTER_CRITICAL(&commandMux);

  // Nhan lenh chuyen che do
  if (strcmp(topic, topic_mode) == 0) {

    if (strcmp(message, "AUTO") == 0) {

      autoMode = true;

      // Xoa cac lenh dieu khien thu cong
      manualPump = false;
      manualValve = false;
    }
    else if (strcmp(message, "MANUAL") == 0) {

      autoMode = false;

      // Vao MANUAL, tat ca thiet bi mac dinh OFF
      manualPump = false;
      manualValve = false;
    }
  }

  // Nhan lenh dieu khien may bom
  else if (strcmp(topic, topic_pump) == 0) {

    if (strcmp(message, "ON") == 0) {

      manualPump = true;

      // Bat bom thi dong van xa
      manualValve = false;
    }
    else if (strcmp(message, "OFF") == 0) {

      manualPump = false;
    }
  }

  // Nhan lenh dieu khien van xa
  else if (strcmp(topic, topic_valve) == 0) {

    if (strcmp(message, "ON") == 0) {

      manualValve = true;

      // Mo van thi tat bom
      manualPump = false;
    }
    else if (strcmp(message, "OFF") == 0) {

      manualValve = false;
    }
  }

  portEXIT_CRITICAL(&commandMux);

}

// ================== KET NOI MQTT ==================

void reconnectMQTT() {

  if (mqttClient.connected()) {
    return;
  }

  String clientId = "ESP32-Aquarium-";
  clientId += String((uint32_t)ESP.getEfuseMac(), HEX);

  if (mqttClient.connect(clientId.c_str())) {

    Serial.println("MQTT connected!");

    mqttClient.subscribe(topic_mode);
    mqttClient.subscribe(topic_pump);
    mqttClient.subscribe(topic_valve);

  }
  else {

    // Tranh thu ket noi lien tuc qua nhanh
    vTaskDelay(pdMS_TO_TICKS(2000));
  }

}

// ==================================================
// TASK 1: DOC DU LIEU CAM BIEN - CORE 1
// ==================================================

void TaskReadData(void *pvParameters) {

  SensorData data;

  for (;;) {

    // Doc khoang cach HC-SR04
    data.distance = readDistance();

    // Doc nhiet do DS18B20
    tempSensor.requestTemperatures();
    data.temperature = tempSensor.getTempCByIndex(0);

    // Hien thi du lieu tren Serial Monitor
    Serial.println("===== DU LIEU CAM BIEN =====");

    if (data.distance >= 0) {

      Serial.print("Khoang cach: ");
      Serial.print(data.distance);
      Serial.println(" m");

    }
    else {

      Serial.println("Loi cam bien khoang cach!");

    }

    if (data.temperature != DEVICE_DISCONNECTED_C &&
        !isnan(data.temperature)) {

      Serial.print("Nhiet do: ");
      Serial.print(data.temperature);
      Serial.println(" *C");

    }
    else {

      Serial.println("Loi cam bien nhiet do!");

    }

    // Gui du lieu cam bien vao dataQueue
    xQueueSend(dataQueue, &data, pdMS_TO_TICKS(100));

    // Gui khoang cach moi nhat vao dataAlert
    xQueueOverwrite(dataAlert, &data.distance);

    vTaskDelay(pdMS_TO_TICKS(2000));
  }

}

// ==================================================
// TASK 2: CANH BAO VA DIEU KHIEN - CORE 1
// ==================================================

void TaskAlert(void *pvParameters) {

  float distance = -1;

  bool mode;
  bool pumpRequest;
  bool valveRequest;

  int currentZone = 0;
  int previousZone = -1;

  bool previousMode = true;

  int pumpDuty = 0;
  bool valveOpen = false;

  unsigned long now;

  DeviceState currentState;
  DeviceState lastState;

  bool stateInitialized = false;

  for (;;) {

    // Nhan khoang cach moi nhat tu dataAlert
    if (xQueueReceive(dataAlert, &distance,
                      pdMS_TO_TICKS(50)) == pdTRUE) {

      // Xac dinh vung muc nuoc
      currentZone = 0;

      if (distance >= 1.0 && distance < 1.5) {
        currentZone = 1;
      }
      else if (distance >= 1.5 && distance <= 2.0) {
        currentZone = 2;
      }
      else if (distance >= 0 && distance < 0.5) {
        currentZone = 3;
      }
    }

    // Doc che do va lenh dieu khien
    portENTER_CRITICAL(&commandMux);

    mode = autoMode;
    pumpRequest = manualPump;
    valveRequest = manualValve;

    portEXIT_CRITICAL(&commandMux);

    now = millis();

    // ================== CHE DO MANUAL ==================

    if (!mode) {

      currentAction = ACTION_NONE;

      // Khong cho bom va van xa hoat dong dong thoi
      if (valveRequest) {

        pumpDuty = 0;
        valveOpen = true;

      }
      else {

        valveOpen = false;

        if (pumpRequest) {
          pumpDuty = 255;
        }
        else {
          pumpDuty = 0;
        }
      }
    }

    // ================== CHE DO AUTO ==================

    else {

      // Khi chuyen tu MANUAL sang AUTO,
      // kiem tra lai vung canh bao hien tai
      if (!previousMode) {
        previousZone = -1;
      }

      // Chi kich hoat khi vua vao vung canh bao
      if (currentZone != previousZone) {

        previousZone = currentZone;

        if (currentAction == ACTION_NONE) {

          if (currentZone == 1) {

            currentAction = ACTION_PUMP_50;
            actionEndTime = now + 10000;

            Serial.println(
              "Canh bao: Bom 50% trong 10 giay"
            );

          }

          else if (currentZone == 2) {

            currentAction = ACTION_PUMP_100;
            actionEndTime = now + 20000;

            Serial.println(
              "Canh bao: Bom 100% trong 20 giay"
            );

          }

          else if (currentZone == 3) {

            currentAction = ACTION_DRAIN;
            actionEndTime = now + 10000;

            Serial.println(
              "Canh bao: Mo van xa trong 10 giay"
            );

          }
        }
      }

      // Kiem tra thoi gian thuc hien hanh dong
      if (currentAction != ACTION_NONE &&
          (int32_t)(now - actionEndTime) >= 0) {

        currentAction = ACTION_NONE;

        Serial.println(
          "Ket thuc hanh dong canh bao"
        );

      }

      // Dieu khien theo hanh dong hien tai
      switch (currentAction) {

        case ACTION_PUMP_50:

          pumpDuty = 128;
          valveOpen = false;

          break;

        case ACTION_PUMP_100:

          pumpDuty = 255;
          valveOpen = false;

          break;

        case ACTION_DRAIN:

          pumpDuty = 0;
          valveOpen = true;

          break;

        default:

          pumpDuty = 0;
          valveOpen = false;

          break;
      }
    }

    previousMode = mode;

    // ================== CAP NHAT GPIO ==================

    setPump(pumpDuty);

    digitalWrite(
      VALVE_PIN,
      valveOpen ? HIGH : LOW
    );

    // ================== GUI TRANG THAI MQTT ==================

    currentState.autoMode = mode;
    currentState.pumpOn = (pumpDuty > 0);
    currentState.pumpDuty = pumpDuty;
    currentState.valveOpen = valveOpen;

    // Chi gui khi trang thai thay doi
    bool stateChanged = !stateInitialized ||
      currentState.autoMode != lastState.autoMode ||
      currentState.pumpOn != lastState.pumpOn ||
      currentState.pumpDuty != lastState.pumpDuty ||
      currentState.valveOpen != lastState.valveOpen;

    if (stateChanged) {

      xQueueOverwrite(stateQueue, &currentState);

      lastState = currentState;
      stateInitialized = true;

    }

    vTaskDelay(pdMS_TO_TICKS(50));
  }

}

// ==================================================
// TASK 3: TRUYEN DU LIEU MQTT - CORE 0
// ==================================================

void TaskTransmitData(void *pvParameters) {

  SensorData receivedData;
  DeviceState stateData;

  char payload[128];
  char statePayload[128];

  // Cau hinh MQTT
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(mqttCallback);

  // Ket noi WiFi
  WiFi.begin(ssid, password);

  Serial.println("Dang ket noi WiFi...");

  while (WiFi.status() != WL_CONNECTED) {

    vTaskDelay(pdMS_TO_TICKS(500));

  }

  Serial.println("WiFi connected!");
  Serial.println(WiFi.localIP());

  for (;;) {

    // Kiem tra ket noi WiFi
    if (WiFi.status() != WL_CONNECTED) {

      WiFi.reconnect();

      vTaskDelay(pdMS_TO_TICKS(500));
      continue;

    }

    // Ket noi lai MQTT khi mat ket noi
    if (!mqttClient.connected()) {

      reconnectMQTT();

    }

    // Xu ly cac ban tin MQTT den
    if (mqttClient.connected()) {

      mqttClient.loop();

      // Nhan trang thai thiet bi tu stateQueue
      if (xQueueReceive(stateQueue, &stateData, 0) == pdTRUE) {

        snprintf(
          statePayload,
          sizeof(statePayload),
          "{\"mode\":\"%s\",\"pump\":%s,"
          "\"pumpDuty\":%d,\"valve\":%s}",
          stateData.autoMode ? "AUTO" : "MANUAL",
          stateData.pumpOn ? "true" : "false",
          stateData.pumpDuty,
          stateData.valveOpen ? "true" : "false"
        );

        // Gui trang thai va luu retained tren MQTT Broker
        mqttClient.publish(
          topic_state,
          statePayload,
          true
        );

        Serial.print("MQTT State: ");
        Serial.println(statePayload);

      }
    }

    // Nhan du lieu cam bien tu dataQueue
    if (xQueueReceive(dataQueue, &receivedData,
                      pdMS_TO_TICKS(100)) == pdTRUE) {

      char distanceText[20];
      char temperatureText[20];

      // Xu ly gia tri khoang cach
      if (receivedData.distance >= 0) {

        snprintf(
          distanceText,
          sizeof(distanceText),
          "%.2f",
          receivedData.distance
        );

      }
      else {

        strcpy(distanceText, "null");

      }

      // Xu ly gia tri nhiet do
      if (receivedData.temperature != DEVICE_DISCONNECTED_C &&
          !isnan(receivedData.temperature)) {

        snprintf(
          temperatureText,
          sizeof(temperatureText),
          "%.2f",
          receivedData.temperature
        );

      }
      else {

        strcpy(temperatureText, "null");

      }

      // Tao ban tin JSON
      snprintf(
        payload,
        sizeof(payload),
        "{\"distance\":%s,\"temperature\":%s}",
        distanceText,
        temperatureText
      );

      // Gui du lieu len MQTT Broker
      if (mqttClient.connected()) {

        mqttClient.publish(topic_sensor, payload);

        Serial.print("MQTT Publish: ");
        Serial.println(payload);

      }
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }

}

// ================== SETUP ==================

void setup() {

  Serial.begin(115200);

  // Cau hinh GPIO
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(VALVE_PIN, OUTPUT);

  digitalWrite(TRIG_PIN, LOW);
  digitalWrite(VALVE_PIN, LOW);

  // Cau hinh PWM cho may bom
  setupPumpPWM();
  setPump(0);

  // Khoi tao cam bien nhiet do
  tempSensor.begin();
  tempSensor.setResolution(10);

  // Tao Queue du lieu cam bien
  dataQueue = xQueueCreate(
    5,
    sizeof(SensorData)
  );

  // Tao Queue khoang cach canh bao
  dataAlert = xQueueCreate(
    1,
    sizeof(float)
  );

  // Tao Queue trang thai thiet bi
  stateQueue = xQueueCreate(
    1,
    sizeof(DeviceState)
  );

  // Kiem tra Queue
  if (dataQueue == NULL ||
      dataAlert == NULL ||
      stateQueue == NULL) {

    Serial.println("Loi tao Queue!");

    while (true) {
      delay(1000);
    }
  }

  // Tao Task doc du lieu - Core 1
  xTaskCreatePinnedToCore(
    TaskReadData,
    "TaskReadData",
    4096,
    NULL,
    2,
    NULL,
    1
  );

  // Tao Task canh bao va dieu khien - Core 1
  xTaskCreatePinnedToCore(
    TaskAlert,
    "TaskAlert",
    4096,
    NULL,
    2,
    NULL,
    1
  );

  // Tao Task truyen du lieu MQTT - Core 0
  xTaskCreatePinnedToCore(
    TaskTransmitData,
    "TaskTransmitData",
    8192,
    NULL,
    1,
    NULL,
    0
  );

  Serial.println("He thong da khoi tao!");

}

// ================== LOOP ==================

void loop() {

  // Cac cong viec duoc xu ly boi FreeRTOS Task
  vTaskDelay(pdMS_TO_TICKS(1000));

}
