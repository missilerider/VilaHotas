class Button : public Control {
    protected:
    unsigned char id;
    unsigned char analogPort;
    unsigned char addr;
    unsigned int momentary;


    public:
    Button(unsigned char addr, unsigned char analogPort, unsigned char buttonId, unsigned int momentaryDelayMs = 0) {
        this->id = buttonId;
        this->addr = addr;
        this->analogPort = analogPort;
        this->momentary = momentaryDelayMs;
    }

    unsigned char getButtonNumber() { return 1; }

    char update(Joystick_ *J) {
        char newState = BUTTONPRESSED(Control::readValue(this->addr, this->analogPort));
        // State change
        if(newState != lastState) {
            lastState = newState;
            if(newState) {
                // Press
                this->lastUpdate = millis();
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

        return newState;
    }
};
