#include <Arduino.h>
#include "simonIIR.h"

// Servo pins
#define ARMSERVO   7
#define ENDSERVO   8
#define TWISTSERVO 9
// IRR Sensor Pins
#define STRAIGHT 10
#define ANGLE    11

// IRR Sensor Threshold
#define WHITE 750

// CR Servo constants
#define STOP 127
#define GO   200

// Arm Heights
#define SIMONHEIGHT 127
#define RUBIKHEIGHT 100
#define HALFHEIGHT  63
#define ZEROHEIGHT  0

#define SIMONTIME 4500

// Horizontal position
colors_t color = RED;
int command;

int readQD(int QRE_PinNum);

void move_arm(int height){
  digitalWrite(ARMSERVO, height);
}

void turn(int steps, int sensor){
  for(int i = 0; i < steps; i++){
    digitalWrite(TWISTSERVO, GO);
    while(readQD(sensor) < WHITE);
    while(readQD(sensor) > WHITE);
    digitalWrite(TWISTSERVO, STOP);
  }
}

int readQD(int QRE_PinNum){
  //Returns value from the QRE1113
  //Lower numbers mean more refleacive
  //More than 3000 means nothing was reflected.
  int diff = 0;

  pinMode( QRE_PinNum, OUTPUT );
  digitalWrite( QRE_PinNum, HIGH );
  delayMicroseconds(10);
  pinMode( QRE_PinNum, INPUT );

  long time = micros();

  //time how long the input is HIGH, but quit after 3ms as nothing happens after that
  while (digitalRead(QRE_PinNum) == HIGH && micros() - time < 3000);
  diff = (micros() - time);

  return diff;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  sense_color_init();
  /*
  if(readQD(STRAIGHT) > WHITE){
    //move_arm(HALFHEIGHT);
    turn(1,STRAIGHT);
    //move_arm(ZEROHEIGHT);
  }
  */
  Serial.print("init finished");
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available()){
    command = Serial.read();
    switch(command){
      case 'S':
        //move_arm(SIMONHEIGHT);
        color = sense_color();
        //move_arm(HALFHEIGHT);
        //delay(SIMONTIME);
        //turn(color, STRAIGHT);
        //move_arm(SIMONHEIGHT);
        //move_arm(HALFHEIGHT);
        //delay(SIMONTIME);
        //move_arm(SIMONHEIGHT);
        //move_arm(ZEROHEIGHT);
        //delay(SIMONTIME);
        //Serial.write('D');
        break;
      case 'R':
        break;
    }
  }
}

