/*******************************************************
 * SMART DOOR LOCK SYSTEM
 * ESP32 + Blynk + Keypad + Servo + PIR + Ultrasonic
 *     + LDR + LED + Buzzer + Push Button
 *******************************************************/

//=======================================================
// BLYNK CONFIGURATION
//=======================================================

#define BLYNK_TEMPLATE_ID   "TMPL6lrPdsUaS"
#define BLYNK_TEMPLATE_NAME "Smart Door Lock System"
#define BLYNK_AUTH_TOKEN    "YOUR_NEW_BLYNK_AUTH_TOKEN"

//=======================================================
// LIBRARIES
//=======================================================

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Keypad.h>
#include <ESP32Servo.h>

//=======================================================
// WIFI
//=======================================================

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

//=======================================================
// PIN DEFINITIONS
//=======================================================

#define BUZZER_PIN  2
#define LED_PIN     4
#define PIR_PIN     13

#define TRIG_PIN    25
#define ECHO_PIN    26

#define LDR_PIN     34

#define BUTTON_PIN  12
#define SERVO_PIN   33

//=======================================================
// BLYNK VIRTUAL PINS
//=======================================================

#define VIRTUAL_ULTRASONIC         V1
#define VIRTUAL_MOTION             V2
#define VIRTUAL_LOCK_STATUS        V3
#define VIRTUAL_LIGHT_LEVEL        V4
#define VIRTUAL_MANUAL_DOOR_CONTROL V5

//=======================================================
// CONSTANTS
//=======================================================

// Door positions
#define DOOR_LOCKED_POSITION   0
#define DOOR_UNLOCKED_POSITION 90

// Door must have at least this much clear space to lock
#define SAFE_LOCK_DISTANCE 150

// LDR threshold
#define DARK_THRESHOLD 200

// Auto-lock delay
// Door automatically locks after 10 seconds if:
// Blynk control is OFF and physical manual control is OFF.
#define AUTO_LOCK_DELAY 10000UL

// Button debounce
#define BUTTON_DEBOUNCE_DELAY 50UL

// Buzzer duration
#define BUZZER_DURATION 1000UL

//=======================================================
// KEYPAD CONFIGURATION
//=======================================================

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {19, 18, 17, 16};
byte colPins[COLS] = {23, 22, 21, 5};

Keypad keypad = Keypad(makeKeymap(keys),
                       rowPins,
                       colPins,
                       ROWS,
                       COLS);

//=======================================================
// OBJECTS
//=======================================================

Servo myServo;
BlynkTimer timer;

//=======================================================
// DOOR / PASSWORD VARIABLES
//=======================================================

const String password = "234#";

String input_password = "";

bool doorUnlocked = false;

// Blynk switch state
bool blynkButtonState = false;

// Physical button state
bool manualButtonPressed = false;

// Time at which door was unlocked
unsigned long doorUnlockedTime = 0;

//=======================================================
// ULTRASONIC VARIABLES
//=======================================================

long duration = 0;

// -1 means invalid / no echo
int distance = -1;

//=======================================================
// PIR / SENSOR VARIABLES
//=======================================================

bool motionDetected = false;

int lightLevel = 0;

//=======================================================
// BUTTON DEBOUNCE VARIABLES
//=======================================================

bool lastButtonReading = HIGH;
bool stableButtonState = HIGH;

unsigned long lastDebounceTime = 0;

//=======================================================
// BUZZER VARIABLES
//=======================================================

bool buzzerActive = false;
unsigned long buzzerStartTime = 0;

//=======================================================
// FUNCTION DECLARATIONS
//=======================================================

void unlockDoor();
void lockDoor();
void wrongPassword();
int measureDistance();

void updateSensors();
void handleKeypad();
void handlePhysicalButton();
void handleAutoLock();
void handleBuzzer();

bool isDoorSafeToLock();

//=======================================================
// SETUP
//=======================================================

