#include <LiquidCrystal_74HC595.h>
#include <FastLED.h>

//pins
#define BUTTON_PIN 8
#define JOYSTICK_X_PIN A0
#define JOYSTICK_Y_PIN A1
#define SSD_LATCH_PIN 6
#define SSD_CLOCK_PIN 7
#define SSD_DATA_PIN 5
#define BUZZER_PIN 10
#define ledPin 9

#define NUM_LEDS 144
CRGB leds[NUM_LEDS];

double joystickX;
double joystickY;

LiquidCrystal_74HC595 lcd(11, 13, 12, 1, 3, 4, 5, 6, 7);

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

boolean isPlaying = false;
int patternArray[255];
int numColors = 4;
int timeToAnswer = 5;

void setup()
{
    Serial.begin(9600);

    FastLED.addLeds<WS2812B, ledPin, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(50);

    randomSeed(analogRead(0));

    pinMode(SSD_LATCH_PIN, OUTPUT);
    pinMode(SSD_CLOCK_PIN, OUTPUT);
    pinMode(SSD_DATA_PIN, OUTPUT);
}

void loop()
{
    
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
    joystickX = map(analogRead(JOYSTICK_X_PIN), 0, 1023, -512, 512) / 512.0;
    joystickY = -map(analogRead(JOYSTICK_Y_PIN), 0, 1023, -512, 512) / 512.0;
}

//DISPLAY FUNCTIONS
void displayDigit(int digit) // display digit on 7sd, -1 to clear display
{
    digitalWrite(SSD_LATCH_PIN, LOW);
    if (digit == -1)
    {
        shiftOut(SSD_DATA_PIN, SSD_CLOCK_PIN, MSBFIRST, B00000000);
    }
    else
    {
        shiftOut(SSD_DATA_PIN, SSD_CLOCK_PIN, MSBFIRST, sevenSegDigits[digit]);
    }
    digitalWrite(SSD_LATCH_PIN, HIGH);
}

//MENU FUNCTIONS
