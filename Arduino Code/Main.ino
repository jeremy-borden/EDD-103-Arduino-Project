#include <fastIO.h>
#include <SevenSegmentPanel.h>
#include <LiquidCrystal_74HC595.h>
#include <FastLED.h>
#include <Input.h>

LiquidCrystal_74HC595 lcd(11, 13, 12, 1, 3, 4, 5, 6, 7);

double joyX; // joystick x coordinate
double joyY; // joystick y coordinate

void setup()
{
    lcd.begin(16, 2);
}

void loop()
{
    joyX = map(analogRead(A0), 0, 1023, -512, 512) / 512.0;
    joyY = -map(analogRead(A1), 0, 1023, -512, 512) / 512.0;
}

int joystickAngle() //return angle of joystick form 0 to 359 degrees
{
    return (int)(degrees(atan2(-joyY, -joyX)) + 180) % 360;
}

double joystickNormalized() // return normalized magnitude of joystick
{
    return sqrt(sq(joyY * sqrt(1.0 - (joyX * joyX * .5))) + sq(joyX * sqrt(1.0 - (joyY * joyY * .5))));
}