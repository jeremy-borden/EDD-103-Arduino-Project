#ifndef Morse_h
#define Morse_h
#include "Arduino.h"

class Input
{
public:
    Input(uint8_t _buttonPin, uint8_t _joystickPinX, uint8_t _joystickPinY);
    int joystickAngle();
    double joystickMagnitude();
    double getJoystickX();
    double getJoystickY();
    void update();

private:
    double joystickX;
    double joystickY;
    uint8_t buttonPin;
    uint8_t joystickPinX;
    uint8_t joystickPinY;
};

#endif