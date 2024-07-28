
#include <Arduino.h>
#include <MobaTools.h>

#include "driver.h"
#include "io.h"

float calc_rpm_from_mm_s(float mm_per_sec) {
  return (mm_per_sec * 60) / (THREAD_PITCH);
}

bool safeRotate(int direction, bool LimitBackStatus, bool LimitFrontStatus) {
  bool safeRotate = false; // Defines if the motor is allowed to rotate in the
                           // requested direction
  if (direction < 0) { // Defines the direction of the stepper motor
    direction = -1;
    if (LimitBackStatus) {
      myStepper.stop();
    } else {
      myStepper.rotate(-1);
      safeRotate = true;
    }
  } // Checks if the back limit switch is pressed

  else if (direction > 0) {
    if (LimitFrontStatus) {
      myStepper.stop();
    } else {
      myStepper.rotate(1);
      safeRotate = true;
    } // Checks if the back limit switch is pressed
  } else {
    myStepper.stop();
    // Serial.println("Limit Switch is pressed, Motor stopped");
  }
  return safeRotate;
}

void stepperControlPID(
    double PIDoutput, int max_pid_speed_mms, bool LimitBackStatus,
    bool LimitFrontStatus) { // PID Output is error, most in the range of 2 - 3
  if (PIDoutput >= pid_clamp_low &&
      PIDoutput <= pid_clamp_high) { // Checks if PID values are within range
    int direction = 0;
    if (PIDoutput > 0) {
      direction = -1;
    } // --> -1 direction is backwards --> force on rope increases
    if (PIDoutput < 0) {
      direction = 1;
    }
    if (PIDoutput == 0) {
      direction = 0;
    }

    double mag = abs(PIDoutput); // Calculates the absolut value
    // Serial.print("Errechnete Stepps/sec: ");
    // Serial.println(map(mag, 0, 100, 0, maxSpeed));
    // Serial.print("Speed mm/sec;");
    // Serial.println(getMMperSec(map(mag, 0, 100, 0, maxSpeed)));

    myStepper.setSpeed(map_double_limit(
        mag, 0.0, pid_clamp_high, 0.0,
        double(calc_rpm_from_mm_s(
            max_pid_speed_mms)))); // Arduino map function can only use int
    safeRotate(direction, LimitBackStatus, LimitFrontStatus);

  } else {
    Serial.println("Error: PIDoutput out of range");
  }
}

void driver_setup() {
  myStepper.setMaxSpeed(
      (MAX_SPEED)); // Due to MoBaTools necessary set speed in rpm*10
  myStepper.setSpeed(REVERSE_SPEED); // as default
}

double map_double_limit(double x, double in_min, double in_max, double out_min,
                        double out_max) {
  double value =
      (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  if (value > out_max) {
    value = out_max;
  }
  return value;
}