#include <Servo.h>

int lockPosition = 0;
int unlockPosition = 115;
int serialSpeed = 9600;
int servoPin = 11;
int redLedPin = 13;
int greenLedPin = 12;
Servo servoMain;

void setup()
{
  Serial.begin(serialSpeed);
  servoMain.attach(servoPin);
  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  lock();
}

void loop()
{
    while (Serial.available())
    {
      byte a = Serial.read();
      if (a == 103) //g (good)
      {
        youShallPass();
      }else if (a == 98) //b (bad)
      {
        youShallNotPass();
      }      
    }
}

void youShallPass()
{
  unlock();
  digitalWrite(greenLedPin,HIGH);
  delay(5000);
  digitalWrite(greenLedPin,LOW);
  lock();
}

void youShallNotPass()
{
  digitalWrite(redLedPin,HIGH);
  delay(500);
  digitalWrite(redLedPin,LOW);
}


void lock()
{
  servoMain.write(lockPosition);
}
void unlock()
{
  servoMain.write(unlockPosition);
}

