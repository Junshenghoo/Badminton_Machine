#include "elevating_platform.h"

void setup() {
  Serial.begin(9600);
  init_elevate_motor();
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
      String base = command.substring(0, firstUnderscore); // "move"
      String target = command.substring(firstUnderscore + 1); // "motor1"

      base.trim();
      target.trim();
      
      Serial.print("Parsed base: ");
      Serial.println(base);
      Serial.print("Parsed target: ");
      Serial.println(target);

      if (base == "motor" && target == "up") {
        motor_up();
      } 
      else if (base == "motor" && target == "down") {
        motor_down();
      } 
      else if (base == "motor" && target == "home") {
        motor_home();
      } 
      else if (base == "motor" && target == "left") {
        motor_left();
      } 
      else if (base == "motor" && target == "right") {
        motor_right();
      } 
      else if (base == "motor" && target == "home") {
        motor_home();
      } 
      else {
        Serial.println("[ERROR] Unknown command");
      }

    } else {
      Serial.println("Invalid format (expected: base_target_steps)");
    }
  }
}
