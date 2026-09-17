#pragma once
#include <Arduino.h>
#include <ESP32Servo.h>
#include "config.h"

static Servo panServo;

void move_servo(int pin, int pulse, int delay_time);