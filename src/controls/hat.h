class Hat : public Control {
    protected:
    unsigned char hatId;
    unsigned char analogPort;
    unsigned char addrUp, addrDown, addrLeft, addrRight;

    public:
    Hat(unsigned char addrUp, unsigned char addrDown, unsigned char addrLeft, unsigned char addrRight, unsigned char analogPort, unsigned char hatId) {
        this->hatId = hatId;
        this->addrUp = addrUp;
        this->addrDown = addrDown;
        this->addrLeft = addrLeft;
        this->addrRight = addrRight;
        this->analogPort = analogPort;
    }

    unsigned char getButtonNumber() { return 0; }
    unsigned char getHatNumber() { return 1; }

    char update(Joystick_ *Joystick) {
        unsigned int w = Control::readValue(this->addrUp, this->analogPort);
        unsigned int x = Control::readValue(this->addrRight, this->analogPort);
        unsigned int y = Control::readValue(this->addrDown, this->analogPort);
        unsigned int v = Control::readValue(this->addrLeft, this->analogPort);

        if(BUTTONPRESSED(v)) { // Left
            if(BUTTONPRESSED(w)) Joystick->setHatSwitch(this->hatId, 315); // L + U
            else if(BUTTONPRESSED(y)) Joystick->setHatSwitch(this->hatId, 225); // L + D
            else Joystick->setHatSwitch(this->hatId, 270); // L
        } else if(BUTTONPRESSED(x)) { // Right
            if(BUTTONPRESSED(w)) Joystick->setHatSwitch(this->hatId, 45); // R + U
            else if(BUTTONPRESSED(y)) Joystick->setHatSwitch(this->hatId, 135); // R + D
            else Joystick->setHatSwitch(this->hatId, 90); // R
        } else if(BUTTONPRESSED(w)) { // Up
            Joystick->setHatSwitch(this->hatId, 0);
        } else if(BUTTONPRESSED(y)) { // Down
            Joystick->setHatSwitch(this->hatId, 180);
        } else {
            Joystick->setHatSwitch(this->hatId, -1); // Sin pulsar
            return 0; // Sin mover
        }

        return 1; // Movido
    }
};
