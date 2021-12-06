#include <arduino-timer.h>
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

#define INTERNAL_NUM_LEDS 67 //inner ring
#define EXTERNAL_NUM_LEDS 76 //outer ring

CRGBArray<INTERNAL_NUM_LEDS + EXTERNAL_NUM_LEDS> leds;

//input
#define JOYSTICK_DEADZONE 0.2
float joystickX = 0;
float joystickY = 0;
float joystickMagnitude = 0;
int joystickAngle = 0;
int joystickDirection = 0;

bool buttonPressed = false;
bool buttonReleased = true;
bool joystickMoved = false;
bool joystickReleased = true;

LiquidCrystal_74HC595 lcd(11, 13, 12, 1, 3, 4, 5, 6, 7);

const byte sevenSegDigits[10] = {
    B01111011, // = 0
    B01100000, // = 1
    B00110111, // = 2
    B01110110, // = 3
    B01101100, // = 4
    B01011110, // = 5
    B01011111, // = 6
    B01110000, // = 7
    B01111111, // = 8
    B01111100  // = 9
};

//Game
enum class GameState
{
    MENU,
    GAME,
    LOSE
};

auto timer = timer_create_default();
GameState gameState = GameState::MENU;
int8_t menuScreen = 0;
int8_t selectedScreen = -1;
bool isPlayerTurn = false;
int8_t patternList[100];
int8_t numColors = 4;
int8_t timeToAnswer = 5;
int8_t roundNum = 1;
int8_t timeLeft = 0;
int8_t inputNum = 0;

CRGB colorList[] = {
    CRGB::Blue, CRGB::Red, CRGB::Green, CRGB::Yellow,                     //
    CRGB::DarkOrchid, CRGB::DarkOrange, CRGB::Fuchsia, CRGB::GreenYellow, //
    CRGB::Cyan, CRGB::Maroon, CRGB::Teal, CRGB::Chartreuse};

bool colorActiveList[12]; //true if color should be bright

Timer<16, millis, uint8_t> timerColor;

#define MAX_BUZZER_FREQ 2000
#define MIN_BUZZER_FREQ 100
int buzzerToneList[12]; //corresponds to each color

/*
TODO:
    ! WHEN NEED TO USE TONE, DISABLE LED UPDATE.
    ! CALIBRATE LIGHTS/JOYSTICK
    ! DISPLAY ROUND NUMBER ON LCD
    ! DISPLAY TIME LEFT ON 7 SEGMENT DISPLAY
    ! FIX MENU NOT WORKING
    
    ? CHANGE JOYSTICK ANGLE AND MAGNITUDE TO uint8_t
    ? HIGH SCORE
    ? GET RID ON ENUM CLASS FOR GAMESTATE AND CHANGE TO VARIABLE
    ? ADD BETTER LIGHTING AND SOUND EFFECTS
    ? CHANGE MENU SYSTEM TO 2D ARRAY OF TITLE AND VALUES (MENU = { {TITLE, VALUE}, {TITLE, VALUE} }
*/
void setup()
{
    Serial.begin(9600);

    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, EXTERNAL_NUM_LEDS + INTERNAL_NUM_LEDS);
    FastLED.clear();
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 3000);

    lcd.begin(16, 2);

    pinMode(SSD_LATCH_PIN, OUTPUT);
    pinMode(SSD_CLOCK_PIN, OUTPUT);
    pinMode(SSD_DATA_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    randomSeed(analogRead(A5));
    resetGame();
}

void loop()
{
    inputUpdate();

    if (gameState == GameState::MENU)
    {
        menu();
    }
    else if (gameState == GameState::GAME)
    {
        game();
    }
    else if (gameState == GameState::LOSE)
    {
        lose();
    }

    timer.tick();
    timerColor.tick();
    displayLEDs();

    EVERY_N_MILLIS(50)
    {
        lcd.clear();
    }
}

