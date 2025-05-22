

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

#define MOISTURE_SENSOR_PIN A0
#define LED_PIN 8
//#define RELAY_PIN 8  // Not used now, replaced by L298N

#define ENA 5   // L298N PWM speed control pin
#define IN1 6   // Motor direction pin
#define IN2 7   // Motor direction pin
#define BUZZER_PIN 9  // Buzzer connected to PB1 (D9)

#define BT_RX 2
#define BT_TX 3

LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial bluetooth(BT_RX, BT_TX);

// Tracks if the motor was previously ON
bool motorPreviouslyOn = false;

void setup() {
  pinMode(MOISTURE_SENSOR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  //pinMode(RELAY_PIN, OUTPUT); // Not used anymore

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);  // Make sure buzzer is off

  // Set default motor direction
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  lcd.init();
  lcd.backlight();

  Serial.begin(9600);
  bluetooth.begin(9600);

  //digitalWrite(RELAY_PIN, HIGH); // Not needed
  analogWrite(ENA, 0); // Motor OFF initially
}

void loop() {
  int rawValue = analogRead(MOISTURE_SENSOR_PIN);
  int moisturePercent = map(rawValue, 1023, 0, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Moist: ");
  lcd.print(moisturePercent);
  lcd.print("%");

  String status, motorStatus;
  int motorSpeed = 0;

  if (moisturePercent < 10) {
    motorSpeed = 255;  // High speed
    status = "Dry";
    motorStatus = "Fast";
  } else if (moisturePercent < 20) {
    motorSpeed = 170;  // Medium speed
    status = "Dry";
    motorStatus = "Medium";
  } else if (moisturePercent < 30) {
    motorSpeed = 100;  // Low speed
    status = "Dry";
    motorStatus = "Slow";
  } else {
    motorSpeed = 0;    // OFF
    status = "Wet";
    motorStatus = "OFF";
  }

  // Buzzer logic: beep only when motor turns ON from OFF
  bool motorCurrentlyOn = (motorSpeed > 0);
  if (motorCurrentlyOn && !motorPreviouslyOn) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);  // short beep
    digitalWrite(BUZZER_PIN, LOW);
  }
  motorPreviouslyOn = motorCurrentlyOn;

  analogWrite(ENA, motorSpeed);
  digitalWrite(LED_PIN, motorSpeed > 0 ? HIGH : LOW);

  lcd.setCursor(0, 1);
  lcd.print(status);
  lcd.print(" | M: ");
  lcd.print(motorStatus);

  // Bluetooth
  bluetooth.print("Soil Moisture: ");
  bluetooth.print(moisturePercent);
  bluetooth.print("% | Status: ");
  bluetooth.print(status);
  bluetooth.print(" | Motor: ");
  bluetooth.println(motorStatus);

  // Serial Monitor
  Serial.print("Soil Moisture: ");
  Serial.print(moisturePercent);
  Serial.print("% | Status: ");
  Serial.print(status);
  Serial.print(" | Motor: ");
  Serial.println(motorStatus);

  delay(1000);
}
