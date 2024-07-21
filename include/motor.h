#ifndef IKEA_PS2014_WIFI_MOTOR_H
#define IKEA_PS2014_WIFI_MOTOR_H

#include <Arduino.h>
#include "ArduinoJson.h"
#include "config.h"

#define RX 3
#define TX 1
#define D0 RX
#define D1 TX
#define D2 16
#define D3 5
#define D4 4
#define D5 14
#define D6 12
#define D7 13

#define D8 0
#define D9 2
#define D10 15

#define ENDSTOP_PIN_SIGNAL D9

#define M_DRIVER_PIN_ENABLE D7

#define M_SPEED_PIN_BIT_0 D6
#define M_SPEED_PIN_BIT_1 D5
#define M_SPEED_PIN_BIT_2 D4

#define M_SPEED_PIN_RESET D3
#define M_SPEED_PIN_SLEEP D2

#define M_DRIVER_PIN_STEP D1
#define M_DRIVER_PIN_DIR D0

enum ProgramId {
  NONE,
  HOME,
  UPDOWN,
  FULLY_OPEN,
  FULLY_CLOSED
};

class Motor {
private:
  static uint8 homed;
  static int8 direction;
  static uint8 speed;
  static uint16 position;
  static void (*program)();
  static ProgramId programId;

  static void setDirection(int8 newDirection);

  static void disableMotor();
  static void enableMotor();

  static void enableSleep();
  static void disableSleep();

  static void resetMotorDriver();

  static bool sendStep();

  static bool isEndstopPushed();

  static void programUpDown();

  static void programGoHome();

  static void programFullyOpen();

  static void programFullyClosed();
public:
  static void setSpeed(byte newsPeed);
  static void setProgram(ProgramId id);
  static void updateStatusDoc();

  static void setup();
  static void loop();
};

#endif //IKEA_PS2014_WIFI_MOTOR_H
