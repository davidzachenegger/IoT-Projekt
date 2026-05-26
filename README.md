# IoT Überwachungssystem mit ESP32 (ESP-NOW)

## Gas- und Bewegungserkennung mit Web-Dashboard und OLED Anzeige

Verfasser: **Nazar Tymoshenko, David [Nachname]**  
Datum: **26.05.2026**

---

## 1. Einführung

Im Rahmen dieses Projekts wird ein IoT-System mit zwei ESP32-Modulen zur Erfassung und Übertragung von Sensordaten entwickelt. Dabei werden Gas- und Bewegungsdaten mittels Sensoren erfasst, drahtlos über ESP-NOW übertragen und in einer Webanwendung visualisiert. Das System wird zur Demonstration moderner IoT-Kommunikation, Datenverarbeitung und Echtzeit-Visualisierung eingesetzt.

---

## 2. Projektbeschreibung

Das entwickelte Projekt ist ein IoT-System auf Basis von zwei ESP32-Modulen zur Erfassung von Gas- und Bewegungsdaten. Die Sensordaten werden drahtlos über ESP-NOW übertragen, verarbeitet und in einem Webinterface sowie optional in einer Datenbank dargestellt. Ziel ist eine einfache, stabile und echtzeitfähige Visualisierung der Messwerte.

---

## 3. Theorie

Für die Umsetzung des Projekts werden grundlegende Kenntnisse im Bereich der Mikrocontroller-Technik und der Sensorik benötigt. Der ESP32 wird als zentrale Steuer- und Kommunikationseinheit eingesetzt, da er über integriertes WLAN verfügt und sowohl analoge als auch digitale Sensoren auslesen kann. Zusätzlich wird das ESP-NOW-Protokoll verwendet, welches eine direkte, stromsparende und internetunabhängige Kommunikation zwischen mehreren ESP-Geräten ermöglicht.

Zur Datenerfassung werden ein PIR-Bewegungssensor sowie ein MQ-Gassensor eingesetzt. Der PIR-Sensor erkennt Bewegungen über Infrarotänderungen und liefert ein digitales Signal, während der MQ-Sensor Gaswerte als analoge Messgröße erfasst. Die erfassten Daten werden anschließend verarbeitet, gefiltert und zeitlich mit einem Zeitstempel versehen.

Für die Visualisierung wird ein Webserver auf dem ESP32 eingesetzt, der eine Webseite im lokalen Netzwerk bereitstellt. Zusätzlich kann eine Speicherung der Daten in einer MySQL-Datenbank erfolgen, um historische Messwerte auszuwerten.

Zentrale Fragestellungen des Projekts sind: Wie zuverlässig können Sensordaten drahtlos ohne Internet übertragen werden? Wie können Messwerte sinnvoll gefiltert und dargestellt werden? Und wie kann ein stabiles System aufgebaut werden, das mehrere Komponenten (Sensoren, Kommunikation, Webinterface und Datenbank) miteinander verbindet?

---

## 4. Arbeitsschritt

Zuerst wird die Hardware aufgebaut, indem der ESP32 mit dem PIR-Bewegungssensor und dem MQ-Gassensor verbunden wird. Der PIR-Sensor wird an einen digitalen GPIO-Pin angeschlossen, während der MQ-Sensor über einen analogen Eingang (ADC) mit dem ESP32 verbunden wird. Anschließend wird geprüft, ob beide Sensoren korrekte Werte liefern, indem die Messdaten im seriellen Monitor kontrolliert werden.

Im nächsten Schritt wird die ESP-NOW-Kommunikation eingerichtet. Dazu wird ein ESP32 als Sender und ein zweiter ESP32 als Empfänger konfiguriert. Beide Geräte werden im gleichen ESP-NOW-Kanal betrieben und über ihre MAC-Adresse miteinander gekoppelt. Der Sender erfasst in regelmäßigen Intervallen die Sensordaten, fasst sie in einer Datenstruktur zusammen und überträgt sie an den Empfänger.

Der Empfänger verarbeitet die eingehenden Daten und speichert die aktuellen Werte im Arbeitsspeicher. Danach wird ein integrierter Webserver gestartet, der ein eigenes WLAN (Access Point) erstellt. Über diese Verbindung kann eine Webseite aufgerufen werden, die die aktuellen Sensorwerte anzeigt und regelmäßig aktualisiert.

