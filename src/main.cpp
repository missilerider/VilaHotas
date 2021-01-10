#include <Arduino.h>
#include "Joystick.h"

// Pines S0-S3 del MUX16
#define MUXS0 15
#define MUXS1 14
#define MUXS2 16
#define MUXS3 10
// Alias de entradas analógicas
#define IN0 A0
// Constantes de identificación de ejes
#define AXIS_X  0
#define AXIS_Y  1
#define AXIS_Z  2
#define AXIS_RX 3
#define AXIS_RY 4
#define AXIS_RZ 5

// Macro para identificar si un botón está pulsado
#define BUTTONPRESSED(X)  ((X) >= 512 ? 0 : 1)
#define TRIACTIVATION 100 // 100 millis pulsado cuando haya cambio de posición

#define TOTALBUTTONS  128 // Botones a notificar por HID

#define MAXBTN 2
const unsigned char btnId[] = { 127, 125 }; // Números de botones
const unsigned char btnAddr[] = { 1, 15 }; // Direcciones del multiplexor (MUX16)
const unsigned char btnPin[] = { IN0, IN0 }; // Pines analógicos donde leer el dato

#define MAXTRI 2
const unsigned char triBtn[] = { 10, 11, 12,  13, 14, 15 }; // Botones, en grupos de 3
const unsigned char triAddr1[] { 1, 15 }; // Dirección MUX16 de posición izq
const unsigned char triAddr2[] { 15, 1 }; // Dirección MUX16 de posición der
const unsigned char triPin[] { IN0, IN0 }; // Pines analógicos

unsigned char triLast[MAXTRI]; // Último estado leído (para pulsar sólo cambios)
unsigned long triMillis[MAXTRI]; // Último momento de cambio de estado

#define MAXAXIS 2
const unsigned char axisId[] = { AXIS_Z, AXIS_RX }; // Ejes
const unsigned char axisPin[] = { IN0, IN0 }; // Pines analógicos
const unsigned char axisAddr[] = { 0, 2 }; // Direcciones del MUX16
const char axisRev[] = { 1, 1 }; // 1 o 0, por si el valor está invertido

unsigned int axisMax[MAXAXIS]; // Máximo leído de un eje
unsigned int axisMin[MAXAXIS]; // Mínimo leído de un eje

#define MAXHAT 2
const unsigned char hatId[] = { 0, 1 }; // Hats
const unsigned char hatAddr[] = { 1, 15, 14, 13,  14, 12, 15, 1 }; // MUX16 en grupos de 4 (Up, Right, Down, Left)
const unsigned char hatPin[] = { IN0, IN0 }; // Pines analógicos


// Variables temporales
unsigned int v, w, x, y, n, flag;

Joystick_ Joystick(
  JOYSTICK_DEFAULT_REPORT_ID,
  JOYSTICK_TYPE_JOYSTICK,
  TOTALBUTTONS, // JOYSTICK_DEFAULT_BUTTON_COUNT
  MAXHAT, // JOYSTICK_DEFAULT_HATSWITCH_COUNT
  true, // xAxis
  true, // yAxis
  true, // zAxis
  true, // rxAxis
  false, // ryAxis
  false, // rzAxis
  false, // Rudder
  false, // Throttle
  false, // Accel
  false, // Brake
  false); // Steering

void setAnalogAddress(unsigned char addr) {
  digitalWrite(MUXS0, addr & 0x1);
  digitalWrite(MUXS1, (addr >> 1) & 0x1);
  digitalWrite(MUXS2, (addr >> 2) & 0x1);
  digitalWrite(MUXS3, (addr >> 3) & 0x1);
}

unsigned int readValue(int addr, int pin) {
  setAnalogAddress(addr);
  return analogRead(pin);
}

void setButton(unsigned char buttonId, unsigned char addr, unsigned char pin) {
  v = readValue(addr, pin);
  Joystick.setButton(buttonId, BUTTONPRESSED(v));
}

void setAxisRange(unsigned char axisId, int min, int max) {
  switch(axisId) {
    case AXIS_X: Joystick.setXAxisRange(min, max); break;
    case AXIS_Y: Joystick.setYAxisRange(min, max); break;
    case AXIS_Z: Joystick.setZAxisRange(min, max); break;
    case AXIS_RX: Joystick.setRxAxisRange(min, max); break;
    case AXIS_RY: Joystick.setRyAxisRange(min, max); break;
    case AXIS_RZ: Joystick.setRzAxisRange(min, max); break;
  }
}

