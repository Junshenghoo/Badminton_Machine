#ifndef ELEVATING_PLATFORM_H
#define ELEVATING_PLATFORM_H

void init_elevate_motor();
void move_motor1(int dir, int steps);
void move_motor2(int dir, int steps);
void motor_up();
void motor_down();
void motor_home();
void motor_left();
void motor_right();

#endif
