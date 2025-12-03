#include <Arduino.h> 
#include "elevating_platform.h"

#define stepper_motor_pulse1 2
#define stepper_motor_direction1 3
#define stepper_motor_pulse2 4
#define stepper_motor_direction2 5
#define sensor1 6
#define sensor2 7
#define stepper_motor_pulse3 8
#define stepper_motor_direction3 9

void init_elevate_motor() {
  pinMode(stepper_motor_pulse1, OUTPUT);
  pinMode(stepper_motor_direction1, OUTPUT);
  pinMode(stepper_motor_pulse2, OUTPUT);
  pinMode(stepper_motor_direction2, OUTPUT);
  pinMode(stepper_motor_pulse3, OUTPUT);
  pinMode(stepper_motor_direction3, OUTPUT);
  Serial.begin(9600);

}

int move_motor(int pul_pin, int dir_pin, int dir, int steps){
    digitalWrite(dir_pin, dir);
    for (int i=0; i<steps; i++){
        digitalWrite(pul_pin, HIGH);
        delayMicroseconds(500);
        digitalWrite(pul_pin, LOW);
        delayMicroseconds(500);
    }
    delay(200);
    Serial.flush();
    Serial.println("motor move completed");
    return 1;
}

void move_motor1(int dir, int steps) {
    move_motor(2, 3, dir, steps);
}

void move_motor2(int dir, int steps){
    move_motor(4, 5, dir, steps);
}

void motor_up(){
    digitalWrite(stepper_motor_direction1, HIGH);
    digitalWrite(stepper_motor_direction2, HIGH);
    for (int i=0; i<200; i++){
        digitalWrite(stepper_motor_pulse1, HIGH);
        digitalWrite(stepper_motor_pulse2, HIGH);
        delayMicroseconds(500);
        digitalWrite(stepper_motor_pulse1, LOW);
        digitalWrite(stepper_motor_pulse2, LOW);
        delayMicroseconds(500);
    }
    Serial.println("up completed");
}

void motor_down(){
    digitalWrite(stepper_motor_direction1, LOW);
    digitalWrite(stepper_motor_direction2, LOW);
    for (int i=0; i<200; i++){
        digitalWrite(stepper_motor_pulse1, HIGH);
        digitalWrite(stepper_motor_pulse2, HIGH);
        delayMicroseconds(400);
        digitalWrite(stepper_motor_pulse1, LOW);
        digitalWrite(stepper_motor_pulse2, LOW);
        delayMicroseconds(400);
    }
    Serial.println("down completed");
}

void motor_home(){
    digitalWrite(stepper_motor_direction1, LOW);
    digitalWrite(stepper_motor_direction2, LOW);

    while (true){
        int sen1_sig = digitalRead(sensor1);
        int sen2_sig = digitalRead(sensor2);
        if ((sen1_sig != 1) && (sen2_sig != 1)){
            digitalWrite(stepper_motor_pulse1, HIGH);
            digitalWrite(stepper_motor_pulse2, HIGH);
            delayMicroseconds(500);
            digitalWrite(stepper_motor_pulse1, LOW);
            digitalWrite(stepper_motor_pulse2, LOW);
            delayMicroseconds(500);
        }
        else if ((sen1_sig != 1) && (sen2_sig == 1)){
            digitalWrite(stepper_motor_pulse2, LOW);
            digitalWrite(stepper_motor_pulse1, HIGH);
            delayMicroseconds(500);
            digitalWrite(stepper_motor_pulse1, LOW);
            delayMicroseconds(500);
        }
        else if ((sen1_sig == 1) && (sen2_sig != 1)){
            digitalWrite(stepper_motor_pulse1, LOW);
            digitalWrite(stepper_motor_pulse2, HIGH);
            delayMicroseconds(500);
            digitalWrite(stepper_motor_pulse2, LOW);
            delayMicroseconds(500);
        }
        else{
            digitalWrite(stepper_motor_pulse1, LOW);
            digitalWrite(stepper_motor_pulse2, LOW);
            delay(1000);
            digitalWrite(stepper_motor_direction1, HIGH);
            digitalWrite(stepper_motor_direction2, HIGH);

            for (int i=0; i<200; i++){
                digitalWrite(stepper_motor_pulse1, HIGH);
                digitalWrite(stepper_motor_pulse2, HIGH);
                delayMicroseconds(500);
                digitalWrite(stepper_motor_pulse1, LOW);
                digitalWrite(stepper_motor_pulse2, LOW);
                delayMicroseconds(500);
            }
            break;
        }
    }
    Serial.println("Homing completed");
}

void motor_left(){
    move_motor(stepper_motor_pulse3, stepper_motor_direction3, HIGH, 11);
}

void motor_right(){
    move_motor(stepper_motor_pulse3, stepper_motor_direction3, LOW, 11);
}
