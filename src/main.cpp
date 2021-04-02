#include <Arduino.h>
//#include <Joystick.h>
#include "Joystick2/Joystick2.h"

#define TESTBTN

#define TRACKBALLMAX  32767 // Pasos de movimiento de trackball = 100%
#define TRACKBALLSTEP 8192
#define TRACKBALLDAMP 0.8
#define UPDATEMILLIS 10 // Milisegundos entre actualizaciones de USB

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

// Macro para identificar si un botón está pulsado
#define BUTTONPRESSED(X)  ((X) >= 512 ? 0 : 1)
#define ACTIVATION 100 // 100 millis pulsado cuando haya cambio de posición (bi y tri)

//#define TOTALBUTTONS  37 // Botones a notificar por HID

#ifndef TESTBTN
#define MAXBTN 37
const unsigned char btnId[] =   {  0,    1,   2,  3,  4,  21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 36, 32, 33, 34, 35,               36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51 }; // Números de botón (0 based)
const unsigned char btnAddr[] = { 15,    6,   4,   0,   0,   1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,                        0, 1, 2,   3, 4,   5,   6,  7, 8,9,10,11,12,13,14,15 }; // Direcciones del multiplexor (MUX16)
const unsigned char btnPin[] =  { IN3, IN3, IN3, IN3, IN1, IN1, IN1, IN1, IN1, IN1, IN1, IN1, IN1, IN1, IN1, IN1, IN1, IN1, IN1, IN1, IN2, IN2, IN2, IN2, IN2, IN2, IN2, IN2, IN2, IN2, IN2, IN2, IN2, IN2, IN2, IN2 }; // Pines analógicos donde leer el dato
#define TOTALBUTTONS (MAXBTN+(MAXTRI*3)+(MAXBI*2))  // Botones a notificar por HID
#else
#define TOTALBUTTONS 16*3

unsigned char MAXBTN = 0;

unsigned char btnId[16*3];
unsigned char btnAddr[16*3];
unsigned char btnPin[16*3];

void testBtn() {
  unsigned char displ;
  unsigned char slot;
  int nn;

  displ = 0;
  slot = IN1;
  unsigned char id = 0;
  for(nn = 0; nn <= 15; nn++) {
    btnId[id] = nn+displ;
    btnAddr[id] = nn;
    btnPin[id] = slot;
    id++;
  }
  displ = 16;
  slot = IN2;
  for(nn = 0; nn <= 15; nn++) {
    btnId[id] = nn+displ;
    btnAddr[id] = nn;
    btnPin[id] = slot;
    id++;
  }
  displ = 32;
  slot = IN3;
  for(nn = 0; nn <= 15; nn++) {
    btnId[id] = nn+displ;
    btnAddr[id] = nn;
    btnPin[id] = slot;
    id++;
  }
  MAXBTN = id;
}
#endif

#define MAXTRI 0
const unsigned char triBtn[] = { 3, 4, 5,   6, 7, 8,   9, 10, 11,   12, 13, 14  }; // Botones, en grupos de 3
const unsigned char triAddr1[] {    7,        9,          11,           13      }; // Dirección MUX16 de posición izq
const unsigned char triAddr2[] {    8,        10,         12,           14      }; // Dirección MUX16 de posición der
const unsigned char triPin[] { IN3, IN3, IN3, IN3 }; // Pines analógicos

unsigned char triLast[MAXTRI]; // Último estado leído (para pulsar sólo cambios)
unsigned long triMillis[MAXTRI]; // Último momento de cambio de estado

#define MAXBI 0
const unsigned char biBtn[] = { 20, 21,  23, 24 }; // Botones, en grupos de 2
const unsigned char biAddr[] { 1, 15 }; // Dirección MUX16 de posición ON
const unsigned char biPin[] { IN0, IN0 }; // Pines analógicos

unsigned char biLast[MAXTRI]; // Último estado leído (para pulsar sólo cambios)
unsigned long biMillis[MAXTRI]; // Último momento de cambio de estado

