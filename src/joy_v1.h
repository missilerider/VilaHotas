#include "controls/controls.h"

unsigned char b = 0;

Control *controls[] = {
    // Handle
    // Disk
    new Button(15, IN3, b++),
    new Switch3(14, IN3, 13, IN3, b++, b++, b++, 200, 200, 200), // Up
    new Switch3(12, IN3, 11, IN3, b++, b++, b++, 200, 200, 200),  // Dn

    // Thumbs
    new Button(8, IN3, b++), // U
    new Button(7, IN3, b++),
    new Button(10, IN3, b++), // D
    new Button(9, IN3, b++),

    new Button(4, IN3, b++), // Hat Press
    new Hat(1, 3, 5, 2, IN3, 0), // Hat
    new Button(6, IN3, b++), // Rotary Press
    new Rotary(8, 9, AXIS_Z), 
    new Button(0, IN3, b++), // Trackball Press
//    new TrackballAxis(AXIS_RY, 3, 4), 

    // Base Top
    new Switch3(2, IN1, 3, IN1, b++, b++), // 1
    new Switch3(0, IN1, 1, IN1, b++, b++), // 2
    new Switch3(5, IN1, 4, IN1, b++, b++, b++, 0, 0, 200), // 3
    new Switch3(7, IN1, 6, IN1, b++, b++, b++, 0, 0, 200), // 4

    // Base Bottom

    // Buttons
    new Button(9, IN1, b++), 
    new Button(8, IN1, b++), 

    // Switches
    new Switch3(13, IN2, 5, IN2, b++, b++, b++, 0, 0, 200), // 1
    new Switch3(1, IN2, 9, IN2, b++, b++, b++, 0, 0, 200), // 2
    new Switch3(6, IN2, 14, IN2, b++, b++, b++, 0, 0, 200), // 3

    // Audio
    // L
    new Button(10, IN2, b++), 
    new Button(2, IN2, b++), 
    new Button(12, IN2, b++), 
    // R
    new Button(4, IN2, b++), 
    new Button(8, IN2, b++), 
    new Button(0, IN2, b++), 

    // Dial
    new Button(15, IN2, b++), 
    new Button(7, IN2, b++), 
    new Button(11, IN2, b++), 
    new Button(3, IN2, b++)
};

unsigned int controlLen = sizeof(controls) / sizeof(Control*);
