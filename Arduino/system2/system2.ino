#include "gripper.h"
#include "vl53l0X.h"

void setup() {
  Serial.begin(9600);
  init_system2();
  vl53l0x_init();
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    Serial.print("Received: ");
    Serial.println(command);

    // Split command by '_'
    int firstUnderscore = command.indexOf('_');


    if (firstUnderscore != -1) {
      String base = command.substring(0, firstUnderscore);
      String target = command.substring(firstUnderscore + 1);

      base.trim();
      target.trim();
      
      Serial.print("Parsed base: '");
      Serial.print(base);
      Serial.println("'");

      Serial.print("Parsed target: '");
      Serial.print(target);
      Serial.println("'");

      if (base.equals("system") && target.equals("one")) {
        system_status(1);
      } 
      else if (base.equals("system") && target.equals("all")) {
        system_status(0);
      } 
      else if (base.equals("move") && target.equals("motor")) {
        move_motor();
      }
      else if (base == "motor" && target == "home") {
        motor_home();
      }
      else if (base == "motorSpeed") {
        motor_speed(target.toInt());
      }
      else if (base == "period") {
        set_period(target.toInt());
      } 
      else if (base == "stepperAngle") {
        stepperAngle(target.toInt());
      } 
      else {
        Serial.println("[ERROR] Unknown command");
      }

    } else {
      Serial.println("Invalid format");
    }
  }
}
