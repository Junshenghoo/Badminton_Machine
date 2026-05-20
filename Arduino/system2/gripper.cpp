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

int t = 550;
int t2 = 100;
int val;
int rot_cnt = 0;
int tube = 1;
int motorSpeed = 0;
int previous_angle = 0;
int homing_timeout_time = 0;
int homing_timeout = 0;
int launch_en = 1;
int time_interval = 200;

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

    digitalWrite(motor_dir_1, LOW);
    digitalWrite(motor_dir_2, LOW);
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

void adjust_angle(int home, int times){
    int step;
    int time = 0;
    if (home == 1){
        step = 40;
        time =1000;
    }
    else{
        step = 10;
        time = 500;
    }

    for (int i=0; i<step*times; i++){
        digitalWrite(stepper_motor_angle1_pulse, HIGH);
        digitalWrite(stepper_motor_angle2_pulse, HIGH);
        delayMicroseconds(time);
        digitalWrite(stepper_motor_angle1_pulse, LOW);
        digitalWrite(stepper_motor_angle2_pulse, LOW);
        delayMicroseconds(time);
    }
}

void cylinder_motor_home(int dir, int t){
  digitalWrite(stepper_motor_tube_direction, dir);
  while (true){
    val = digitalRead(home_sen);
    if (val == 1){
      digitalWrite(stepper_motor_tube_pulse, HIGH);
      delayMicroseconds(t);
      digitalWrite(stepper_motor_tube_pulse, LOW);
      delayMicroseconds(t);
      homing_timeout_time++;
    }
    else{
      break; 
    }
    if(homing_timeout_time>5000){
        Serial.println("homing timeout");
        homing_timeout_time = 0;
        homing_timeout = 1;
        break; 
    }
  }
}

void angle_motor_homing(){
    int time = 1000;
    while(1){
        if (digitalRead(limit_switch_1) == 1){
            delay(1000);
            digitalWrite(stepper_motor_angle1_direction, LOW);
            digitalWrite(stepper_motor_angle2_direction, HIGH);
            for (int i=0; i<4; i++){
                adjust_angle(0, 4);
            }
            Serial.println("moving up");
            break;
        }
        else{
            digitalWrite(stepper_motor_angle1_direction, HIGH);
            digitalWrite(stepper_motor_angle2_direction, LOW);
            adjust_angle(1, 1);
            Serial.println("moving to limit switch");
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

void cylindrical_motor_homing(){
    cylinder_motor_home(1, 500);
    if (homing_timeout != 1){
        move_steps(500, 100);
        delay(1000);
        cylinder_motor_home(0, 1000);
        delay(1000);
        digitalWrite(stepper_motor_tube_direction, 1);
        move_steps(1000, 50);
        Serial.println("tube homing done");
    }
    else{
        homing_timeout = 0;
    }
}

void motor_home(){
    cylindrical_motor_homing();
    angle_motor_homing();
    Serial.println("homing completed");
    previous_angle = 16;
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
      cylinder_motor_home(1, 500);
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
    delay(time_interval);

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
    switch (period)
    {
        case 2:
            time_interval = 700;
            break;
        case 3:
            time_interval = 1700;
            break;
        case 4:
            time_interval = 2700;
            break;
        case 5:
            time_interval = 3700;
            break;
        default:
            time_interval = 200;
            break;
    }
    Serial.println("period set completed");
}

void stepperAngle(int angle){
    if (angle > previous_angle){
        digitalWrite(stepper_motor_angle1_direction, LOW);
        digitalWrite(stepper_motor_angle2_direction, HIGH);
        adjust_angle(0, 1);
    }
    else{
        digitalWrite(stepper_motor_angle1_direction, HIGH);
        digitalWrite(stepper_motor_angle2_direction, LOW);
        adjust_angle(0, 1);
    }
    previous_angle = angle;
    Serial.print("angle: ");
    Serial.print(angle);
    Serial.println(" set completed");
}

void movePosY(int angle){
    int steps_y;
    if ((angle > 21) || (angle <0)){
        Serial.print("y over limit");
    }
    else{
        if ((angle - previous_angle) > 0){
            steps_y = angle - previous_angle;
            digitalWrite(stepper_motor_angle1_direction, LOW);
            digitalWrite(stepper_motor_angle2_direction, HIGH);
            adjust_angle(0, steps_y);
        }
        else{
            steps_y = previous_angle - angle;
            digitalWrite(stepper_motor_angle1_direction, HIGH);
            digitalWrite(stepper_motor_angle2_direction, LOW);
            adjust_angle(0, steps_y);
        }
        previous_angle = angle;
        Serial.println("move zone completed");
    }
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

void resetAutoMode(){
    tube = 1;
    launch_en = 1;
    Serial.println("[auto mode] sys2 launch enable reset complete");
}

void autoMode(){
    if (launch_en == 1){
        while (1){
            int shuttlecock_status = 0;
            int distance = 0;
            int stop_sig = 0;
            
            stop_sig = emo_sig();
            distance = vl53l0x_read();
            Serial.print("mo_sig: ");
            Serial.println(stop_sig);

            if (stop_sig == 1){
                dc_motor_stop();
                stop_motor();
                Serial.println("[auto mode] emo pressed");
                break;
            }
            else{
                dc_motor_run();
                start_motor();
                if (distance < 300){
                    shuttlecock_status = 1;
                }
                else{
                    shuttlecock_status = 0;
                }

                if (shuttlecock_status == 1){
                    grip();
                    Serial.println("[auto mode] launch completed");
                    delay(800);
                    break;
                }
                else{
                    if (tube < 6){
                        rotate_feeder();
                    }
                    tube++;
                }

                if (tube == 7){
                    cylinder_motor_home(1, 500);
                    move_steps(500, 90);
                    delay(1000);
                    stop_motor();
                    dc_motor_stop();
                    Serial.println("[auto mode] all shuttlecock launching complete and sys2 launching disable");
                    tube = 1;
                    launch_en = 0;
                    break;
                }
            }
        }
    }
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
                delay(1500);
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
            }
        }

        if (tube == 7){
            cylinder_motor_home(1, 500);
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