void setAxis(unsigned char axisId, unsigned char addr, unsigned char pin, char reverse) {
  flag = 0;
  v = readValue(addr, pin);

  if(reverse == 1)
    v = 1023 - v;

  if(v > axisMax[n]) {
    flag = 1;
    axisMax[n] = v;
  }

  if(v < axisMin[n]) {
    flag = 1;
    axisMin[n] = v;
  }

  if(flag == 1)
    setAxisRange(n, axisMin[n], axisMax[n]);

  switch(axisId) {
    case AXIS_X: Joystick.setXAxis(v); break;
    case AXIS_Y: Joystick.setYAxis(v); break;
    case AXIS_Z: Joystick.setZAxis(v); break;
    case AXIS_RX: Joystick.setRxAxis(v); break;
    case AXIS_RY: Joystick.setRyAxis(v); break;
    case AXIS_RZ: Joystick.setRzAxis(v); break;
  }
}

void setTristate(unsigned char triId, unsigned char btnLeft, unsigned char btnCenter, unsigned char btnRight, 
                  unsigned char addr1, unsigned char addr2, unsigned char pin) {
  v = readValue(addr1, pin);
  w = readValue(addr2, pin);

  if(BUTTONPRESSED(v)) {
    flag = 1;
  } else if(BUTTONPRESSED(w)) {
    flag = 2;
  } else {
    flag = 0;
  }

  if(flag != triLast[triId]) {
    triMillis[triId] = millis();
    triLast[triId] = flag;
  }

  // Si paso mucho tiempo, todo cero
  if(triMillis[triId] < millis() - TRIACTIVATION) {
    Joystick.setButton(btnLeft, 0);
    Joystick.setButton(btnCenter, 0);
    Joystick.setButton(btnRight, 0);
    return;
  }

  switch(triLast[triId]) {
    case 1:
      Joystick.setButton(btnLeft, 1);
      Joystick.setButton(btnCenter, 0);
      Joystick.setButton(btnRight, 0);
      break;

    case 2:
      Joystick.setButton(btnLeft, 0);
      Joystick.setButton(btnCenter, 0);
      Joystick.setButton(btnRight, 1);
      break;

    case 0:
      Joystick.setButton(btnLeft, 0);
      Joystick.setButton(btnCenter, 1);
      Joystick.setButton(btnRight, 0);
      break;
  }
}

void setHat(unsigned char hatId, unsigned char addrUp, unsigned char addrRight, unsigned char addrDown, unsigned char addrLeft, unsigned char pin) {
  w = readValue(addrUp, pin);
  x = readValue(addrRight, pin);
  y = readValue(addrDown, pin);
  v = readValue(addrLeft, pin);

  if(BUTTONPRESSED(v)) { // Left
    if(BUTTONPRESSED(w)) Joystick.setHatSwitch(hatId, 315); // L + U
    else if(BUTTONPRESSED(y)) Joystick.setHatSwitch(hatId, 225); // L + D
    else Joystick.setHatSwitch(hatId, 270); // L
  } else if(BUTTONPRESSED(x)) { // Right
    if(BUTTONPRESSED(w)) Joystick.setHatSwitch(hatId, 45); // R + U
    else if(BUTTONPRESSED(y)) Joystick.setHatSwitch(hatId, 135); // R + D
    else Joystick.setHatSwitch(hatId, 90); // R
  } else if(BUTTONPRESSED(w)) { // Up
    Joystick.setHatSwitch(hatId, 0);
  } else if(BUTTONPRESSED(y)) { // Down
    Joystick.setHatSwitch(hatId, 180);
  } else Joystick.setHatSwitch(hatId, -1); // Sin pulsar
}

void setup() {
  Serial1.begin(9600);
  Serial1.println("Init");

  pinMode(A0, INPUT_PULLUP);
  pinMode(MUXS0, OUTPUT);
  pinMode(MUXS1, OUTPUT);
  pinMode(MUXS2, OUTPUT);
  pinMode(MUXS3, OUTPUT);

  for(n = 0; n < MAXAXIS; n++) {
    axisMax[n] = 512;
    axisMin[n] = 511;
    setAxisRange(n, axisMin[n], axisMax[n]);
  }

  for(n = 0; n < MAXTRI; n++) {
    triLast[n] = 3;
    triMillis[n] = 0;
    setAxisRange(n, axisMin[n], axisMax[n]);
  }

  Joystick.begin(false); // autoSendMode = false
}

void loop() {
  for(n = 0; n < MAXBTN; n++) {
    setButton(btnId[n], btnAddr[n], btnPin[n]);
  }

  for(n = 0; n < MAXAXIS; n++) {
    setAxis(axisId[n], axisAddr[n], axisPin[n], axisRev[n]);
  }

  for(n = 0; n < MAXTRI; n++) {
    setTristate(n, triBtn[n*3], triBtn[n*3+1], triBtn[n*3+2], triAddr1[n], triAddr2[n], triPin[n]);
  }

  for(n = 0; n < MAXHAT; n++) {
    setHat(hatId[n], hatAddr[n*4], hatAddr[n*4+1], hatAddr[n*4+2], hatAddr[n*4+3], hatPin[n]);
  }

  Joystick.sendState();
}
