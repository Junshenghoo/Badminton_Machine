#include <Arduino.h> 
#include "gripper.h"
#include <Servo.h>
#include "vl53l0X.h"

#define servo1 2
#define servo2 3
#define motor_pwm_1 4
#define motor_pwm_2 5
#define motor_dir_1 6
#define motor_dir_2 7
#define dc_motor 8
#define emo 22
#define home_sen 23
#define stepper_motor_tube_pulse 24
#define stepper_motor_tube_direction 26
#define stepper_motor_angle1_pulse 28
#define stepper_motor_angle1_direction 30
#define stepper_motor_angle2_pulse 32
#define stepper_motor_angle2_direction 34
#define limit_switch_1 36
#define limit_switch_2 38

int t = 650;
int t2 = 100;
int val;
int rot_cnt = 0;
int tube = 1;
int motorSpeed = 0;
int previous_angle = 0;
int homing_timeout = 0;

Servo myservo1;
Servo myservo2;

void init_stepper(){
    pinMode(stepper_motor_tube_pulse, OUTPUT);
    pinMode(stepper_motor_tube_direction, OUTPUT);
    pinMode(stepper_motor_angle1_pulse, OUTPUT);
    pinMode(stepper_motor_angle1_direction, OUTPUT);
    pinMode(stepper_motor_angle2_pulse, OUTPUT);
    pinMode(stepper_motor_angle2_direction, OUTPUT);
    pinMode(motor_pwm_1, OUTPUT);
    pinMode(motor_pwm_2, OUTPUT);
    pinMode(motor_dir_1, OUTPUT);
    pinMode(motor_dir_2, OUTPUT);
    pinMode(dc_motor, OUTPUT);

    pinMode(emo, INPUT);
    pinMode(limit_switch_1, INPUT);
    pinMode(limit_switch_2, INPUT);

    digitalWrite(motor_dir_1, HIGH);
    digitalWrite(motor_dir_2, HIGH);
    digitalWrite(dc_motor, LOW);
    
}

void init_gripper(){
    myservo1.attach(servo1);// 0 = open, 180 = close
    myservo2.attach(servo2);// 0 =down, 100 = up
  
    myservo1.write(0);
    myservo2.write(0);
    delay(1000);
}

void init_system2() {
    init_stepper();
    init_gripper();
    Serial.begin(9600);
}

int rotate_feeder(){
    digitalWrite(stepper_motor_tube_direction, HIGH);
    for (int i=0; i<640; i++){
        digitalWrite(stepper_motor_tube_pulse, HIGH);
        delayMicroseconds(500);
        digitalWrite(stepper_motor_tube_pulse, LOW);
        delayMicroseconds(500);
    }
    delay(200);//200
    Serial.flush();
    Serial.println("feeder rotate");
    Serial.println(tube);
    return 1;
}

void motor_homing(int dir, int t){
  digitalWrite(stepper_motor_tube_direction, dir);
  while (true){
    val = digitalRead(home_sen);
    if (val == 1){
      digitalWrite(stepper_motor_tube_pulse, HIGH);
      delayMicroseconds(t);
      digitalWrite(stepper_motor_tube_pulse, LOW);
      delayMicroseconds(t);
      homing_timeout++;
    }
    else{
      break; 
    }
    if(homing_timeout>5000){
        Serial.println("homing timeout");
        homing_timeout = 0;
        break; 
    }
  }
}

void move_steps(int t, int steps){
  for (int i=0; i<steps; i++){
    digitalWrite(stepper_motor_tube_pulse, HIGH);
    delayMicroseconds(t);
    digitalWrite(stepper_motor_tube_pulse, LOW);
    delayMicroseconds(t);
  }
}

void motor_home(){
    motor_homing(1, 500);
    move_steps(500, 100);
    delay(1000);
    motor_homing(0, 1000);
    delay(1000);
    digitalWrite(stepper_motor_tube_direction, 1);
    move_steps(1000, 50);
    Serial.println("tube homing completed");
    previous_angle = 0;
}

