/*
 * Meta-OS for Vehicle Demonstrator — FIXED BUTTON & LEDS
 * Arduino Uno + Encoder + Servo + LEDs + Button
 */

#include <Servo.h>
#include <Encoder.h>

// ========== PIN DEFINITIONS ==========
#define ENC_CLK 11
#define ENC_DT  12
#define PIN_BRAKE 2
#define PIN_LED_NORM 6   // Green
#define PIN_LED_ECO 7    // Yellow
#define PIN_LED_SAFE 8   // Red
#define PIN_SERVO 9

// ========== OBJECTS ==========
Servo throttleServo;
Encoder myEncoder(ENC_CLK, ENC_DT);

// ========== GLOBAL VARIABLES ==========
int servoAngle = 90;
bool brakePressed = false;
int systemMode = 0;        // 0=NORMAL, 1=ECO, 2=SAFE
int simulatedBattery = 100;
long lastEncoderPos = 0;

// For timing
unsigned long lastBatteryTime = 0;
unsigned long lastSerialTime = 0;
unsigned long lastEncoderCheck = 0;

// For button debounce
unsigned long lastButtonCheck = 0;
bool lastButtonState = false;

// ========== SETUP ==========
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=========================================");
  Serial.println("META-OS VEHICLE DEMONSTRATOR - FIXED");
  Serial.println("=========================================");
  
  // Pin configuration
  pinMode(PIN_BRAKE, INPUT_PULLUP);
  pinMode(PIN_LED_NORM, OUTPUT);
  pinMode(PIN_LED_ECO, OUTPUT);
  pinMode(PIN_LED_SAFE, OUTPUT);
  
  // Initial LED state
  digitalWrite(PIN_LED_NORM, HIGH);
  digitalWrite(PIN_LED_ECO, LOW);
  digitalWrite(PIN_LED_SAFE, LOW);
  
  // Servo initialization
  throttleServo.attach(PIN_SERVO);
  throttleServo.write(90);
  
  Serial.println("Ready:");
  Serial.println("  - Turn encoder → servo moves");
  Serial.println("  - Press button (pin2) → SAFE mode (red LED)");
  Serial.println("  - Wait ~80 sec → ECO mode (yellow LED)");
  Serial.println("=========================================");
}

// ========== MAIN LOOP ==========
void loop() {
  unsigned long now = millis();
  
  // ========== 1. READ ENCODER (every 10ms) ==========
  if (now - lastEncoderCheck >= 10) {
    lastEncoderCheck = now;
    
    long newPos = myEncoder.read();
    if (newPos != lastEncoderPos) {
      int delta = (newPos - lastEncoderPos) * 1;
      servoAngle += delta;
      servoAngle = constrain(servoAngle, 0, 180);
      lastEncoderPos = newPos;
    }
  }
  
  // ========== 2. READ BRAKE BUTTON (with debounce, every 20ms) ==========
  if (now - lastButtonCheck >= 20) {
    lastButtonCheck = now;
    
    // Read button (LOW = pressed due to INPUT_PULLUP)
    bool rawButton = (digitalRead(PIN_BRAKE) == LOW);
    
    // Simple debounce: only change if state is stable
    if (rawButton != lastButtonState) {
      // Wait a bit and re-check
      delay(5);
      bool confirm = (digitalRead(PIN_BRAKE) == LOW);
      if (confirm == rawButton) {
        brakePressed = rawButton;
        lastButtonState = rawButton;
        
        // Debug output
        if (brakePressed) {
          Serial.println("BUTTON PRESSED → SAFE MODE");
        } else {
          Serial.println("BUTTON RELEASED");
        }
      }
    }
  }
  
  // ========== 3. SIMULATE BATTERY DISCHARGE (every 2 seconds) ==========
  if (now - lastBatteryTime >= 2000) {
    lastBatteryTime = now;
    if (simulatedBattery > 0 && systemMode != 2) {
      simulatedBattery--;
      
      // Debug when battery gets low
      if (simulatedBattery == 20) {
        Serial.println("BATTERY LOW (20%) → ECO MODE SOON");
      }
    }
  }
  
  // ========== 4. MODE SWITCHING LOGIC ==========
  int newMode = systemMode;
  
  if (brakePressed) {
    newMode = 2;                           // SAFE
  }
  else if (simulatedBattery <= 20) {
    newMode = 1;                           // ECO
  }
  else {
    newMode = 0;                           // NORMAL
  }
  
  // Update LEDs ALWAYS based on current mode (not only on change)
  digitalWrite(PIN_LED_NORM, (newMode == 0) ? HIGH : LOW);
  digitalWrite(PIN_LED_ECO,  (newMode == 1) ? HIGH : LOW);
  digitalWrite(PIN_LED_SAFE, (newMode == 2) ? HIGH : LOW);
  
  // If mode actually changed, print to Serial
  if (newMode != systemMode) {
    systemMode = newMode;
    
    Serial.print("MODE CHANGE: ");
    if (systemMode == 0) Serial.println("NORMAL (green LED)");
    else if (systemMode == 1) Serial.println("ECO (yellow LED) — throttle limited to 90°");
    else Serial.println("SAFE (red LED) — servo stopped");
  } else {
    systemMode = newMode;  // still update systemMode even if no change
  }
  
  // ========== 5. CONTROL SERVO (with mode priorities) ==========
  if (brakePressed || systemMode == 2) {
    throttleServo.write(0);                    // SAFE: full stop
  } 
  else if (systemMode == 1) {
    int limited = constrain(servoAngle, 0, 90); // ECO: 50% limit
    throttleServo.write(limited);
  } 
  else {
    throttleServo.write(servoAngle);            // NORMAL: full range
  }
  
  // ========== 6. CSV OUTPUT (every 100ms) ==========
  if (now - lastSerialTime >= 100) {
    lastSerialTime = now;
    
    Serial.print(now);
    Serial.print(",");
    Serial.print(systemMode);
    Serial.print(",");
    Serial.print(servoAngle);
    Serial.print(",");
    Serial.print(simulatedBattery);
    Serial.print(",");
    Serial.println(brakePressed ? 1 : 0);
  }
}
