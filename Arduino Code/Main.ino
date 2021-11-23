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
#define LED_PIN 9

#define NUM_LEDS 144
#define INTERNAL_NUM_LEDS 60 //inner ring
#define EXTERNAL_NUM_LEDS 60 //outer ring
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
    MENU,
    GAME
};

GameState gameState = GameState::MENU;
int menuScreen = 0;
int selectedScreen = -1;
boolean isPlaying = false;
int patternArray[255];
int numColors = 4;
int timeToAnswer = 5;
int roundNum = 0;
int timeLeft = 0;
int inputNum = 0;

void setup()
{
    Serial.begin(9600);

    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, INTERNAL_NUM_LEDS);
    FastLED.clear();
    FastLED.setBrightness(50);
    lcd.begin(16, 2);

    pinMode(SSD_LATCH_PIN, OUTPUT);
    pinMode(SSD_CLOCK_PIN, OUTPUT);
    pinMode(SSD_DATA_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    displayDigit(-1);
    randomSeed(analogRead(0));
}

void loop()
{
    inputUpdate();

    if (gameState == GameState::MENU)
    {
        menu();
    }
    if (gameState == GameState::GAME)
    {
        game();
    }

    displayLEDs();
    EVERY_N_MILLIS(50)
    {
        lcd.clear(); //send data to lcd as buffer, only clear if data changes?
    }
}

//INPUT FUNCTIONS
void inputUpdate()
{
    buttonPressed = !digitalRead(BUTTON_PIN) && buttonReleased;
    buttonReleased = digitalRead(BUTTON_PIN);

    joystickX = map(analogRead(JOYSTICK_X_PIN), 0, 1023, -512, 512) / 512.0;
    joystickY = -map(analogRead(JOYSTICK_Y_PIN), 0, 1023, -512, 512) / 512.0;

    joystickAngle = (int)(degrees(atan2(-joystickY, -joystickX)) + 180) % 360;
    joystickMagnitude = sqrt(sq(joystickY * sqrt(1.0 - (joystickX * joystickX * .5))) + sq(joystickX * sqrt(1.0 - (joystickY * joystickY * .5))));

    joystickMoved = joystickMagnitude >= 0.5 && joystickReleased;
    joystickReleased = joystickMagnitude < 0.5;

    joystickDirection = getJoystickDirection();
}

int getJoystickDirection()
{
    if (joystickMagnitude < 0.5)
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

int getSelectedColor()
{
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

//PROCESSES
void menu()
{
    //have list of values for menu?

    switch (menuScreen)
    {
    case 0: // Play screen
        lcd.setCursor(6, 0);
        lcd.print("Play");
        break;
    case 1: // Change color num screen
        lcd.setCursor(2, 0);
        lcd.print("# of Colors");
        lcd.setCursor(7, 1);
        lcd.print(numColors);
        break;
    case 2: // Change time to answer screen
        lcd.setCursor(2, 0);
        lcd.print("Answer Time");
        lcd.setCursor(7, 1);
        if (timeToAnswer == 10)
        {
            lcd.print("INF");
        }
        else
        {
            lcd.print(timeToAnswer);
        }
        break;
    }
    if (buttonPressed && selectedScreen == -1)
    {
        selectedScreen = menuScreen;
        return;
    }

    switch (selectedScreen) //this is disgusting. if you have time please fix this
    {
    case -1: //no screen is selected, allow player to scroll thru screens
        if (menuScreen != 0)
        {
            lcd.setCursor(0, 0);
            lcd.print("<");
        }
        if (menuScreen != 2)
        {
            lcd.setCursor(15, 0);
            lcd.print(">");
        }
        if (joystickMoved)
        {
            if (joystickDirection == 1)
            {
                menuScreen++;
                if (menuScreen > 2)
                {
                    menuScreen = 2;
                }
            }
            else if (joystickDirection == 3)
            {
                menuScreen--;
                if (menuScreen < 0)
                {
                    menuScreen = 0;
                }
            }
        }
        break;
    case 0: //Selected play
        for (int i = 0; i < 255; i++)
        {
            patternArray[i] = random(0, numColors - 1);
        }
        gameState = GameState::GAME;
        break;
    case 1: //Selected color num
        if (numColors != 2)
        {
            lcd.setCursor(6, 1);
            lcd.print("<");
        }
        if (numColors != 12)
        {
            lcd.setCursor(9, 1);
            lcd.print(">");
        }
        if (joystickMoved)
        {
            if (joystickDirection == 1)
            {
                numColors++;
                if (numColors > 12)
                {
                    numColors = 12;
                }
            }
            else if (joystickDirection == 3)
            {
                numColors--;
                if (numColors < 2)
                {
                    numColors = 2;
                }
            }
        }
        break;
    case 2: //selected answer time
        if (timeToAnswer != 3)
        {
            lcd.setCursor(6, 1);
            lcd.print("<");
        }
        if (timeToAnswer != 10)
        {
            lcd.setCursor(9, 1);
            lcd.print(">");
        }
        if (joystickMoved)
        {
            if (joystickDirection == 1)
            {
                timeToAnswer++;
                if (timeToAnswer > 10)
                {
                    timeToAnswer = 10;
                }
            }
            else if (joystickDirection == 3)
            {
                timeToAnswer--;
                if (timeToAnswer < 3)
                {
                    timeToAnswer = 3;
                }
            }
        }
        break;
    }
    if (buttonPressed && selectedScreen != -1)
    {
        selectedScreen = -1;
        return;
    }
}

void game() // use isPlaying to choose whether a pattern is showing or player is supposed to input
{
    if (!isPlaying)
    {
    }

    if (isPlaying)
    {
    }
    /*
    if is playing
        start timer
        if no input, continue to count down
        if input, reset timer
        if input, check if input color == array color at input number
        if correct, continue, if incorrect, LOSE
        if timer == 0, LOSE
        if number of inputs so far != round number, continue prev
        if number of inputs == round number, round number++,
    if !isPlaying
        run thru pattern up to round number
    */
}

void displayLEDs() // use this to handle what the led strip should be doing
{
    if (joystickMagnitude > 0.1)
    {

        int x = map(joystickAngle, 0, 359, 0, 59);
        leds[x].setRGB(255, 255, 255);
        if(x==59){
            leds[0].setRGB(255, 255, 255);
        }
        if(x==0){
            leds[59].setRGB(255, 255, 255);
        }
    }
    blur1d(leds, INTERNAL_NUM_LEDS, 150);
    fadeToBlackBy(leds, INTERNAL_NUM_LEDS, 50);
    FastLED.show();
}