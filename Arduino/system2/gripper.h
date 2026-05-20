#ifndef GRIPPER_H
#define GRIPPER_H

void init_system2();
void system_status(int start);
int move_motor();
void motor_home();
void set_period(int period);
void stepperAngle(int angle);
void motor_speed(int motorSpeed);
void movePosY(int angle);
void autoMode();
void resetAutoMode();
void stop_motor();
void dc_motor_stop();

#endif
