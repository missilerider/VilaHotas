class TrackballAxis : public Control {
    protected:
    unsigned char axisId;
    unsigned int pin1, pin2;
    float damp = TRACKBALLDAMP;
    unsigned int step = TRACKBALLSTEP;
    unsigned int max = TRACKBALLMAX;

    bool status1 = false, status2 = false;
    int value = 0, realValue = 0;

    public:
    TrackballAxis(unsigned char axisId, unsigned int pin1, unsigned int pin2) {
        this->axisId = axisId;
        this->pin1 = pin1;
        this->pin2 = pin2;
    }

    void poll() {
        if(this->status1) {
            if(digitalRead(this->pin1) == LOW) {
            this->status1 = false;
            this->value++;
            }
        } else {
            if(digitalRead(this->pin1) == HIGH) {
            this->status1 = true;
            this->value++;
            }
        }

        if(this->status2) {
            if(digitalRead(this->pin2) == LOW) {
            this->status2 = false;
            this->value--;
            }
        } else {
            if(digitalRead(this->pin2) == HIGH) {
            this->status2 = true;
            this->value--;
            }
        }
    }

    char update(Joystick_ *Joystick) {
        long target = this->realValue * this->damp + this->value * this->step;

        if(target > this->max) target = this->max;
        else if(target < -this->max-1) target = -this->max-1;

        this->realValue = (int)target;

        switch(this->axisId) {
            case AXIS_X: Joystick->setXAxis(this->realValue); break;
            case AXIS_Y: Joystick->setYAxis(this->realValue); break;
            case AXIS_Z: Joystick->setZAxis(this->realValue); break;
            case AXIS_RX: Joystick->setRxAxis(this->realValue); break;
            case AXIS_RY: Joystick->setRyAxis(this->realValue); break;
            case AXIS_RZ: Joystick->setRzAxis(this->realValue); break;
        }

        if(this->value > 0) {
            //tb->value -= TRACKBALLMAX/2;
            this->value /= 1.2;
            if(this->value < 0) this->value = 0;
        } else if(this->value < 0) {
            //tb->value += TRACKBALLMAX/2;
            this->value /= 1.2;
            if(this->value > 0) this->value = 0;
        }

        return 0;
    }
/*

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
    }*/
};
