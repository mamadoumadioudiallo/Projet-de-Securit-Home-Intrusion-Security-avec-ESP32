#include <Arduino.h>


static const int PIN_PIR    = 27;   
static const int PIN_DOOR   = 26;   
static const int PIN_BUZZER = 25;   


static bool systemArmed = true;     
static bool alarmOn = false;


static uint32_t lastDoorChangeMs = 0;
static int lastDoorRaw = HIGH;
static int doorStable = HIGH;
static const uint32_t DOOR_DEBOUNCE_MS = 40;


static uint32_t lastPirTriggerMs = 0;
static const uint32_t PIR_COOLDOWN_MS = 3000;


static uint32_t lastBeepMs = 0;
static bool beepState = false;

void setAlarm(bool on) {
  alarmOn = on;
  if (!on) {
    digitalWrite(PIN_BUZZER, LOW);
    beepState = false;
  }
}

void readSerialCommands() {
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == 'a' || c == 'A') {
      systemArmed = true;
      Serial.println("[SYSTEM] ARMED");
    } else if (c == 'd' || c == 'D') {
      systemArmed = false;
      setAlarm(false);
      Serial.println("[SYSTEM] DISARMED");
    } else if (c == 's' || c == 'S') {
        Serial.printf(
            "[STATUS] armed=%s alarm=%s door=%s pir=%d\n",
            systemArmed ? "true" : "false",
            alarmOn ? "true" : "false",
            (doorStable == LOW) ? "CLOSED" : "OPEN",
            digitalRead(PIN_PIR)
          );

      } else if (c == 'r' || c == 'R') {
      setAlarm(false);
      Serial.println("[ALARM] reset/off");
    }
  }
}

bool doorIsOpenDebounced() {
  int raw = digitalRead(PIN_DOOR);

  if (raw != lastDoorRaw) {
    lastDoorRaw = raw;
    lastDoorChangeMs = millis();
  }

  if (millis() - lastDoorChangeMs > DOOR_DEBOUNCE_MS) {
    doorStable = raw;
  }

  
  return (doorStable == HIGH);
}

bool pirTriggered() {
  int v = digitalRead(PIN_PIR);
  if (v == HIGH) {
    uint32_t now = millis();
    if (now - lastPirTriggerMs > PIR_COOLDOWN_MS) {
      lastPirTriggerMs = now;
      return true;
    }
  }
  return false;
}

void alarmBeepPattern() {
  
  uint32_t now = millis();
  if (now - lastBeepMs >= 200) {
    lastBeepMs = now;
    beepState = !beepState;
    digitalWrite(PIN_BUZZER, beepState ? HIGH : LOW);
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);

  pinMode(PIN_PIR, INPUT);               
  pinMode(PIN_DOOR, INPUT_PULLUP);       
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);

  Serial.println("\n=== ESP32 Home Security ===");
  Serial.println("Commands: A=arm, D=disarm, S=status, R=alarm off");
  Serial.println("Ready.");
}

void loop() {
  readSerialCommands();

  bool doorOpen = doorIsOpenDebounced();
  bool motion = pirTriggered();

  if (systemArmed && !alarmOn) {
    if (doorOpen) {
      Serial.println("[INTRUSION] Door OPEN detected -> ALARM!");
      setAlarm(true);
    } else if (motion) {
      Serial.println("[INTRUSION] Motion detected -> ALARM!");
      setAlarm(true);
    }
  }

  if (alarmOn) {
    alarmBeepPattern();
  }

  delay(5);
}
