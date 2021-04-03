class Switch3 : public Control {
    protected:
    Button b1, b2;

    unsigned char idCenter;
    unsigned int momentaryCenter = 0;


    public:
    Switch3(unsigned char addrPos1, unsigned char analogPort1, unsigned char addrPos2, unsigned char analogPort2, 
        unsigned char buttonPos1, unsigned char buttonPos2, unsigned char buttonPosCenter, 
        unsigned int momentaryPos1 = 0, unsigned int momentaryPos2 = 0, unsigned int momentaryCenter = 0) : 
            b1(addrPos1, analogPort1, buttonPos1, momentaryPos1), 
            b2(addrPos2, analogPort2, buttonPos2, momentaryPos2) {

        if(momentaryCenter > 0)
        {
            this->momentaryCenter = momentaryCenter;
            this->idCenter = buttonPosCenter;
        }
    }

    Switch3(unsigned char addrPos1, unsigned char analogPort1, unsigned char addrPos2, unsigned char analogPort2, 
        unsigned char buttonPos1, unsigned char buttonPos2, 
        unsigned int momentaryPos1 = 0, unsigned int momentaryPos2 = 0) : 
            b1(addrPos1, analogPort1, buttonPos1), 
            b2(addrPos2, analogPort2, buttonPos2) {
    }

    unsigned char getButtonNumber() { return this->momentaryCenter > 0 ? 3 : 2; }

    char update(Joystick_ *J) {
        char newState = 0;

        if(b1.update(J)) { this->lastUpdate = millis(); newState = 1; }
        if(b2.update(J)) { this->lastUpdate = millis(); newState = 1; }

        if(this->momentaryCenter > 0) {
            if(newState != this->lastState) {
                this->lastState = newState;
                if(newState == 0) {
                    this->lastUpdate = millis();
                    J->setButton(this->idCenter, 1);
                } else {
                    J->setButton(this->idCenter, 0);
                }
            } else {
                if(newState == 0) {
                    if(millis() > this->lastUpdate + this->momentaryCenter) {
                        J->setButton(this->idCenter, 0);
                    }
                }
            }
        }

        return this->lastState;
/*
        char newState = BUTTONPRESSED(Control::readValue(this->addr, this->analogPort));
        // State change
        if(newState != lastState) {
            lastState = newState;
            if(newState) {
                // Press
                lastUpdate = millis();
                J->setButton(this->id, 1);

            } else {
                // Release
                J->setButton(this->id, 0);
            }
        } else {
            if(newState && this->momentary > 0) {
                if(millis() > this->lastUpdate + this->momentary) {
                    J->setButton(this->id, 0);
                }
            }
        }

        return newState;*/
    }
};
