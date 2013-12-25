#include <Servo.h>

int lockPosition = 0;
int unlockPosition = 115;
int serialSpeed = 9600;
int servoPin = 11;
int redLedPin = 13;
int greenLedPin = 12;


Servo servo;

void setup()
{
  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  digitalWrite(redLedPin,HIGH);
  Serial.begin(serialSpeed);
  servo.attach(servoPin);
  lock();
  delay(500);
  servo.detach();
  digitalWrite(redLedPin,LOW);
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
  servo.attach(servoPin);
  unlock();
  digitalWrite(greenLedPin,HIGH);
  delay(5000);
  digitalWrite(greenLedPin,LOW);
  lock();
  delay(500); //enough time for the lock to close
  servo.detach();
}

void youShallNotPass()
{
  digitalWrite(redLedPin,HIGH);
  delay(500);
  digitalWrite(redLedPin,LOW);
}


void lock()
{
  servo.write(lockPosition);
}
void unlock()
{
  servo.write(unlockPosition);
}

