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
#define sensor3 10
#define lift_motor_period 700

int previous_angle = 0;

void init_elevate_motor() {
  pinMode(stepper_motor_pulse1, OUTPUT);
  pinMode(stepper_motor_direction1, OUTPUT);
  pinMode(stepper_motor_pulse2, OUTPUT);
  pinMode(stepper_motor_direction2, OUTPUT);
  pinMode(stepper_motor_pulse3, OUTPUT);
  pinMode(stepper_motor_direction3, OUTPUT);

  pinMode(sensor1, INPUT);
  pinMode(sensor2, INPUT);
  pinMode(sensor3, INPUT);
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
    move_motor(stepper_motor_pulse1, stepper_motor_direction1, dir, steps);
}

void move_motor2(int dir, int steps){
    move_motor(stepper_motor_pulse2, stepper_motor_direction2, dir, steps);
}

void motor_up(){
    digitalWrite(stepper_motor_direction1, LOW);
    digitalWrite(stepper_motor_direction2, LOW);
    for (int i=0; i<200; i++){
        digitalWrite(stepper_motor_pulse1, HIGH);
        digitalWrite(stepper_motor_pulse2, HIGH);
        delayMicroseconds(lift_motor_period);
        digitalWrite(stepper_motor_pulse1, LOW);
        digitalWrite(stepper_motor_pulse2, LOW);
        delayMicroseconds(lift_motor_period);
    }
    Serial.println("up completed");
}

void motor_down(){
    digitalWrite(stepper_motor_direction1, HIGH);
    digitalWrite(stepper_motor_direction2, HIGH);
    for (int i=0; i<200; i++){
        digitalWrite(stepper_motor_pulse1, HIGH);
        digitalWrite(stepper_motor_pulse2, HIGH);
        delayMicroseconds(lift_motor_period);
        digitalWrite(stepper_motor_pulse1, LOW);
        digitalWrite(stepper_motor_pulse2, LOW);
        delayMicroseconds(lift_motor_period);
    }
    Serial.println("down completed");
}

void left_right_motor_home(){
    int steps = 360;
    digitalWrite(stepper_motor_direction3, HIGH);

    while (true){
        int sen3_sig = digitalRead(sensor3);
        digitalWrite(stepper_motor_pulse3, HIGH);
        delayMicroseconds(1500);
        digitalWrite(stepper_motor_pulse3, LOW);
        delayMicroseconds(1500);
        if (sen3_sig == 1){
            delay(1000);
            digitalWrite(stepper_motor_direction3, LOW);
            for (int i=0; i<steps; i++){
                digitalWrite(stepper_motor_pulse3, HIGH);
                delayMicroseconds(500);
                digitalWrite(stepper_motor_pulse3, LOW);
                delayMicroseconds(500);
            }
            Serial.println("left_right_motor_home done");
            break;
        }
    }
}

void lift_motor_home(){
    digitalWrite(stepper_motor_direction1, HIGH);
    digitalWrite(stepper_motor_direction2, HIGH);

    while (true){
        int sen1_sig = digitalRead(sensor1);
        int sen2_sig = digitalRead(sensor2);
        if ((sen1_sig != 1) && (sen2_sig != 1)){
            digitalWrite(stepper_motor_pulse1, HIGH);
            digitalWrite(stepper_motor_pulse2, HIGH);
            delayMicroseconds(lift_motor_period);
            digitalWrite(stepper_motor_pulse1, LOW);
            digitalWrite(stepper_motor_pulse2, LOW);
            delayMicroseconds(lift_motor_period);
        }
        else if ((sen1_sig != 1) && (sen2_sig == 1)){
            digitalWrite(stepper_motor_pulse2, LOW);
            digitalWrite(stepper_motor_pulse1, HIGH);
            delayMicroseconds(lift_motor_period);
            digitalWrite(stepper_motor_pulse1, LOW);
            delayMicroseconds(lift_motor_period);
        }
        else if ((sen1_sig == 1) && (sen2_sig != 1)){
            digitalWrite(stepper_motor_pulse1, LOW);
            digitalWrite(stepper_motor_pulse2, HIGH);
            delayMicroseconds(lift_motor_period);
            digitalWrite(stepper_motor_pulse2, LOW);
            delayMicroseconds(lift_motor_period);
        }
        else{
            digitalWrite(stepper_motor_pulse1, LOW);
            digitalWrite(stepper_motor_pulse2, LOW);
            delay(1000);
            digitalWrite(stepper_motor_direction1, LOW);
            digitalWrite(stepper_motor_direction2, LOW);

            for (int i=0; i<200; i++){
                digitalWrite(stepper_motor_pulse1, HIGH);
                digitalWrite(stepper_motor_pulse2, HIGH);
                delayMicroseconds(lift_motor_period);
                digitalWrite(stepper_motor_pulse1, LOW);
                digitalWrite(stepper_motor_pulse2, LOW);
                delayMicroseconds(lift_motor_period);
            }
            Serial.println("lift_motor_home done");
            break;
        }
    }
}

void motor_home(){
    left_right_motor_home();
    lift_motor_home();
    Serial.println("Homing completed");
}

void motorLeftRight(int angle){
    int steps_x;
    if ((angle > 50) || (angle <-50)){
        Serial.print("x over limit");
    }
    else{
        if ((angle - previous_angle) > 0){
            steps_x = angle - previous_angle;
            previous_angle = angle;
            Serial.print("current_pos_x: ");
            move_motor(stepper_motor_pulse3, stepper_motor_direction3, 1, steps_x*11);
        }
        else{
            steps_x = previous_angle - angle;
            previous_angle = angle;
            Serial.print("current_pos_x: ");
            move_motor(stepper_motor_pulse3, stepper_motor_direction3, 0, steps_x*11);
        }
    }
}

