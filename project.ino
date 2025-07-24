// KUA KAI YI 2516000 DCEP/FT/1A/04
// WONG JIA KANG 2524708 DCEP/FT/1A/04

// Description: The project is on Home Security System, detecting movement and changes based on change in LDR value.
// When LDR triggered, it gives the user 5 seconds to input the Password to disarm the alarm. After 5 seconds, the alarm rings.
// The user is still able to disarm the alarm with the Password while the alarm is ringing.

#include <Wire.h>
#include "RichShieldTM1637.h" // somehow breaks Arduino's default BUZZER functions
#include "RichShieldPassiveBuzzer.h" // use their BUZZER function
#include "PCA9685.h" // Use for servo motor

#define PassiveBuzzerPin 3
PassiveBuzzer buz(PassiveBuzzerPin);
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
TM1637 disp(CLK,DIO);
#define PASSWORDLENGTH 5 // Set password length, must be same as the length of array, Key.
PCA9685 pwmController(Wire);
PCA9685_ServoEval pwmServo1;

int LdrValue = 0;
int PotValue = 0;
int Alert = 0;
int Pass = 0;
int Password[PASSWORDLENGTH];
int Key[PASSWORDLENGTH] = {1,2,1,2,1}; // 1 is Blue, 2 is Yellow
int AlarmDelay = 0;
int AlarmArmed = 0;
int time = 0;

void EnterPass(int Button);
void beep(void);
void Blink(int colour,int msdelay);
void AlarmLoop(void);

void setup() {
  Serial.begin(9600);
  pinMode(LED_RED,OUTPUT);
  pinMode(LED_GREEN,OUTPUT);
  pinMode(LED_BLUE,OUTPUT);
  pinMode(LED_YELLOW,OUTPUT);
  //pinMode(BUZZER,OUTPUT);
  pinMode(BUTTONK1, INPUT_PULLUP);
  pinMode(BUTTONK2, INPUT_PULLUP);
  disp.init();
  pwmController.resetDevices();
  pwmController.init();
  pwmController.setPWMFreqServo();
  pwmController.setChannelPWM(0, pwmServo1.pwmForAngle(-10));
  pwmController.setChannelPWM(1, pwmServo1.pwmForAngle(0));
delay(1000);
}

unsigned long previousMillisAlarm = 0;
unsigned long previousMillisGrace = 0;
unsigned long previousMillisCount = 0;
const unsigned long AlarmInt = 300;
const unsigned long GraceInt = 5001;
const unsigned long CountInt = 1;

void loop()
{
  unsigned long currentMillis = millis();
  LdrValue = analogRead(LDR_PIN);
  PotValue = analogRead(POTENT_PIN);
  if (LdrValue <= (952 - (PotValue/1023.0 * 952)))
  {
    if (Alert == 0 || AlarmDelay == 0 || AlarmArmed == 0) {
      AlarmDelay = 1;
    }
  }
  if (currentMillis - previousMillisAlarm >= AlarmInt) {
    previousMillisAlarm = currentMillis;
    AlarmLoop();
    Serial.println(LdrValue);
  }
  if (currentMillis - previousMillisGrace >= GraceInt) {
    if (AlarmDelay) {
      previousMillisGrace = currentMillis;
      if (AlarmArmed)
      {
        disp.clearDisplay();
        AlarmDelay = 0;
        AlarmArmed = 0;
        Alert = 1;
      }
      else
      {
        time = 500;
        Blink(LED_BLUE, 100);
        AlarmArmed = 1;
      }
    }
  }
  if (currentMillis - previousMillisCount >= CountInt) {
    if (AlarmDelay) {
      previousMillisCount = currentMillis;
      if (AlarmArmed)
      {
        if (time > -1) {
          time -= 1;
          disp.display(time / 100.0);
        }
      }
    }
  }
  if (Alert || AlarmArmed)
  {
    if (digitalRead(BUTTONK1) == 0)
    {
      EnterPass(1);
    }
    if (digitalRead(BUTTONK2) == 0)
    {
      EnterPass(2);
    }
    if (Pass >= PASSWORDLENGTH)
    {
    Pass = 0;
      for (int i = 0; i < PASSWORDLENGTH; i++)
      {
        if (Password[i] != Key[i])
        {
          Serial.println("WRONGPASS");
          Blink(LED_YELLOW, 100);
          return;
        }
        if (i == PASSWORDLENGTH - 1)
        {
          Alert = 0;
          AlarmArmed = 0;
          disp.clearDisplay();
          Blink(LED_GREEN, 1000);
        }
      }
      Serial.println("Alarm Reset!");
    }
  }
  
  if (Alert) {
    //code here 
    for (int i = 0; i <=90; i += 5;) {
        pwmController.setChannelPWM(1, pwmServo1.pwmForAngle(i)); 
        delay(200);
    }
    for (int i = 90; i>=90; i-=5;){
        pwmController.setChannelPWM(1, pwmServo1.pwmForAngle(i));
        delay(100);
    }
    for (int i = 0; i >= -90; i-=5;){
        pwmController.setChannelPWM(1, pwmServo1.pwmForAngle(i));
      delay(200);
    }
    for (int i = -90; i <= 0; i+=5;){
        pwmController.setChannelPWM(1, pwmServo1.pwmForAngle(i));
      delay(100);
    }
  }
}

void beep(void)
{
  //tone(BUZZER, 100);
  buz.playTone(20, 100);
  digitalWrite(LED_RED, HIGH);
  delay(100);
  digitalWrite(LED_RED, LOW);
  //noTone(BUZZER);
  return;
}

void Blink(int colour,int msdelay)
{
  digitalWrite(colour, HIGH);
  delay(msdelay);
  digitalWrite(colour, LOW);
  return;
}

void EnterPass(int Button)
{
  Password[Pass] = Button;
  Pass++;
  Blink(LED_BLUE, 200);
  if (Button == 1)
  while (digitalRead(BUTTONK1) == 0);
  if (Button == 2)
  while (digitalRead(BUTTONK2) == 0);
}

void AlarmLoop(void)
{
  if (AlarmDelay)
  {
    return;
  }
  if (Alert)
  {
    Serial.println("Beep!");
    beep();
    
  }
}
