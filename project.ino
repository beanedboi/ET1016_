#define BUZZER 3
#define LED_RED 4
#define LED_GREEN 5
#define LED_BLUE 6
#define LED_YELLOW 7
#define BUTTONK1 8
#define BUTTONK2 9
#define POTENT_PIN A0
#define LDR_PIN A2

#define PASSWORDLENGTH 5 // Set password length, must be same as the length of array, Key.

int LdrValue = 0;
int PotValue = 0;
int Alert = 0;
int Pass = 0;
int Password[PASSWORDLENGTH];
int Key[PASSWORDLENGTH] = {1,2,1,2,1}; // 1 is Blue, 2 is Yellow

void EnterPass(int Button);
void alarm(void);
void Blink(int colour,int msdelay);

void setup() {
  Serial.begin(9600);
  pinMode(LED_RED,OUTPUT);
  pinMode(LED_GREEN,OUTPUT);
  pinMode(LED_BLUE,OUTPUT);
  pinMode(LED_YELLOW,OUTPUT);
  pinMode(BUZZER,OUTPUT);
  pinMode(BUTTONK1, INPUT_PULLUP);
  pinMode(BUTTONK2, INPUT_PULLUP);

  
}


void loop()
{
  LdrValue = analogRead(LDR_PIN);
  PotValue = analogRead(POTENT_PIN);
  if (LdrValue <= (952 - (PotValue/1023.0 * 952)))
  {
    Alert = 1;
  }
  if (Alert == 1)
  {
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
          Blink(LED_GREEN, 1000);
        }
      }
      Serial.println("ResetPass");
    }
    alarm();
    if (digitalRead(BUTTONK1) == 0)
    {
      EnterPass(1);
    }
    if (digitalRead(BUTTONK2) == 0)
    {
      EnterPass(2);
    }
  }
  if (digitalRead(BUTTONK2) == 0)
  {
    if (Alert == 0)
    {
      Alert = 1;
      Blink(LED_YELLOW, 100);
    }
    if (Alert == 1)
    {
      Blink(LED_BLUE, 100);
    }
  }
  
  Serial.println(LdrValue);
  delay(200);
}

void alarm(void)
{
  tone(BUZZER, 100);
  digitalWrite(LED_RED, HIGH);
  delay(100);
  digitalWrite(LED_RED, LOW);
  noTone(BUZZER);
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
