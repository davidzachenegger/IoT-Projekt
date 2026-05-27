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