#define MAXAXIS 0
const unsigned char axisId[] = { AXIS_RZ, AXIS_Y }; // Ejes
const unsigned char axisPin[] = { IN1, IN1 }; // Pines analógicos
const unsigned char axisAddr[] = { 9, 15 }; // Direcciones del MUX16
const char axisRev[] = { 0, 0 }; // 1 o 0, por si el valor está invertido

unsigned int axisMax[MAXAXIS]; // Máximo leído de un eje
unsigned int axisMin[MAXAXIS]; // Mínimo leído de un eje

#define MAXHAT 0
const unsigned char hatId[] = { 0 }; // Hats
const unsigned char hatAddr[] = { 1, 2, 3, 5 }; // MUX16 en grupos de 4 (Up, Right, Down, Left)
const unsigned char hatPin[] = { IN3 }; // Pines analógicos


// Variables temporales
unsigned int v, w, x, y, n, flag;
unsigned long lastUpdate = 0; // Micros con último update de Joystick

struct trackballData {
  bool status1, status2;
  unsigned char pin1, pin2;
  unsigned char axisId;
  int value;
  int realValue;
} tbX, tbY;

Joystick_ Joystick(
  JOYSTICK_DEFAULT_REPORT_ID,
  JOYSTICK_TYPE_JOYSTICK,
  TOTALBUTTONS, // JOYSTICK_DEFAULT_BUTTON_COUNT
  MAXHAT, // JOYSTICK_DEFAULT_HATSWITCH_COUNT
  true, // xAxis
  true, // yAxis
  false, // zAxis
  false, // rxAxis
  false, // ryAxis
  true, // rzAxis
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
  if(triMillis[triId] < millis() - ACTIVATION) {
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

void setBistate(unsigned char biId, unsigned char btnOn, unsigned char btnOff, unsigned char addr, unsigned char pin) {
  v = readValue(addr, pin);

  if(BUTTONPRESSED(v)) {
    flag = 1;
  } else {
    flag = 0;
  }

  if(flag != biLast[biId]) {
    biMillis[biId] = millis();
    biLast[biId] = flag;
  }

  // Si paso mucho tiempo, todo cero
  if(biMillis[biId] < millis() - ACTIVATION) {
    Joystick.setButton(btnOn, 0);
    Joystick.setButton(btnOff, 0);
    return;
  }

  switch(biLast[biId]) {
    case 1:
      Joystick.setButton(btnOn, 1);
      Joystick.setButton(btnOff, 0);
      break;

    case 0:
      Joystick.setButton(btnOn, 0);
      Joystick.setButton(btnOff, 1);
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

void processTrackball(trackballData *tb) {
  if(tb->status1) {
    if(digitalRead(tb->pin1) == LOW) {
      tb->status1 = false;
      tb->value++;
    }
  } else {
    if(digitalRead(tb->pin1) == HIGH) {
      tb->status1 = true;
      tb->value++;
    }
  }

  if(tb->status2) {
    if(digitalRead(tb->pin2) == LOW) {
      tb->status2 = false;
      tb->value--;
    }
  } else {
    if(digitalRead(tb->pin2) == HIGH) {
      tb->status2 = true;
      tb->value--;
    }
  }
}

void resetTrackball(trackballData *tb) {
  long target = tb->realValue * TRACKBALLDAMP + tb->value * TRACKBALLSTEP;

  if(target > TRACKBALLMAX) target = TRACKBALLMAX;
  else if(target < -TRACKBALLMAX-1) target = -TRACKBALLMAX-1;

  tb->realValue = (int)target;

  switch(tb->axisId) {
    case AXIS_X: Joystick.setXAxis(tb->realValue); break;
    case AXIS_Y: Joystick.setYAxis(tb->realValue); break;
    case AXIS_Z: Joystick.setZAxis(tb->realValue); break;
    case AXIS_RX: Joystick.setRxAxis(tb->realValue); break;
    case AXIS_RY: Joystick.setRyAxis(tb->realValue); break;
    case AXIS_RZ: Joystick.setRzAxis(tb->realValue); break;
  }

  if(tb->value > 0) {
    //tb->value -= TRACKBALLMAX/2;
    tb->value /= 1.2;
    if(tb->value < 0) tb->value = 0;
  } else if(tb->value < 0) {
    //tb->value += TRACKBALLMAX/2;
    tb->value /= 1.2;
    if(tb->value > 0) tb->value = 0;
  }
}

void setup() {
  Serial1.begin(115200);
  Serial1.println("Init");

  testBtn();

//  pinMode(IN0, INPUT_PULLUP);
  pinMode(IN0, INPUT); // Específico para sensor hall, AH3503
  pinMode(IN1, INPUT_PULLUP);
  pinMode(IN2, INPUT_PULLUP);
  pinMode(IN3, INPUT_PULLUP);

  pinMode(MUXS0, OUTPUT);
  pinMode(MUXS1, OUTPUT);
  pinMode(MUXS2, OUTPUT);
  pinMode(MUXS3, OUTPUT);

  setAxisRange(AXIS_X, 0, 1023); // Exclusivo A0, sensor hall

  // Rangos de ejes (mínimo)
  for(n = 0; n < MAXAXIS; n++) {
    axisMax[n] = 512;
    axisMin[n] = 511;
    setAxisRange(n, axisMin[n], axisMax[n]);
  }

  for(n = 0; n < MAXTRI; n++) {
    triLast[n] = 3;
    triMillis[n] = 0;
  }

  for(n = 0; n < MAXBI; n++) {
    biLast[n] = 2;
    biMillis[n] = 0;
  }

  // Trackball X
  tbX.pin1 = 3; // Dcha
  tbX.pin2 = 4; // Izq
  tbX.status1 = tbX.status2 = false;
  tbX.value = tbX.realValue = 0;
  tbX.axisId = AXIS_X;
  setAxisRange(tbX.axisId, -TRACKBALLMAX-1, TRACKBALLMAX);
  pinMode(tbX.pin1, INPUT);
  pinMode(tbX.pin2, INPUT);
  resetTrackball(&tbX);

  // Trackball Y
  tbY.pin1 = 5; // Dcha
  tbY.pin2 = 6; // Izq
  tbY.status1 = tbY.status2 = false;
  tbY.value = tbY.realValue = 0;
  tbY.axisId = AXIS_Y;
  setAxisRange(tbY.axisId, -TRACKBALLMAX-1, TRACKBALLMAX);
  pinMode(tbY.pin1, INPUT);
  pinMode(tbY.pin2, INPUT);
  resetTrackball(&tbY);

  Joystick.begin(false); // autoSendMode = false
}

void loop() {
  unsigned long now = millis();

  processTrackball(&tbX);
  processTrackball(&tbY);

  if(now - lastUpdate < UPDATEMILLIS) return;

  Joystick.setRzAxis(analogRead(IN0)); // Eje de acelerador

  for(n = 0; n < MAXBTN; n++) {
    setButton(btnId[n], btnAddr[n], btnPin[n]);
  }

  for(n = 0; n < MAXAXIS; n++) {
    setAxis(axisId[n], axisAddr[n], axisPin[n], axisRev[n]);
  }

  for(n = 0; n < MAXTRI; n++) {
    setTristate(n, triBtn[n*3], triBtn[n*3+1], triBtn[n*3+2], triAddr1[n], triAddr2[n], triPin[n]);
  }

  for(n = 0; n < MAXBI; n++) {
    setBistate(n, biBtn[n*2], biBtn[n*2+1], biAddr[n], biPin[n]);
  }

  for(n = 0; n < MAXHAT; n++) {
    setHat(hatId[n], hatAddr[n*4], hatAddr[n*4+1], hatAddr[n*4+2], hatAddr[n*4+3], hatPin[n]);
  }


  resetTrackball(&tbX);
  resetTrackball(&tbY);

  Joystick.sendState();
  lastUpdate = now;

/*  Serial1.print("X-: ");
  Serial1.print(digitalRead(3) == HIGH ? "x" : "o");
  Serial1.print(", X+: ");
  Serial1.print(digitalRead(4) == HIGH ? "x" : "o");
  Serial1.println();*/
}
