#include "Arduino.h"
#include "Input.h"

Input::Input(uint8_t _buttonPin, uint8_t _joystickPinX, uint8_t _joystickPinY)
{
    buttonPin = _buttonPin;
    joystickPinX = _joystickPinX;
    joystickPinY = _joystickPinY;
}

double Input::getJoystickX()
{
    return joystickX;
}

double Input::getJoystickY()
{
    return joystickY;
}

int Input::joystickAngle()
{
    return (int)(degrees(atan2(-joystickY, -joystickX)) + 180) % 360;
}

double Input::joystickMagnitude()
{
    return sqrt(sq(joystickY * sqrt(1.0 - (joystickX * joystickX * .5))) + sq(joystickX * sqrt(1.0 - (joystickY * joystickY * .5))));
}

void Input::update()
{
    joystickX = map(analogRead(joystickPinX), 0, 1023, -512, 512) / 512.0;
    joystickY = -map(analogRead(joystickPinY), 0, 1023, -512, 512) / 512.0;
}