#include "servo_handler.h"

void move_servo(int pin, int pulse, int delay_time) {
    panServo.attach(pin);
    panServo.write(pulse);
    delay(delay_time);
    panServo.write(ServoConfig::no_rotation);
    delay(50);
    panServo.detach();
}