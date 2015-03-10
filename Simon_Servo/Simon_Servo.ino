#include <Servo.h>
#include "../simonIIR/simonIIR.h"

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
#define STOP 90
#define GO   125

// Arm Heights
#define SIMONHEIGHT 90
#define RUBIKHEIGHT 70
#define HALFHEIGHT  45
#define ZEROHEIGHT  0

#define SIMONTIME 4500

// Servos
Servo Arm;
Servo End;
Servo Twist;

// Horizontal position
colors_t color = 0;
int command;



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  sense_color_init();
  if(readQD(STRAIGHT) > WHITE){
    move_arm(HALFHEIGHT);
    turn(1,STRAIGHT);
    move_arm(ZEROHEIGHT);
  }
}

void loop() {
  // put your main code here, to run repeatedly: 
  if(Serial.available()){
    command = Serial.read();
    switch(command){
      case 'S':
        move_arm(SIMONHEIGHT);
        color = sense_color();
        move_arm(HALFHEIGHT);
        delay(SIMONTIME);
        turn(color, STRAIGHT);
        move_arm(SIMONHEIGHT);
        move_arm(HALFHEIGHT);
        delay(SIMONTIME);
        move_arm(SIMONHEIGHT);
        move_arm(ZEROHEIGHT);
        delay(SIMONTIME);
        Serial.write('D');
        break;
      case 'R':
        break;
    }
  }
}

void move_arm(int height){
  Arm.write(height);
}

void turn(int steps, int sensor){
  for(int i = 0; i < steps; i++){
    Twist.write(GO);
    while(readQD(sensor) < WHITE);
    while(readQD(sensor) > WHITE);
    Twist.write(STOP);
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