int move_motor(){
    digitalWrite(stepper_motor_tube_direction, 1);
    if (tube != 6){
      for (int i=0; i<640; i++){
        digitalWrite(stepper_motor_tube_pulse, HIGH);
        delayMicroseconds(500);
        digitalWrite(stepper_motor_tube_pulse, LOW);
        delayMicroseconds(500);
      }
      delay(200);//200
      Serial.flush();
      tube++;
      Serial.println(tube);
      Serial.println("feeder rotate completed");
      return 1; 
    }
    else{
      motor_homing(1, 500);
      move_steps(500, 90);
      tube = 1;
      Serial.println("feeder rotate completed");
    }
}

void grip(){
    myservo2.write(80);//up
    delay(t2);

    myservo1.write(180);//open
    delay(t);
    
    myservo2.write(0);//down
    delay(t2);

    myservo1.write(5);//close
    delay(t);
}

void dc_motor_run(){
    digitalWrite(dc_motor, HIGH);
}

void dc_motor_stop(){
    digitalWrite(dc_motor, LOW);
}

void set_period(int period){
    if (period == 1){
        t2 = 100;
    }
    else if (period == 2){
        t2 = 100 + 500/2;
    }
    else{
        t2 = ((period * 1000) - 1300)/2;
    }
    Serial.println("period set completed");
}

void adjust_angle(){
    for (int i=0; i<10; i++){
        digitalWrite(stepper_motor_angle1_pulse, HIGH);
        digitalWrite(stepper_motor_angle2_pulse, HIGH);
        delayMicroseconds(500);
        digitalWrite(stepper_motor_angle1_pulse, LOW);
        digitalWrite(stepper_motor_angle2_pulse, LOW);
        delayMicroseconds(500);
    }
}

void stepperAngle(int angle){
    if (angle > previous_angle){
        digitalWrite(stepper_motor_angle1_direction, LOW);
        digitalWrite(stepper_motor_angle2_direction, HIGH);
        adjust_angle();
    }
    else{
        digitalWrite(stepper_motor_angle1_direction, HIGH);
        digitalWrite(stepper_motor_angle2_direction, LOW);
        adjust_angle();
    }
    previous_angle = angle;
    Serial.print("angle: ");
    Serial.print(angle);
    Serial.println(" set completed");
}

void motor_speed(int speed){
    motorSpeed = speed; 
    Serial.println("motor speed set completed");
}

void start_motor(){
    analogWrite(motor_pwm_1, motorSpeed);
    analogWrite(motor_pwm_2, motorSpeed);
}

void stop_motor(){
    analogWrite(motor_pwm_1, 0);
    analogWrite(motor_pwm_2, 0);
}

int emo_sig(){
    return digitalRead(emo);
}

void system_status(int start){
    tube = 1;
    int shuttlecock_status = 0;
    int stop_sig = 0;
    dc_motor_run();
    while (true){
        stop_sig = emo_sig();
        int distance = vl53l0x_read();

        if (stop_sig == 1){
            Serial.println("emo press");
            Serial.println("stop completed");
            stop_motor();
            dc_motor_stop();
            break;
        }
        
        if (distance < 300){
            shuttlecock_status = 1;
        }
        else{
            shuttlecock_status = 0;
        }
        
        if (start == 1){
            if (motorSpeed == 0){
                Serial.println("stop completed, motor speed cannot set to 0 when launching");
                break;
            }
            else{
                start_motor();
            }

            if (shuttlecock_status == 1){
                grip();
                dc_motor_stop();
                stop_motor();
                Serial.println("shuttlecock launching completed");
                break;
            }
            else{
                rotate_feeder();
                tube++;
            }
        }
        else{
            if (motorSpeed == 0){
                Serial.println("stop completed, motor speed cannot set to 0 when launching");
                break;
            }
            else{
                start_motor();
            }

            if (shuttlecock_status == 1){
                grip();
            }
            else{
                if (tube < 6){
                    rotate_feeder();
                }
                tube++;
                Serial.print("tube: ");
                Serial.println(tube);
            }
        }

        if (tube == 7){
            motor_homing(1, 500);
            move_steps(500, 90);
            delay(1000);
            stop_motor();
            Serial.println("all shuttlecock launching completed");
            dc_motor_stop();
            tube = 1;
            break;
        }
    }
}