//INPUT FUNCTIONS
void inputUpdate()
{
    buttonPressed = !digitalRead(BUTTON_PIN) && buttonReleased;
    buttonReleased = digitalRead(BUTTON_PIN);

    joystickX = map(analogRead(JOYSTICK_X_PIN), 0, 1023, -512, 512) / 512.0;
    joystickY = -map(analogRead(JOYSTICK_Y_PIN), 0, 1023, -512, 512) / 512.0;

    joystickAngle = (int)(degrees(atan2(-joystickY, joystickX)) + 180) % 360;
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
    else if (joystickAngle > 315 || joystickAngle <= 45)
    {
        return 1; //RIGHT
    }
    else if (joystickAngle > 45 && joystickAngle <= 135)
    {
        return 2; //UP
    }
    else if (joystickAngle > 135 && joystickAngle <= 225)
    {
        return 3; //LEFT
    }
    else if (joystickAngle > 225 && joystickAngle <= 315)
    {
        return 4; //DOWN
    }
    else
    {
        return 0;
    }
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
    // if (selectedScreen == -1)
    // {

    switch (menuScreen)
    {
    case 0: // Play screen
        lcd.setCursor(6, 0);
        lcd.print(F("Play"));
        break;
    case 1: // Change color num screen
        lcd.setCursor(2, 0);
        lcd.print(F("# of Colors"));
        lcd.setCursor(7, 1);
        lcd.print(numColors);
        break;
    case 2: // Change time to answer screen
        lcd.setCursor(2, 0);
        lcd.print(F("Answer Time"));
        lcd.setCursor(7, 1);
        if (timeToAnswer == 10)
        {
            lcd.print(F("INF"));
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
            lcd.print(F("<"));
        }
        if (menuScreen != 2)
        {
            lcd.setCursor(15, 0);
            lcd.print(F(">"));
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
    case 0:                           //Selected play
        for (int i = 0; i < 100; i++) //fill pattern list
        {
            patternList[i] = random(0, numColors - 1);
        }
        for (int i = 0; i < numColors; i++) //fill buzzer tone list
        {
            int step = (MAX_BUZZER_FREQ - MIN_BUZZER_FREQ) / (numColors - 1);
            buzzerToneList[i] = MIN_BUZZER_FREQ + (step * i);
        }
        gameState = GameState::GAME;
        break;
    case 1: //Selected color num
        if (numColors != 2)
        {
            lcd.setCursor(6, 1);
            lcd.print(F("<"));
        }
        if (numColors != 12)
        {
            lcd.setCursor(9, 1);
            lcd.print(F(">"));
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
            lcd.print(F("<"));
        }
        if (timeToAnswer != 10)
        {
            lcd.setCursor(9, 1);
            lcd.print(F(">"));
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

void game() // use isPlayerTurn to choose whether a pattern is showing or player is supposed to input
{
    lcd.setCursor(3, 0);
    lcd.print(F("Round"));
    lcd.setCursor(10, 0);
    lcd.print(roundNum);
    if (!isPlayerTurn)
    {

        EVERY_N_MILLIS(500)
        {
            for (int i = 0; i < 12; i++)
            {
                colorActiveList[i] = false;
            }
            if (inputNum >= roundNum)
            {
                inputNum = 0;
                isPlayerTurn = true;
                timeLeft = timeToAnswer;
                return;
            }
            colorActiveList[patternList[inputNum]] = true;
            timerColor.in(200, disableSection, patternList[inputNum]);
            tone(BUZZER_PIN, buzzerToneList[patternList[inputNum]]);
            delay(50);
            noTone(BUZZER_PIN);
            inputNum++;
        }
    }

    if (isPlayerTurn)
    {
        if (timeToAnswer != 10)
        {
            EVERY_N_SECONDS(1)
            {
                displayDigit(timeLeft);
                timeLeft--;
                if (timeLeft < 0)
                {
                    gameState = GameState::LOSE;
                    return;
                }
            }
        }

        int input = getInput();

        if (input != -1)
        {
            if (input == patternList[inputNum])
            {
                inputNum++;
                timeLeft = timeToAnswer;
                colorActiveList[input] = true;
                timerColor.in(100, disableSection, input);
                tone(BUZZER_PIN, buzzerToneList[input]);
                delay(50);
                noTone(BUZZER_PIN);
            }
            else
            {
                gameState = GameState::LOSE;
                return;
            }
        }

        if (inputNum >= roundNum)
        {
            roundNum++;
            inputNum = 0;

            long now = millis();
            while (millis() - now < 200)
            {
                timerColor.tick();
            }
            for (int i = 0; i < 12; i++)
            {
                colorActiveList[i] = false;
            }
            isPlayerTurn = false;
            FastLED.delay(1000);
        }
    }
}

void lose()
{
    leds.fill_solid(CRGB::Red);
    tone(BUZZER_PIN, 100);
    delay(250);
    FastLED.clear();
    tone(BUZZER_PIN, 300);
    delay(250);
    leds.fill_solid(CRGB::Red);
    tone(BUZZER_PIN, 100);
    delay(250);
    FastLED.clear();
    tone(BUZZER_PIN, 300);
    delay(250);
    leds.fill_solid(CRGB::Red);
    tone(BUZZER_PIN, 50);
    delay(250);
    FastLED.clear();
    noTone(BUZZER_PIN);
    resetGame();
}

void displayLEDs() // use this to handle what the led strip should be doing
{
    //display outer ring of colors
    for (int i = 0; i < numColors; i++)
    { //FIX BOUNDS LATER
        int startLed = (numColors - (i + 1)) / ((double)numColors) * EXTERNAL_NUM_LEDS;
        int numLedsInSection = EXTERNAL_NUM_LEDS / numColors;
        int endLed = startLed + numLedsInSection - 1;
        fill_solid(leds(INTERNAL_NUM_LEDS + startLed, INTERNAL_NUM_LEDS + endLed), numLedsInSection, colorList[i]);
        if (!colorActiveList[i])
        { // if the color is not "active" dim it a bit
            leds(INTERNAL_NUM_LEDS + startLed, INTERNAL_NUM_LEDS + endLed).fadeLightBy(250);
        }
    }

    //inner ring selection
    if (joystickMagnitude > JOYSTICK_DEADZONE)
    {
        int adjustedAngle = (((-1 * joystickAngle) + 90) % 360) + 270;
        adjustedAngle = (adjustedAngle + 140) % 360;
        CRGB colorPointedAt = colorList[getJoystickColorIndex(adjustedAngle)];

        int x = map(adjustedAngle, 0, 359, 0, (INTERNAL_NUM_LEDS - 2));

        leds[((x + 2) % INTERNAL_NUM_LEDS) + 1] = colorPointedAt;
        leds[((x + 1) % INTERNAL_NUM_LEDS) + 1] = colorPointedAt;
        leds[x + 1] = colorPointedAt;
        leds[((x - 1) % INTERNAL_NUM_LEDS) + 1] = colorPointedAt;
        leds[((x - 2) % INTERNAL_NUM_LEDS) + 1] = colorPointedAt;

        leds[((x + 2) % INTERNAL_NUM_LEDS) + 1].fadeLightBy(220);
        leds[((x + 1) % INTERNAL_NUM_LEDS) + 1].fadeLightBy(192);
        leds[((x - 1) % INTERNAL_NUM_LEDS) + 1].fadeLightBy(192);
        leds[((x - 2) % INTERNAL_NUM_LEDS) + 1].fadeLightBy(220);
    }
    leds(0, INTERNAL_NUM_LEDS - 1).fadeToBlackBy(50);

    FastLED.show();
}

int getJoystickColorIndex(int angle) //get color joystick is pointing to
{
    return (int)(angle * numColors / 360.0f);
}

int getInput()
{ //get color player selected, -1 if none has 5been selected

    if (joystickMagnitude > JOYSTICK_DEADZONE && buttonPressed)
    {
        int adjustedAngle = (((-1 * joystickAngle) + 90) % 360) + 270;
        adjustedAngle = (adjustedAngle + 140) % 360;
        return getJoystickColorIndex(adjustedAngle);
    }
    else
    {
        return -1;
    }
}

bool disableSection(uint8_t section)
{
    colorActiveList[section] = false;
    return false;
}

void resetGame()
{
    menuScreen = 0;
    selectedScreen = -1;
    roundNum = 1;
    timeLeft = 0;
    inputNum = 0;
    isPlayerTurn = false;
    displayDigit(-1);
    timerColor.empty();
    timer.empty();
    gameState = GameState::MENU;
}
