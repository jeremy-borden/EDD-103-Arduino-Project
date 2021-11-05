#include <BetterJoystick.h>
#include <LiquidCrystal.h>
#include <FastLED.h>

LiquidCrystal lcd = LiquidCrystal(2, 3, 4, 5, 6, 7);
Joystick joystick(A0, A1, 8); // X, Y, switch

void setup()
{
    lcd.begin(16, 2);
    Serial.begin();
}

void loop()
{
    lcd.print("X: ");
    lcd.print(joystick.x());
    lcd.setCursor(0, 1);
    lcd.print("Y: ");
    lcd.print(joystick.y());
    delay(500);
    lcd.clear();
}