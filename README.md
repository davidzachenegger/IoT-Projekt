# IoT Überwachungssystem mit ESP32 (ESP-NOW)

## Gas- und Bewegungserkennung mit Web-Dashboard und OLED Anzeige

Verfasser: **Nazar Tymoshenko, David [Nachname]**  
Datum: **26.05.2026**

---

## 1. Einführung

Im Rahmen dieses Projekts wurde ein IoT-System entwickelt, das Umweltdaten in Echtzeit erfasst und drahtlos überträgt. Dazu werden Gas- und Bewegungsdaten mit Sensoren erfasst und zwischen zwei ESP32-Modulen über ESP-NOW übertragen. Ziel ist eine einfache, stabile und lokal unabhängige Überwachungslösung mit visueller Darstellung.

---

## 2. Projektbeschreibung

Es wurde ein System aus zwei ESP32-Mikrocontrollern umgesetzt. Ein Sender erfasst Gas- und Bewegungsdaten und überträgt diese drahtlos an einen Receiver. Der Receiver verarbeitet die Daten, zeigt sie auf einem OLED-Display an und stellt sie zusätzlich über einen Webserver mit Live-Diagramm im Browser dar.

---

## 3. Theorie

Der ESP32 ist ein Mikrocontroller mit integrierter WLAN-Funktion, der sich besonders für IoT-Anwendungen eignet. Für die Kommunikation wird ESP-NOW verwendet, ein Protokoll, das eine direkte Verbindung zwischen zwei ESP-Geräten ohne Router ermöglicht. Dadurch werden Daten schnell und energieeffizient übertragen.

Zur Datenerfassung werden zwei Sensoren verwendet:
- ein Gassensor zur Messung der Luftqualität
- ein PIR-Bewegungssensor zur Erkennung von Bewegungen

Die Daten werden anschließend verarbeitet und sowohl lokal als auch über einen Webserver visualisiert.

---

## 4. Arbeitsschritt

### 4.1 Systemaufbau

Das System besteht aus:
- ESP32 Sender (Sensoren)
- ESP32 Receiver (Anzeige + Webserver)

Datenfluss:

Sender → ESP-NOW → Receiver → OLED + Web + LED

---

## 4.2 Sender (ESP32)

```cpp
#include <WiFi.h>
#include <esp_now.h>

#define GAS_PIN     34
#define MOTION_PIN  32

uint8_t receiverMac[] = {0x00,0x70,0x07,0x1C,0xED,0xED};

typedef struct SensorData {
  int gas;
  bool motion;
} SensorData;

SensorData data;

void onSend(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
}

void setup() {
  Serial.begin(115200);

  pinMode(GAS_PIN, INPUT);
  pinMode(MOTION_PIN, INPUT);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  esp_now_init();
  esp_now_register_send_cb(onSend);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  esp_now_add_peer(&peerInfo);

  Serial.println("Sender ready");
}

void loop() {
  data.gas = analogRead(GAS_PIN);
  data.motion = digitalRead(MOTION_PIN);

  esp_now_send(receiverMac, (uint8_t*)&data, sizeof(data));

  delay(1500);
}
