#pragma once

#include "Joystick2/Joystick2.h"

// Pines S0-S3 del MUX16
#define MUXS0 10
#define MUXS1 16
#define MUXS2 14
#define MUXS3 15
// Alias de entradas analógicas
#define IN0 A0
#define IN1 A1
#define IN2 A2
#define IN3 A3
// Constantes de identificación de ejes
#define AXIS_X  0
#define AXIS_Y  1
#define AXIS_Z  2
#define AXIS_RX 3
#define AXIS_RY 4
#define AXIS_RZ 5

#ifndef BUTTONPRESSED
#define BUTTONPRESSED(X)  ((X) >= 512 ? 0 : 1)
#endif

class Control {
    protected:
    unsigned long lastUpdate = 0;
    char lastState = 0;

    public:
    static void setAnalogAddress(unsigned char addr) {
        digitalWrite(MUXS0, addr & 0x1);
        digitalWrite(MUXS1, (addr >> 1) & 0x1);
        digitalWrite(MUXS2, (addr >> 2) & 0x1);
        digitalWrite(MUXS3, (addr >> 3) & 0x1);
    }

    static unsigned int readValue(int addr, int pin) {
        Control::setAnalogAddress(addr);
        return analogRead(pin);
    }

    static void setButton(Joystick_ *Joystick, unsigned char buttonId, unsigned char addr, unsigned char pin) {
        Joystick->setButton(buttonId, BUTTONPRESSED(Control::readValue(addr, pin)));
    }

    public:
    virtual void poll() {}

    virtual char update(Joystick_*);
    virtual unsigned char getButtonNumber() { return 0; };
    virtual unsigned char getHatNumber() { return 0; };
};

class Axis {
    unsigned char axisId;
    Joystick_ *Joystick;
    unsigned int axisMax = 512,  axisMin = 512;

    Axis(Joystick_ *Joystick, unsigned char axisId) {
        this->axisId = axisId;
        this->Joystick = Joystick;
    }

    void setAxisRange(int min, int max) {
        switch(this->axisId) {
            case AXIS_X: this->Joystick->setXAxisRange(min, max); break;
            case AXIS_Y: this->Joystick->setYAxisRange(min, max); break;
            case AXIS_Z: this->Joystick->setZAxisRange(min, max); break;
            case AXIS_RX: this->Joystick->setRxAxisRange(min, max); break;
            case AXIS_RY: this->Joystick->setRyAxisRange(min, max); break;
            case AXIS_RZ: this->Joystick->setRzAxisRange(min, max); break;
        }
    }

    void setAxis(unsigned char addr, unsigned char pin, char reverse) {
        char flag = 0;
        unsigned int v = Control::readValue(addr, pin);

        if(reverse == 1)
            v = 1023 - v;

        if(v > this->axisMax) {
            flag = 1;
            this->axisMax = v;
        }

        if(v < this->axisMin) {
            flag = 1;
            this->axisMin = v;
        }

        if(flag == 1)
            this->setAxisRange(this->axisMin, this->axisMax);

        switch(this->axisId) {
            case AXIS_X: this->Joystick->setXAxis(v); break;
            case AXIS_Y: this->Joystick->setYAxis(v); break;
            case AXIS_Z: this->Joystick->setZAxis(v); break;
            case AXIS_RX: this->Joystick->setRxAxis(v); break;
            case AXIS_RY: this->Joystick->setRyAxis(v); break;
            case AXIS_RZ: this->Joystick->setRzAxis(v); break;
        }
    }
};

#include "button.h"
#include "switch3.h"
#include "hat.h"
#include "trackball.h"
#include "rotary.h"