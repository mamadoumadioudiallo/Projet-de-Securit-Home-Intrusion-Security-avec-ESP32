#include <Arduino.h>

// ====== Pins (change si besoin) ======
static const int PIN_PIR    = 27;   // motion sensor OUT
static const int PIN_DOOR   = 26;   // reed switch to GND (INPUT_PULLUP)
static const int PIN_BUZZER = 25;   // active buzzer recommended

// ====== Logic ======
static bool systemArmed = true;     // armed by default
static bool alarmOn = false;

// Debounce door
static uint32_t lastDoorChangeMs = 0;
static int lastDoorRaw = HIGH;
static int doorStable = HIGH;
static const uint32_t DOOR_DEBOUNCE_MS = 40;

// PIR cooldown to avoid spam
static uint32_t lastPirTriggerMs = 0;
static const uint32_t PIR_COOLDOWN_MS = 3000;

// Alarm pattern timing
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

  // With INPUT_PULLUP + reed to GND:
  // LOW = closed, HIGH = open
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
  // Pattern: beep 200ms ON, 200ms OFF (simple)
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

  pinMode(PIN_PIR, INPUT);               // PIR outputs HIGH/LOW
  pinMode(PIN_DOOR, INPUT_PULLUP);       // reed switch to GND
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
