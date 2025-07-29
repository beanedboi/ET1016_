// KUA KAI YI 2516000 DCEP/FT/1A/04
// WONG JIA KANG 2524708 DCEP/FT/1A/04

// Description: The project is on Home Security System, detecting movement and changes based on change in LDR value.
// When LDR triggered, it gives the user 5 seconds to input the Password to disarm the alarm. After 5 seconds, the alarm rings.
// The sensitivity of the LDR can be changed with the Potentiometer.
// The user is still able to disarm the alarm with the Password while the alarm is ringing.
// The user also has an IR Remote, which enables user to control the system remotely.
// The user can Arm/Disarm the alarm, move the camera and connect to the camera remotely.

// Config
#define PASSWORDLENGTH 5 // Set password length, must be same as the length of array, Key.
int Key[PASSWORDLENGTH] = {1,2,1,2,1}; // 1 is Blue, 2 is Yellow.

// Libraries
#include <Wire.h>
#include "RichShieldTM1637.h" // Segment Display, somehow breaks Arduino's default BUZZER functions
#include "RichShieldPassiveBuzzer.h" // Buzzer, replace with Rich's BUZZER function
#include "RichShieldIRremote.h" // IR Remote
#include "PCA9685.h" // Survo

// Constant Variables

// PINS
#define RECV_PIN 2
#define PassiveBuzzerPin 3
#define LED_RED 4
#define LED_GREEN 5
#define LED_BLUE 6
#define LED_YELLOW 7
#define BUTTONK1 8
#define BUTTONK2 9
#define POTENT_PIN A0
#define LDR_PIN A2
#define CLK 10
#define DIO 11

// IR BUTTONS
#define POWERKEY 0x45
#define PLUSKEY 0x40
#define MINUSKEY 0x19
#define LEFTKEY 0x07
#define RIGHTKEY 0x09
#define PLAYKEY 0x15

// Libraries Setup
TM1637 disp(CLK,DIO);
PassiveBuzzer buz(PassiveBuzzerPin);
IRrecv IR(RECV_PIN);
PCA9685 pwmController(Wire);
PCA9685_ServoEval pwmServo1;

// Global Variables
int LdrValue = 0;
int PotValue = 0;
int Alert = 0;
int Pass = 0;
int Password[PASSWORDLENGTH];
int AlarmArmed = 0;
int time = 0;
int xaxis = 0;
int yaxis = -10;

// Prototype Functions
void EnterPass(int Button);
void beep(void);
void Blink(int colour,int msdelay);
void AlarmLoop(void);

void setup() {
  // Setup
  Serial.begin(9600);
  pinMode(LED_RED,OUTPUT);
  pinMode(LED_GREEN,OUTPUT);
  pinMode(LED_BLUE,OUTPUT);
  pinMode(LED_YELLOW,OUTPUT);
  pinMode(BUTTONK1, INPUT_PULLUP);
  pinMode(BUTTONK2, INPUT_PULLUP);
  disp.init();
  IR.enableIRIn();
  Wire.begin();
  pwmController.resetDevices();
  pwmController.init();
  pwmController.setPWMFreqServo();

  // Set Survo to Default Position (Center)
  pwmController.setChannelPWM(0, pwmServo1.pwmForAngle(-10));
  pwmController.setChannelPWM(1, pwmServo1.pwmForAngle(0));
}

// Timer Setup
unsigned long previousMillisAlarm = 0;
unsigned long previousMillisGrace = 0;
unsigned long previousMillisCount = 0;
// Timer Delay, where 1 is 1ms
const unsigned long AlarmInt = 300;
const unsigned long GraceInt = 7001;
const unsigned long CountInt = 1;

