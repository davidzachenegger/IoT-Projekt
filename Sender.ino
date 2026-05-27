#include <WiFi.h>
#include <esp_now.h>

/* ===== PINS ===== */
#define GAS_PIN     34
#define MOTION_PIN  32

/* ===== RECEIVER MAC ===== */
uint8_t receiverMac[] = {0x00,0x70,0x07,0x1C,0xED,0xED};

/* ===== DATA ===== */
typedef struct SensorData {
  int gas;
  bool motion;
} SensorData;

SensorData data;

/* ===== SEND CALLBACK (ESP32 CORE 3.x) ===== */
void onSend(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  Serial.print("Send status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
}

/* ===== SETUP ===== */
void setup() {
  Serial.begin(115200);

  pinMode(GAS_PIN, INPUT);
  pinMode(MOTION_PIN, INPUT);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_send_cb(onSend);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMac, 6);
  peerInfo.channel = 0;     // ВАЖНО: авто
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  Serial.println("Sender ready");
}

/* ===== LOOP ===== */
void loop() {
  data.gas = analogRead(GAS_PIN);
  data.motion = digitalRead(MOTION_PIN);

  esp_err_t result = esp_now_send(receiverMac,
                                  (uint8_t *)&data,
                                  sizeof(data));

  if (result != ESP_OK) {
    Serial.println("Send error");
  }

  delay(1500);
}