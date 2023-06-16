#include <Wire.h>
#include <LiquidCrystal_I2C.h> // Library for LCD
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2);

int num_Measure = 20;
int pinSignal = A0;
int PIN_QUIET = 3;
int PIN_MODERATE = 4;
int PIN_LOUD = 5;
long Sound_signal;
long sum = 0;
long level = 0;
int soundlow = 40;
int soundmedium = 150;

void setup ()
{
  pinMode (pinSignal, INPUT); // Set the signal pin as input
  pinMode(PIN_QUIET, OUTPUT);
  pinMode(PIN_MODERATE, OUTPUT);
  pinMode(PIN_LOUD, OUTPUT);

  digitalWrite(PIN_QUIET, LOW);
  digitalWrite(PIN_MODERATE, LOW);
  digitalWrite(PIN_LOUD, LOW);

  Serial.begin(9600);
  lcd.begin(16, 2);

  // Turn on the backlight.
  lcd.backlight();
  lcd.clear();
}

void loop ()
{
  for (int i = 0 ; i < num_Measure; i++)
  {
    Sound_signal = analogRead (pinSignal);
    sum = sum + Sound_signal;
  }
  level = sum/ num_Measure;

  Serial.println(level);

  if (level<soundlow)
  {
    lcd.setCursor(0, 1);
    lcd.print("Level: Quiet");
    Serial.print(level);
    digitalWrite(PIN_QUIET, HIGH);
    digitalWrite(PIN_MODERATE, LOW);
    digitalWrite(PIN_LOUD, LOW);
  }
  else if (level>soundlow && level<soundmedium)
  {
    lcd.setCursor(0, 1);
    lcd.print("Level: Moderate");
    Serial.print(level);
    digitalWrite(PIN_QUIET, LOW);
    digitalWrite(PIN_MODERATE, HIGH);
    digitalWrite(PIN_LOUD, LOW);
  }
  else if (level>soundmedium)
  {
    lcd.setCursor(0, 1);
    lcd.print("Level: High");
    Serial.print(level);
    digitalWrite(PIN_QUIET, LOW);
    digitalWrite(PIN_MODERATE, LOW);
    digitalWrite(PIN_LOUD, HIGH);
  }
  sum = 0;
  delay(1000);
  lcd.clear();
}