void loop()
{
  // Set Current time for timer to compare.
  unsigned long currentMillis = millis();

  // Set Current LDR and Pot Values to compare.
  LdrValue = analogRead(LDR_PIN);
  PotValue = analogRead(POTENT_PIN);

  // Check LDR
  if (LdrValue <= (952 - (PotValue/1023.0 * 952)))
  {
    if (Alert == 0 && AlarmArmed == 0) {
    previousMillisGrace = currentMillis;
    time = 500;
    Blink(LED_BLUE, 100);
    AlarmArmed = 1;
    }
  }
  // Timer 1
  if (currentMillis - previousMillisAlarm >= AlarmInt) {
    previousMillisAlarm = currentMillis;
    AlarmLoop();
    Serial.println(LdrValue);
    Serial.println(952 - (PotValue/1023.0 * 952));
  }
  // Timer 2
  if (currentMillis - previousMillisGrace >= GraceInt) {
    if (Alert == 0) {
      if (AlarmArmed) {
        disp.clearDisplay();
        AlarmArmed = 0;
        Alert = 1;
      }
    }
  }
  // Timer 3
  if (currentMillis - previousMillisCount >= CountInt) {
    if (AlarmArmed) {
      previousMillisCount = currentMillis;
      if (time > -1) {
        time -= 1;
        disp.display(time / 100.0);
      }
    }
  }
  // Password Checking
  if (Alert || AlarmArmed)
  {
    if (digitalRead(BUTTONK1) == 0) // Blue Button
    {
      EnterPass(1);
    }
    if (digitalRead(BUTTONK2) == 0) // Yellow Button
    {
      EnterPass(2);
    }
    if (Pass >= PASSWORDLENGTH) // Total Inputs >= Password Length
    {
    Pass = 0; // Reset total Inputs.
      for (int i = 0; i < PASSWORDLENGTH; i++)
      {
        if (Password[i] != Key[i]) // If the [i]th Input != to the [i]th of the Key
        {
          // Reject Password
          Serial.println("WRONGPASS");
          Blink(LED_YELLOW, 100);
          return;
        }
        if (i == PASSWORDLENGTH - 1) // If the subsequent Inputs fits the Key
        {
          // Disarm Alarm
          Alert = 0;
          AlarmArmed = 0;
          disp.clearDisplay();
          Blink(LED_GREEN, 1000);
        }
      }
    }
  }
  // IR Implimentation
  if (IR.decode()) {
    if (IR.isReleased()) {
      if (IR.keycode == POWERKEY) {
        buz.playTone(200, 100);
        Alert = !Alert; // Toggles Alarm
        IR.resume();
      }

      // Camera Movement
      if (IR.keycode == PLUSKEY) {
        if (yaxis < 90) {
          yaxis += 10;
        }
        pwmController.setChannelPWM(0, pwmServo1.pwmForAngle(yaxis));
        IR.resume();
      }
      if (IR.keycode == MINUSKEY) {
        if (yaxis > -10) {
          yaxis -= 10;
        }
        pwmController.setChannelPWM(0, pwmServo1.pwmForAngle(yaxis));
        IR.resume();
      }
      if (IR.keycode == LEFTKEY) {
        if (xaxis > -90) {
          xaxis -= 10;
        }
        pwmController.setChannelPWM(1, pwmServo1.pwmForAngle(xaxis));
        IR.resume();
      }
      if (IR.keycode == RIGHTKEY) {
        if (xaxis < 90) {
          xaxis += 10;
        }
        pwmController.setChannelPWM(1, pwmServo1.pwmForAngle(xaxis));
        IR.resume();
      }
      if (IR.keycode == PLAYKEY) {
        xaxis = 0;
        yaxis = -10;
        pwmController.setChannelPWM(1, pwmServo1.pwmForAngle(xaxis));
        pwmController.setChannelPWM(0, pwmServo1.pwmForAngle(yaxis));
        IR.resume();
      }
      IR.resume();
    }
    IR.resume();
  }
}


// Functions
void beep(void)
{
  // Plays a noise
  buz.playTone(20, 100); // Change by changing values in playTone(a, b) where a = Pitch and b = Duration
  Blink(LED_RED, 100)
  return;
}

void Blink(int colour,int msdelay)
{
  // Blinks an LED
  digitalWrite(colour, HIGH);
  delay(msdelay);
  digitalWrite(colour, LOW);
  return;
}

void EnterPass(int Button)
{
  // Inputs a Value in Password, to be compared to Key later
  Password[Pass] = Button;
  Pass++;
  Blink(LED_BLUE, 200);
  // Delays while button is held down to prevent accidental repeat inputs.
  if (Button == 1)
  while (digitalRead(BUTTONK1) == 0);
  if (Button == 2)
  while (digitalRead(BUTTONK2) == 0);
}

void AlarmLoop(void)
{
  // There was more stuff here but got moved/removed. Function could be used for future additions/debugging.
  if (Alert)
  {
    Serial.println("Beep!");
    beep();
  }
}