Optional wird eine Datenbankverbindung eingerichtet, bei der die empfangenen Messwerte zusätzlich an eine MySQL-Datenbank gesendet werden. Dort werden die Daten mit Zeitstempel gespeichert und können später für Diagramme und Analysen verwendet werden.

Zum Abschluss wird das gesamte System getestet, indem Bewegungen ausgelöst und Gaswerte verändert werden. Dabei wird überprüft, ob die Daten korrekt übertragen, angezeigt und gespeichert werden.

---

## Sender (ESP32)

```cpp
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
```

---

## Empfänger (ESP32)

```cpp
#include <WiFi.h>
#include <esp_now.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/* ==== OLED ==== */
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

/* ==== RGB LED ==== */
#define LED_R 25
#define LED_G 26
#define LED_B 27

/* ==== DATA ==== */
typedef struct {
int gas;
bool motion;
} SensorData;

SensorData incoming;
unsigned long lastPacket = 0;

/* ==== WEB ==== */
WebServer server(80);

/* ==== LED ==== */
void setColor(bool r, bool g, bool b) {
digitalWrite(LED_R, r);
digitalWrite(LED_G, g);
digitalWrite(LED_B, b);
}

/* ==== ESP-NOW RECEIVE ==== */
void onReceive(const esp_now_recv_info*, const uint8_t* data, int len) {
if (len == sizeof(SensorData)) {
memcpy(&incoming, data, sizeof(incoming));
lastPacket = millis();
}
}

/* ==== DISPLAY ==== */
void updateDisplay() {
display.clearDisplay();
display.setCursor(0,0);
display.setTextSize(1);
display.setTextColor(SSD1306_WHITE);

display.println("IoT Receiver");
display.print("Gas: "); display.println(incoming.gas);
display.print("Motion: "); display.println(incoming.motion ? "YES":"NO");
display.print("Alive: "); display.print((millis()-lastPacket)/1000); display.println("s");

display.display();
}

/* ==== API ==== */
void handleAPI() {
server.sendHeader("Access-Control-Allow-Origin","*");
server.send(200,"application/json",
"{ \"gas\": "+String(incoming.gas)+
", \"motion\": "+String(incoming.motion?"true":"false")+" }"
);
}

/* ==== WEB PAGE ==== */
void handleRoot() {
server.sendHeader("Cache-Control","no-store");

String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>IoT Dashboard</title>
<script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
<style>
body{background:#111;color:#0f0;font-family:Arial;text-align:center}
canvas{max-width:90%}
</style>
</head>
<body>

<h1>IoT Dashboard</h1>
<p id="gas">Gas: --</p>
<p id="motion">Motion: --</p>
<canvas id="chart"></canvas>

<script>
let gasData=[], labels=[];
const ctx=document.getElementById("chart").getContext("2d");
const chart=new Chart(ctx,{
type:"line",
data:{labels:labels,datasets:[{label:"Gas",data:gasData,borderColor:"lime"}]}
});

async function update(){
const r=await fetch("/api/data",{cache:"no-store"});
const d=await r.json();
document.getElementById("gas").innerText="Gas: "+d.gas;
document.getElementById("motion").innerText="Motion: "+(d.motion?"YES":"NO");

labels.push("");
gasData.push(d.gas);
if(labels.length>20){labels.shift();gasData.shift();}
chart.update();
}
setInterval(update,2000);
</script>

</body>
</html>
)rawliteral";

server.send(200,"text/html",html);
}

void setup() {
Serial.begin(115200);

pinMode(LED_R,OUTPUT);
pinMode(LED_G,OUTPUT);
pinMode(LED_B,OUTPUT);

Wire.begin();
display.begin(SSD1306_SWITCHCAPVCC,0x3C);

WiFi.mode(WIFI_AP);
WiFi.softAP("IoT_Station");

esp_now_init();
esp_now_register_recv_cb(onReceive);

server.on("/", handleRoot);
server.on("/api/data", handleAPI);
server.begin();

Serial.println("Receiver ready");
}

void loop() {
if (millis() - lastPacket > 6000) setColor(1,0,0);

else if (incoming.motion) setColor(0,0,1);
else setColor(0,1,0);

updateDisplay();
server.handleClient();
}
```
