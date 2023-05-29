#include <Arduino.h>
#include <Servo.h>
#include <SoftwareSerial.h>

// Macros to work with regs easier
#define SET_OUTPUT(PORT, PIN) PORT |= (1 << PIN)
#define SET_INPUT(PORT, PIN) PORT &= ~(1 << PIN)
#define SET_PULLUP(PORT, PIN) PORT |= (1 << PIN)
#define SET_HIGH(PORT, PIN) PORT |= (1 << PIN)
#define SET_LOW(PORT, PIN) PORT &= ~(1 << PIN)
#define READ(PORT, PIN) PORT & (1 << PIN)

// PIN definitions
#define knockSensor 0
#define programSwitch PD2
#define lockMotor PD3
#define redLED PD4
#define greenLED PD5
#define topSwitch PD7
#define servoMotor1 PB1
#define servoMotor2 PB2

// Servomotor object to control the lock mechanism with knocks
Servo myServo1;
Servo myServo2;

// SoftwareSerial object to communicate with the bluetooth module
SoftwareSerial mySerial(12, 13); // RX, TX

// Constants used to calibrate the knock sensor
const int threshold = 3;           // Minimum signal from the piezo to register as a knock
const int rejectValue = 25;        // If an individual knock is off by this percentage of a knock we don't unlock.
const int averageRejectValue = 15; // If the average timing of the knocks is off by this percent we don't unlock.
const int knockFadeTime = 150;     // Milliseconds we allow a knock to fade before we listen for another one.
const int lockTurnTime = 650;      // Milliseconds that we run the motor to get it to go a half turn.

const int maximumKnocks = 20;       // Maximum number of knocks to listen for.
const int knockComplete = 1200;     // Longest time to wait for a knock before we assume that it's finished.
const int lockTurnWait = 4000;      // milliseconds to wait before turning the lock back

// Variables used to store the knock pattern
int secretCode[maximumKnocks] = {50, 25, 25, 50, 100, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};  // Initial setup: "Shave and a Hair Cut, two bits."
int knockReadings[maximumKnocks];   // When someone knocks this array fills with delays between knocks.
int knockSensorValue = 0;           // Last reading of the knock sensor.

boolean programButtonPressed = false;   // Tells us if the programming button has been pressed.
boolean isSwitchOpen = false;            // lidButton tells us if lid is open
boolean isBoxLocked = true;           // Tells us if the lock is locked or opened. Starts assuming it's locked.

void listenToSecretKnock();
void triggerDoorUnlock();
void triggerDoorLock();
boolean validateKnock();
void getBluetoothMessage();
void motorSinging(int sliderValue);
