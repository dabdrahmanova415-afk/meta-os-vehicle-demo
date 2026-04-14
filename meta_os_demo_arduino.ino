/*
 * Meta-OS for Vehicle Demonstrator
 * Arduino Uno + FreeRTOS + Encoder + Servo + LEDs
 * 
 * Modes: NORMAL (green), ECO (yellow), SAFE (red)
 * - Encoder controls servo angle (throttle simulation)
 * - Button triggers SAFE mode (brake simulation)
 * - Battery discharges over time, switching to ECO at 20%
 * - Non-critical task is suspended in ECO/SAFE to save energy
 * - CSV data output over Serial for analysis
 */

#include <Arduino_FreeRTOS.h>
#include <task.h>
#include <Servo.h>
#include <Encoder.h>

// Pin definitions
#define ENC_CLK 11
#define ENC_DT  12
#define PIN_BRAKE 2
#define PIN_LED_NORM 6   // Green
#define PIN_LED_ECO 7    // Yellow
#define PIN_LED_SAFE 8   // Red
#define PIN_SERVO 9

// Objects
Servo throttleServo;
Encoder myEncoder(ENC_CLK, ENC_DT);

// Global variables
volatile int servoAngle = 90;
volatile bool brakePressed = false;
volatile int systemMode = 0;        // 0=NORMAL, 1=ECO, 2=SAFE
volatile int simulatedBattery = 100; // percent
volatile long lastEncoderPos = 0;

// Task handles
TaskHandle_t criticalHandle = NULL;
TaskHandle_t nonCriticalHandle = NULL;
TaskHandle_t metaHandle = NULL;

// ----------------------------------------------------------------------
// Critical task (highest priority, 10ms period)
// Reads encoder and brake, controls servo
// ----------------------------------------------------------------------
void vCriticalTask(void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  
  for (;;) {
    // Read encoder position
    long newPos = myEncoder.read();
    if (newPos != lastEncoderPos) {
      int delta = (newPos - lastEncoderPos) * 2;
      servoAngle += delta;
      servoAngle = constrain(servoAngle, 0, 180);
      lastEncoderPos = newPos;
    }
    
    // Read brake button (LOW when pressed due to INPUT_PULLUP)
    brakePressed = (digitalRead(PIN_BRAKE) == LOW);
    
    // Servo control logic with mode priorities
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
    
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10));
  }
}

// ----------------------------------------------------------------------
// Non-critical task (lowest priority, 100ms period)
// Simulates sensor polling, can be suspended in ECO/SAFE
// ----------------------------------------------------------------------
void vNonCriticalTask(void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  
  for (;;) {
    // Simulate temperature reading
    int fakeTemp = random(20, 35);
    
    // CSV output (time from scheduler, mode, angle, battery, brake)
    Serial.print(millis());
    Serial.print(",");
    Serial.print(systemMode);
    Serial.print(",");
    Serial.print(servoAngle);
    Serial.print(",");
    Serial.print(simulatedBattery);
    Serial.print(",");
    Serial.println(brakePressed ? 1 : 0);
    
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100));
  }
}

// ----------------------------------------------------------------------
// Meta-OS task (medium priority, 50ms period)
// Monitors battery, switches modes, controls LEDs, suspends non-critical task
// ----------------------------------------------------------------------
void vMetaOSTask(void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  int batteryCounter = 0;
  
  for (;;) {
    // Simulate battery discharge (1% every 2 seconds)
    if (++batteryCounter >= 40) {
      batteryCounter = 0;
      if (simulatedBattery > 0 && systemMode != 2) {
        simulatedBattery--;
      }
    }
    
    // Mode switching logic
    if (brakePressed) {
      systemMode = 2;                           // SAFE
    }
    else if (simulatedBattery <= 20) {
      systemMode = 1;                           // ECO
    }
    else {
      systemMode = 0;                           // NORMAL
    }
    
    // LED indicators
    digitalWrite(PIN_LED_NORM, (systemMode == 0) ? HIGH : LOW);
    digitalWrite(PIN_LED_ECO,  (systemMode == 1) ? HIGH : LOW);
    digitalWrite(PIN_LED_SAFE, (systemMode == 2) ? HIGH : LOW);
    
    // Suspend or resume non-critical task based on mode
    if (nonCriticalHandle != NULL) {
      if (systemMode == 1 || systemMode == 2) {
        vTaskSuspend(nonCriticalHandle);
      } else {
        vTaskResume(nonCriticalHandle);
      }
    }
    
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(50));
  }
}

// ----------------------------------------------------------------------
// Setup
// ----------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Print CSV header
  Serial.println("Time_ms,Mode,Angle,Battery,Brake");
  
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
  
  // Create FreeRTOS tasks
  xTaskCreate(vCriticalTask, "Critical", 256, NULL, 3, &criticalHandle);
  xTaskCreate(vNonCriticalTask, "NonCritical", 256, NULL, 1, &nonCriticalHandle);
  xTaskCreate(vMetaOSTask, "MetaOS", 256, NULL, 2, &metaHandle);
  
  // Start scheduler
  vTaskStartScheduler();
}

void loop() {
  // Empty — FreeRTOS takes over
}