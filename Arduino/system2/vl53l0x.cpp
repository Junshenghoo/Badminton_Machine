#include "Adafruit_VL53L0X.h"
#include "vl53l0X.h"
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

void vl53l0x_init(){

    // wait until serial port opens for native USB devices
    while (! Serial) {
        delay(1);
    }
    
    Serial.println("Adafruit VL53L0X initializing");
    if (!lox.begin()) {
        Serial.println(F("Failed to boot VL53L0X"));
        while(1);
    }
    // power 
    Serial.println("VL53L0X Initialized"); 
}

int vl53l0x_read(){
    VL53L0X_RangingMeasurementData_t measure;

    lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!

    if (measure.RangeStatus != 4) {  // phase failures have incorrect data
        return measure.RangeMilliMeter;
    } else {
        return 1000;
    }
        
    delay(100);
    return 1000;
}