void setup() {

  Serial.begin(115200);

  //----------- Pin configuration -----------

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  pinMode(PIR_PIN, INPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  //----------- Initial outputs -----------

  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  //----------- Servo -----------

  myServo.attach(SERVO_PIN);

  // Start with door locked
  myServo.write(DOOR_LOCKED_POSITION);
  doorUnlocked = false;

  //----------- WiFi + Blynk -----------

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  //----------- Periodic sensor update -----------

  timer.setInterval(1000L, updateSensors);

  Serial.println();
  Serial.println("=================================");
  Serial.println(" Smart Door Lock System Started");
  Serial.println("=================================");
}

//=======================================================
// MAIN LOOP
//=======================================================

void loop() {

  // Blynk communication
  Blynk.run();

  // Timer callbacks
  timer.run();

  // Handle keypad
  handleKeypad();

  // Handle physical push button
  handlePhysicalButton();

  // Handle automatic locking
  handleAutoLock();

  // Handle non-blocking buzzer
  handleBuzzer();
}

//=======================================================
// KEYPAD HANDLING
//=======================================================

void handleKeypad() {

  char key = keypad.getKey();

  if (!key) {
    return;
  }

  Serial.print("Key Pressed: ");
  Serial.println(key);

  // '*' clears current input
  if (key == '*') {

    input_password = "";

    Serial.println("Password input cleared.");

    return;
  }

  // Add key to password
  input_password += key;

  // Limit input length
  if (input_password.length() > 10) {
    input_password = "";
    Serial.println("Input too long. Password reset.");
    return;
  }

  // '#' submits password
  if (key == '#') {

    Serial.print("Entered Password: ");
    Serial.println(input_password);

    if (input_password == password) {

      Serial.println("Correct password.");
      unlockDoor();

    }
    else {

      Serial.println("Incorrect password.");
      wrongPassword();
    }

    // Clear password after checking
    input_password = "";
  }
}

//=======================================================
// PHYSICAL BUTTON HANDLING
//=======================================================

void handlePhysicalButton() {

  bool reading = digitalRead(BUTTON_PIN);

  // Reading changed
  if (reading != lastButtonReading) {

    lastDebounceTime = millis();
    lastButtonReading = reading;
  }

  // Check whether reading has remained stable
  if ((millis() - lastDebounceTime) > BUTTON_DEBOUNCE_DELAY) {

    if (reading != stableButtonState) {

      stableButtonState = reading;

      // Button pressed
      if (stableButtonState == LOW) {

        manualButtonPressed = !manualButtonPressed;

        Serial.print("Manual button: ");
        Serial.println(
          manualButtonPressed ? "ON" : "OFF"
        );

        if (manualButtonPressed) {

          unlockDoor();

        }
        else {

          lockDoor();
        }
      }
    }
  }
}

//=======================================================
// SENSOR UPDATE
//=======================================================

void updateSensors() {

  //=====================================================
  // PIR
  //=====================================================

  motionDetected = digitalRead(PIR_PIN);

  if (motionDetected) {

    Serial.println("Motion detected near the keypad!");

    Blynk.virtualWrite(
      VIRTUAL_MOTION,
      "Motion Detected Near The Keypad"
    );

  }
  else {

    Blynk.virtualWrite(
      VIRTUAL_MOTION,
      "No Motion"
    );
  }

  //=====================================================
  // ULTRASONIC
  //=====================================================

  distance = measureDistance();

  if (distance >= 0) {

    Serial.print("Ultrasonic Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    Blynk.virtualWrite(
      VIRTUAL_ULTRASONIC,
      distance
    );

  }
  else {

    Serial.println("Ultrasonic: No valid echo.");

    Blynk.virtualWrite(
      VIRTUAL_ULTRASONIC,
      -1
    );
  }

  //=====================================================
  // LDR
  //=====================================================

  lightLevel = analogRead(LDR_PIN);

  Serial.print("Current light level (LDR): ");
  Serial.println(lightLevel);

  if (lightLevel < DARK_THRESHOLD) {

    digitalWrite(LED_PIN, HIGH);

    Serial.println("LED ON - Low light.");

  }
  else {

    digitalWrite(LED_PIN, LOW);

    Serial.println("LED OFF - Sufficient light.");
  }

  Blynk.virtualWrite(
    VIRTUAL_LIGHT_LEVEL,
    lightLevel
  );

  //=====================================================
  // Send current door state to Blynk
  //=====================================================

  Blynk.virtualWrite(
    VIRTUAL_MANUAL_DOOR_CONTROL,
    doorUnlocked ? 1 : 0
  );
}

//=======================================================
// ULTRASONIC MEASUREMENT
//=======================================================

int measureDistance() {

  // Make sure trigger starts LOW
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  // Send 10 us trigger pulse
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Wait for echo, timeout after 30 ms
  duration = pulseIn(
    ECHO_PIN,
    HIGH,
    30000UL
  );

  // No echo received
  if (duration == 0) {
    return -1;
  }

  // Speed of sound ≈ 0.034 cm/us
  int measuredDistance =
      (int)(duration * 0.034 / 2.0);

  return measuredDistance;
}

//=======================================================
// CHECK WHETHER DOOR IS SAFE TO LOCK
//=======================================================

bool isDoorSafeToLock() {

  // Invalid ultrasonic reading
  // -> fail safe and DO NOT lock
  if (distance < 0) {

    Serial.println(
      "Cannot lock: invalid ultrasonic reading."
    );

    Blynk.virtualWrite(
      VIRTUAL_LOCK_STATUS,
      "Lock Failed: Invalid Distance Reading"
    );

    return false;
  }

  // Object/person too close
  if (distance <= SAFE_LOCK_DISTANCE) {

    Serial.print(
      "Cannot lock: Object/person detected at "
    );

    Serial.print(distance);
    Serial.println(" cm.");

    Blynk.virtualWrite(
      VIRTUAL_LOCK_STATUS,
      "Lock Failed: Obstacle Near Door"
    );

    return false;
  }

  return true;
}

//=======================================================
// UNLOCK DOOR
//=======================================================

void unlockDoor() {

  if (doorUnlocked) {

    // Already unlocked
    return;
  }

  Serial.println("Unlocking door...");

  myServo.write(DOOR_UNLOCKED_POSITION);

  doorUnlocked = true;

  // Record unlock time for auto-lock
  doorUnlockedTime = millis();

  Blynk.virtualWrite(
    VIRTUAL_LOCK_STATUS,
    "Door Unlocked"
  );

  Blynk.virtualWrite(
    VIRTUAL_MANUAL_DOOR_CONTROL,
    1
  );

  Serial.println("Door is now UNLOCKED.");
}

//=======================================================
// LOCK DOOR
//=======================================================

void lockDoor() {

  if (!doorUnlocked) {

    return;
  }

  // Check obstacle before locking
  if (!isDoorSafeToLock()) {

    return;
  }

  Serial.println("Locking door...");

  myServo.write(DOOR_LOCKED_POSITION);

  doorUnlocked = false;

  Blynk.virtualWrite(
    VIRTUAL_LOCK_STATUS,
    "Door Locked"
  );

  Blynk.virtualWrite(
    VIRTUAL_MANUAL_DOOR_CONTROL,
    0
  );

  Serial.println("Door is now LOCKED.");
}

//=======================================================
// WRONG PASSWORD
//=======================================================

void wrongPassword() {

  Serial.println(
    "Wrong password - buzzer activated."
  );

  digitalWrite(BUZZER_PIN, HIGH);

  buzzerActive = true;

  buzzerStartTime = millis();

  Blynk.virtualWrite(
    VIRTUAL_LOCK_STATUS,
    "Wrong Password"
  );
}

//=======================================================
// BUZZER HANDLER
//=======================================================

void handleBuzzer() {

  if (!buzzerActive) {
    return;
  }

  if (millis() - buzzerStartTime >= BUZZER_DURATION) {

    digitalWrite(BUZZER_PIN, LOW);

    buzzerActive = false;
  }
}

//=======================================================
// AUTOMATIC LOCK
//=======================================================

void handleAutoLock() {

  // Door is already locked
  if (!doorUnlocked) {
    return;
  }

  // Blynk manual control is keeping the door unlocked
  if (blynkButtonState) {
    return;
  }

  // Physical button is keeping the door unlocked
  if (manualButtonPressed) {
    return;
  }

  // Wait until auto-lock delay expires
  if (millis() - doorUnlockedTime < AUTO_LOCK_DELAY) {
    return;
  }

  Serial.println(
    "Auto-lock timeout reached."
  );

  // Attempt to lock.
  // lockDoor() checks ultrasonic safety.
  lockDoor();

  // If locking failed because something is nearby,
  // retry on the next loop.
}

//=======================================================
// BLYNK MANUAL DOOR CONTROL
//=======================================================

BLYNK_WRITE(VIRTUAL_MANUAL_DOOR_CONTROL) {

  blynkButtonState = param.asInt();

  Serial.print("Blynk Door Control: ");
  Serial.println(blynkButtonState);

  if (blynkButtonState) {

    // User wants door unlocked
    unlockDoor();

  }
  else {

    // User wants door locked
    lockDoor();
  }
}

//=======================================================
// BLYNK CONNECTION
//=======================================================

BLYNK_CONNECTED() {

  Serial.println("Connected to Blynk.");

  // Synchronize Blynk control state
  Blynk.syncVirtual(
    VIRTUAL_MANUAL_DOOR_CONTROL
  );

  // Send current status
  Blynk.virtualWrite(
    VIRTUAL_LOCK_STATUS,
    doorUnlocked ? "Door Unlocked" : "Door Locked"
  );

  Blynk.virtualWrite(
    VIRTUAL_MANUAL_DOOR_CONTROL,
    doorUnlocked ? 1 : 0
  );
}