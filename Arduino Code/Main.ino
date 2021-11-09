#include <fastIO.h>
#include <SevenSegmentPanel.h>
#include <LiquidCrystal_74HC595.h>
#include <FastLED.h>
#include <Input.h>

LiquidCrystal_74HC595 lcd(11, 13, 12, 1, 3, 4, 5, 6, 7);
Input input(8, A0, A1);

void setup()
{
}

void loop()
{
    input.update();
}
