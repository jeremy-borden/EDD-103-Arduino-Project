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

//input
double joystickX = 0;
double joystickY = 0;
int joystickAngle = 0;
double joystickMagnitude = 0;
int joystickDirection = 0;

bool buttonPressed = false;
bool buttonReleased = true;
bool joystickMoved = false;
bool joystickReleased = true;

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
//Game
enum class GameState
{
    Menu,
    Game
};

GameState gameState = GameState::Menu;
int menuScreen = 0;
int selectedScreen = NULL;
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

    lcd.begin(16, 2);
}

void loop()
{
    inputUpdate();

    if (gameState == GameState::Menu)
    {
        lcd.setCursor(0, 0);
        lcd.print("<");
        lcd.setCursor(15, 0);
        lcd.print(">");
        switch (menuScreen)
        {
        case 0: // Play screen
            lcd.setCursor(6, 0);
            lcd.print("Play");
            if (buttonPressed)
            {
                selectedScreen = 0;
            }
            break;
        case 1: // Change color num screen
            lcd.setCursor(2, 0);
            lcd.print("# of Colors");
            lcd.setCursor(7, 1);
            lcd.print(numColors);
            if (buttonPressed)
            {
                selectedScreen = 1;
            }
            break;
        case 2: // Change time to answer screen
            lcd.setCursor(2, 0);
            lcd.print("Answer Time");
            lcd.setCursor(7, 1);
            lcd.print(timeToAnswer);
            if (buttonPressed)
            {
                selectedScreen = 2;
            }
            break;
        }
        //switch to handle sleected screen?
        switch (selectedScreen)
        {
        case NULL: //no screen is selected, allow player to scroll thru screens

            break;
        }
    }

    lcd.clear();
}

//INPUT FUNCTIONS
void inputUpdate()
{
    buttonPressed = digitalRead(BUTTON_PIN) && buttonReleased;
    buttonReleased = !digitalRead(BUTTON_PIN);

    joystickX = map(analogRead(JOYSTICK_X_PIN), 0, 1023, -512, 512) / 512.0;
    joystickY = -map(analogRead(JOYSTICK_Y_PIN), 0, 1023, -512, 512) / 512.0;

    joystickAngle = (int)(degrees(atan2(-joystickY, -joystickX)) + 180) % 360;
    joystickMagnitude = sqrt(sq(joystickY * sqrt(1.0 - (joystickX * joystickX * .5))) + sq(joystickX * sqrt(1.0 - (joystickY * joystickY * .5))));

    joystickMoved = joystickMagnitude>3;
    joystickDirection = calculateJoystickDirection();
}

int calculateJoystickDirection()
{
    if (joystickMagnitude < 0.7)
    {
        return 0; //CENTER
    }
    if (joystickAngle < 45 || joystickAngle > 315)
    {
        return 1; //RIGHT
    }
    if (joystickAngle > 45 && joystickAngle < 135)
    {
        return 2; //UP
    }
    if (joystickAngle > 135 && joystickAngle < 225)
    {
        return 3; //LEFT
    }
    if (joystickAngle > 225 && joystickAngle < 315)
    {
        return 4; //DOWN
    }
    return 0;
}

//DISPLAY FUNCTIONS
void displayDigit(int digit) // display digit on 7sd, NULL to clear display
{
    digitalWrite(SSD_LATCH_PIN, LOW);
    if (digit == NULL)
    {
        shiftOut(SSD_DATA_PIN, SSD_CLOCK_PIN, MSBFIRST, B00000000);
    }
    else
    {
        shiftOut(SSD_DATA_PIN, SSD_CLOCK_PIN, MSBFIRST, sevenSegDigits[digit]);
    }
    digitalWrite(SSD_LATCH_PIN, HIGH);
}