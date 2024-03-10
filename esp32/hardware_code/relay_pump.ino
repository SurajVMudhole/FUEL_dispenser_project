#include <LiquidCrystal.h>

#define ESP32_SERIAL 1//change the pin number

volatile int flow_frequency; // Measures flow sensor pulses
float total_litres = 0; // Total litres passed
unsigned char flowsensor = 2; // Sensor Input
unsigned long currentTime;
unsigned long cloopTime;

// Define pins for relay control
#define MOTOR_RELAY_PIN 3 // Pin for motor relay
#define SOLENOID_VALVE_PIN 4 // Pin for solenoid valve relay

// Define initial state of relays
bool motorRelayState = LOW; // Initially off
bool solenoidValveState = LOW; // Initially off

// Variables for flow rate calculation
volatile unsigned int pulseCount;
float calibrationFactor = 4.5; // Conversion factor for flow sensor
float flowRate;
unsigned long oldTime;
float flowMilliLitres;
float totalMilliLitres;

float volume = 0; // Variable to store volume of fuel to be dispensed

// LCD Configuration
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);

void flow() // Interrupt function
{
  pulseCount++;
}

void setup()
{
  pinMode(flowsensor, INPUT);
  digitalWrite(flowsensor, HIGH); // Optional Internal Pull-Up
  Serial.begin(9600);
  ESP32_SERIAL.begin(9600); // Initialize serial communication with ESP32

  while (!Serial || !ESP32_SERIAL) {
    ; // Wait for both serial ports to connect
  }

  attachInterrupt(0, flow, RISING); // Setup Interrupt
  sei(); // Enable interrupts
  currentTime = millis();
  cloopTime = currentTime;

  // Set relay pins as output
  pinMode(MOTOR_RELAY_PIN, OUTPUT);
  pinMode(SOLENOID_VALVE_PIN, OUTPUT);

  // Initially turn off motor relay and solenoid valve relay
  digitalWrite(MOTOR_RELAY_PIN, motorRelayState);
  digitalWrite(SOLENOID_VALVE_PIN, solenoidValveState);

  // Initialize LCD
  lcd.begin(16, 2);
  lcd.print("Volume: ");
}

void loop()
{
  currentTime = millis();
  // Every second, calculate flow rate and check volume
  if (currentTime >= (cloopTime + 1000))
  {
    cloopTime = currentTime; // Updates cloopTime

    // If total volume hasn't reached desired volume
    if (totalMilliLitres < volume)
    {
      digitalWrite(MOTOR_RELAY_PIN, HIGH); // Turn on motor relay

      if ((millis() - oldTime) > 1000)
      {
        detachInterrupt(0);

        flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
        oldTime = millis();
        flowMilliLitres = (flowRate / 60) * 1000;
        totalMilliLitres += flowMilliLitres;

        // Reattach interrupt
        attachInterrupt(0, flow, RISING);

    
        // Display volume on LCD
        lcd.setCursor(8, 0);
        lcd.print(totalMilliLitres / 1000.0);
        lcd.print("L");

        // Optional delay for stability
        delay(100);
      }
    }
    else // If total volume has reached desired volume
    {
      // Turn off motor relay and solenoid valve relay
      digitalWrite(MOTOR_RELAY_PIN, LOW);
      digitalWrite(SOLENOID_VALVE_PIN, LOW);

      // Send acknowledgment signal back to ESP32
      ESP32_SERIAL.println("1");

      // Reset total volume
      totalMilliLitres = 0;
      volume = 0; // Reset volume variable

      // Clear LCD
      lcd.clear();
      lcd.print("Volume: ");
    }

    pulseCount = 0; // Reset pulse count
  }

  if (ESP32_SERIAL.available() > 0) {
    // Read the incoming data from ESP32
    volume = ESP32_SERIAL.parseFloat(); // Receive volume of fuel to be dispensed from ESP32
  }
}
