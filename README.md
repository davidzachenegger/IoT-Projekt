# IoT-Projekt
## Smartes Überwachungssystem mit Gas- und Bewegungssensor  

**Untertitel:** Sicherheitsüberwachung mit ESP32 und Sensorik  
**Verfasser:** Dein Name  
**Datum:** 26.05.2026  

---

## 1. Einführung  

In modernen Gebäuden steigt die Bedeutung von automatisierten Sicherheitssystemen stetig. Insbesondere die Erkennung von Gaslecks und unerwünschten Bewegungen stellt einen wichtigen Bestandteil dar.  
Im Rahmen dieses Projekts wird ein System entwickelt, das sowohl Gaskonzentrationen als auch Bewegungen erkennt.

---

## 2. Projektbeschreibung  

Es wurde ein System realisiert, welches mithilfe eines Gas­sensors und eines Bewegungssensors potenzielle Gefahren erkennt.  
Die erfassten Daten werden über einen Mikrocontroller verarbeitet und über die serielle Schnittstelle ausgegeben.

---

## 3. Theorie  

### Bewegungssensor (PIR)  

Ein PIR-Sensor (Passive Infrared Sensor) erkennt Bewegungen durch Veränderungen in der Infrarotstrahlung.  
Er wird häufig in Alarmanlagen eingesetzt.

**Funktionsweise:**  
- Er misst Wärmestrahlung von Objekten  
- Bewegung verändert das IR-Muster  
- Der Sensor gibt ein digitales Signal (HIGH/LOW) aus  

---

### Gassensor (MQ-Serie)  

Ein MQ-Gassensor misst die Konzentration von Gasen in der Luft.

**Eigenschaften:**  
- Analoger Ausgang  
- Reagiert auf verschiedene Gase (z. B. Methan, LPG)  
- Benötigt Aufwärmzeit  

---

### Mikrocontroller (ESP32)  

Der ESP32 dient als zentrale Steuereinheit.

**Aufgaben:**  
- Auslesen der Sensorwerte  
- Verarbeitung der Daten  
- Ausgabe über Serial Monitor  

---

## 4. Arbeitsschritte  

### 4.1 Hardware Aufbau  

Folgende Komponenten wurden verwendet:  
- ESP32  
- PIR Bewegungssensor  
- MQ Gassensor  
- Kabel und Breadboard  

---

### 4.2 Schaltung  

![Schaltung](https://i.pinimg.com/originals/22/99/fd/2299fd6a63810585136801a720768cde.jpg)

**Beschreibung:**  
- PIR → Digital Pin (z. B. GPIO 13)  
- MQ Sensor → Analog Pin (z. B. GPIO 34)  
- VCC und GND entsprechend verbinden  

---

### 4.3 Implementierung  

#### Code  

```cpp
const int pirPin = 13;
const int gasPin = 34;

int pirState = 0;
int gasValue = 0;

void setup() {
  Serial.begin(9600);
  pinMode(pirPin, INPUT);
}

void loop() {
  pirState = digitalRead(pirPin);
  gasValue = analogRead(gasPin);

  Serial.print("Bewegung: ");
  if (pirState == HIGH) {
    Serial.println("Erkannt");
  } else {
    Serial.println("Keine Bewegung");
  }

  Serial.print("Gaswert: ");
  Serial.println(gasValue);

  delay(1000);
}
