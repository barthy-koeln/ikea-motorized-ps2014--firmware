#include <Arduino.h>
#include <digitalWriteFast.h>
#include "motor.h"
#include "config.h"
#include "server.h"

JsonDocument statusDoc;

unsigned long currentTime = 0;
unsigned long prevStepTime = 0;
unsigned int coarseOffsetSteps = 0;

uint8 Motor::homed = HOMED_NO;
int8 Motor::direction = OPEN;
uint8 Motor::speed = 0;
ProgramId Motor::programId = NONE;
void (*Motor::program)() = nullptr;

uint16 Motor::position = END_POS;

uint16 speedMap[] = {
  1000,
  750,
  500,
  250,
  125,
  75,
  50,
  25,
};

void Motor::setDirection(int8 newDirection){
  digitalWriteFast(M_DRIVER_PIN_DIR, newDirection == OPEN ? LOW : HIGH);
  statusDoc["direction"] = direction = newDirection;
  delay(300);
}

void Motor::setSpeed(byte newsPeed) {
  statusDoc["speed"] = speed = newsPeed;
}

void Motor::disableMotor(){
  digitalWriteFast(M_DRIVER_PIN_ENABLE, HIGH);
}

void Motor::enableMotor() {
  digitalWriteFast(M_DRIVER_PIN_ENABLE, LOW);
}

void Motor::enableSleep() {
  digitalWriteFast(M_SPEED_PIN_SLEEP, LOW);
}

void Motor::disableSleep() {
  digitalWriteFast(M_SPEED_PIN_SLEEP, HIGH);
}

void Motor::resetMotorDriver() {
  digitalWriteFast(M_SPEED_PIN_RESET, HIGH);
}

bool Motor::sendStep(){
  if (currentTime - prevStepTime >= speedMap[speed]) {
    prevStepTime = currentTime;
    digitalWriteFast(M_DRIVER_PIN_STEP, HIGH);
    digitalWriteFast(M_DRIVER_PIN_STEP, LOW);
    position += direction;

    if(position % 100 == 0) {
      statusDoc["position"] = position;
    }

    return true;
  }

  return false;
}


void Motor::setup () {
  pinModeFast(RX, FUNCTION_3);
  pinModeFast(TX, FUNCTION_3);
  pinModeFast(D2, FUNCTION_3);
  pinModeFast(D5, FUNCTION_3);
  pinModeFast(D6, FUNCTION_3);
  pinModeFast(D7, FUNCTION_3);

  pinModeFast(M_DRIVER_PIN_DIR, OUTPUT);
  pinModeFast(M_DRIVER_PIN_STEP, OUTPUT);

  pinModeFast(M_SPEED_PIN_SLEEP, OUTPUT);
  pinModeFast(M_SPEED_PIN_RESET, OUTPUT);
  pinModeFast(M_SPEED_PIN_BIT_2, OUTPUT);
  pinModeFast(M_SPEED_PIN_BIT_1, OUTPUT);
  pinModeFast(M_SPEED_PIN_BIT_0, OUTPUT);
  pinModeFast(M_DRIVER_PIN_ENABLE, OUTPUT);

  pinModeFast(ENDSTOP_PIN_SIGNAL, INPUT_PULLUP);

  // 16th microstepping
  digitalWriteFast(M_SPEED_PIN_BIT_0, HIGH);
  digitalWriteFast(M_SPEED_PIN_BIT_1, HIGH);
  digitalWriteFast(M_SPEED_PIN_BIT_2, HIGH);

  resetMotorDriver();

  setDirection(CLOSE);
  setSpeed(2);
}

void Motor::programUpDown() {
  if(position == 0){
    setDirection(CLOSE);
  }

  if(position >= END_POS){
    setDirection(OPEN);
  }

  sendStep();
}

void Motor::programFullyOpen() {
  if(position == 0){
    setProgram(ProgramId::NONE);
    return;
  }

  sendStep();
}

void Motor::programFullyClosed() {
  if(position == END_POS){
    setProgram(ProgramId::NONE);
    return;
  }

  sendStep();
}

bool Motor::isEndstopPushed(){
  return digitalReadFast(ENDSTOP_PIN_SIGNAL) == LOW;
}

void Motor::programGoHome() {
  if(homed == HOMED_COARSE){
    if(coarseOffsetSteps != 0) {
      if(sendStep()){
        coarseOffsetSteps--;
      }

      if(coarseOffsetSteps == 0){
        setSpeed(1);
        setDirection(CLOSE);
      }

      return;
    }

    if(isEndstopPushed()){
      setDirection(OPEN);
      setSpeed(2);

      statusDoc["position"] = position = END_POS;
      statusDoc["homed"] = homed = HOMED_FINE;

      setProgram(ProgramId::NONE);
      return;
    }

    sendStep();
    return;
  }

  if(isEndstopPushed()){
    coarseOffsetSteps = 2000;
    setDirection(OPEN);

    statusDoc["position"] = position = END_POS;
    statusDoc["homed"] = homed = HOMED_COARSE;
    return;
  }

  sendStep();
}

void Motor::loop(){
  currentTime = micros();

  if(program != nullptr){
    (*program)();
  }

  if(statusDoc.size() != 0){
    SocketServer::broadcast(statusDoc);
    statusDoc.clear();
  }
}

void Motor::setProgram (ProgramId id) {
  if(id == ProgramId::NONE) {
    disableMotor();
    enableSleep();
  } else {
    enableMotor();
    disableSleep();
  }

  switch (id) {
    case ProgramId::NONE:
      program = nullptr;
      break;
    case ProgramId::UPDOWN:
      program = &Motor::programUpDown;
      break;
    case ProgramId::HOME:
      setSpeed(4);
      setDirection(CLOSE);
      statusDoc["homed"] = homed = HOMED_NO;
      program = &Motor::programGoHome;
      break;
    case ProgramId::FULLY_OPEN:
      setDirection(OPEN);
      program = &Motor::programFullyOpen;
      break;
    case ProgramId::FULLY_CLOSED:
      setDirection(CLOSE);
      program = &Motor::programFullyClosed;
      break;
  }

  statusDoc["program"] = programId = id;
}

void Motor::updateStatusDoc () {
  statusDoc["homed"] = homed;
  statusDoc["position"] = position;
  statusDoc["direction"] = direction;
  statusDoc["speed"] = speed;
  statusDoc["program"] = programId;
}
