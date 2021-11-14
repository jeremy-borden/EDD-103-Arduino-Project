#include <LiquidCrystal_74HC595.h>
#include <FastLED.h>

//input
double joystickX;
double joystickY;
uint8_t buttonPin = 8;
uint8_t joystickPinX = A0;
uint8_t joystickPinY = A1;

//7 segment display
uint8_t ssdLatchPin = 6;
uint8_t ssdClockPin = 7;
uint8_t ssdDataPin = 5;

byte sevenSegDigits[10] = {
    B01111011, // = 0
    B00001001, // = 1
    B10110011, // = 2
    B10011011, // = 3
    B11001001, // = 4
    B11011010, // = 5
    B11111000, // = 6
    B00001011, // = 7
    B11111011, // = 8
    B11001011  // = 9
};
//lcd
LiquidCrystal_74HC595 lcd(11, 13, 12, 1, 3, 4, 5, 6, 7);

void setup()
{
    pinMode(ssdLatchPin, OUTPUT);
    pinMode(ssdClockPin, OUTPUT);
    pinMode(ssdDataPin, OUTPUT);
}

void loop()
{
    displayDigit(8);
}

//INPUT FUNCTIONS
int joystickAngle()
{
    return (int)(degrees(atan2(-joystickY, -joystickX)) + 180) % 360;
}

double joystickMagnitude()
{
    return sqrt(sq(joystickY * sqrt(1.0 - (joystickX * joystickX * .5))) + sq(joystickX * sqrt(1.0 - (joystickY * joystickY * .5))));
}

void joystickUpdate()
{
    joystickX = map(analogRead(joystickPinX), 0, 1023, -512, 512) / 512.0;
    joystickY = -map(analogRead(joystickPinY), 0, 1023, -512, 512) / 512.0;
}

void displayDigit(int digit) // display digit on 7sd, -1 to clear display
{
    digitalWrite(ssdLatchPin, LOW);
    if (digit == -1)
    {
        shiftOut(ssdDataPin, ssdClockPin, MSBFIRST, B00000000);
    }
    else
    {
        shiftOut(ssdDataPin, ssdClockPin, MSBFIRST, sevenSegDigits[digit]);
    }
    digitalWrite(ssdLatchPin, HIGH);
}
