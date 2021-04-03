class Rotary : public Control {
    protected:
    unsigned char axisId;
    unsigned char pin1, pin2;
    int step = 0;
    unsigned char lastA = 0, lastB = 0;

    public:
    Rotary(unsigned char pin1, unsigned char pin2, unsigned char axisId) {
        this->axisId = axisId;
        this->pin1 = pin1;
        this->pin2 = pin2;
    }

    void poll() {
        int a = digitalRead(this->pin1);
        int b = digitalRead(this->pin2);

        if((this->lastA == a || this->lastB == b) && (this->lastA != a || this->lastB != b)) {
            if(a == this->lastA) {
                if(a == LOW) {
                    if(b == LOW) this->step--;
                    else this->step++;
                } else {
                    if(b == LOW) this->step++;
                    else this->step--;
                }
            } else { // a != lastA
                if(b == LOW) {
                    if(a == LOW) this->step++;
                    else this->step--;
                } else {
                    if(a == LOW) this->step--;
                    else this->step++;
                }
            }

            this->lastA = a;
            this->lastB = b;
        }
    }

    char update(Joystick_ *Joystick) {
        int v = 0;

        v = this->step * 16786;

        if(v > TRACKBALLMAX) {
            v = TRACKBALLMAX;
            this->step = (int)(v / TRACKBALLMAX);
        }
        if(v > -TRACKBALLMAX) {
            v = -TRACKBALLMAX;
            this->step = (int)(v / -TRACKBALLMAX);
        }

        switch(this->axisId) {
            case AXIS_X: Joystick->setXAxis(v); break;
            case AXIS_Y: Joystick->setYAxis(v); break;
            case AXIS_Z: Joystick->setZAxis(v); break;
            case AXIS_RX: Joystick->setRxAxis(v); break;
            case AXIS_RY: Joystick->setRyAxis(v); break;
            case AXIS_RZ: Joystick->setRzAxis(v); break;
        }


        //this->step = 0;
        return 0;
    }
};
