// This file contains all the Stepper related functions and definitions
// The MAX_SPEED for safety is set to rpm value that should never be reached by
// the motor

// HOW to USE: create a driver object in the main file like this: driver
// myStepper; if needed call the driver_setup(). But it only defines default
// speed. Set the pins in th IO.h file

#ifndef DRIVE_H
#define DRIVE_H

#include "io.h"
#include <Arduino.h>
#include <MobaTools.h>

#define MAX_SPEED 1000
// Max Speed in rpm --> 20 mm/s at 10 mm pitch - this is used to define
// the overall max speed of the motor using the maxSpeed() function from
// moabtools
#define REVERSE_SPEED 30
// rpm that the motor will rotate backwards when "STOP" is pressed on the
// GUI
#define STEPS_PER_REV 200    // Step per revolution set in the DM556T driver
#define THREAD_PITCH 10      // Pitch in mm
#define CALIBRATION_SPEED 25 // Speed for the calibration process
#define STEPS_PER_MM (STEPS_PER_REV / THREAD_PITCH) // Steps per mm

extern double pid_clamp_low;
extern double pid_clamp_high;

class driver {
public:
  MoToStepper myStepper;

  driver() : myStepper(STEPS_PER_REV, STEPDIR) {
    myStepper.attach(STEP_PIN, DIR_PIN);
  } // constructor for the driver class
  // functions for the mobatools stepper class --> used in the driver class
  void setMaxSpeed(int maxSpeed) {
    myStepper.setMaxSpeed(maxSpeed * 10);
  } // mobatools speed is set in rpm*10
  void setSpeed(int speed) {
    myStepper.setSpeed(speed * 10);
  } // mobatools speed is set in rpm*10
  void rotate(int direction) { myStepper.rotate(direction); }
  void stop() { myStepper.stop(); }
  long int currentPosition() { return myStepper.currentPosition(); }
  void setZero() { myStepper.setZero(); }
  void setRampLen(int rampLen) { myStepper.setRampLen(rampLen); }
  void moveTo(long position) { myStepper.moveTo(position); }
  uint8_t moving() { return myStepper.moving(); }
  long distanceToGo() { return myStepper.distanceToGo(); }
  void set_Zero() { myStepper.setZero(); }
  void write(long angle) { myStepper.write(angle); }
  void move(long stepcount) { myStepper.move(stepcount); }
};

extern driver myStepper;
// extern MoToStepper myStepper;

void driver_setup();

/**
 * @brief calculates the speed in rmp from the mm/s
 * @param mm_per_sec: speed in mm/s
 * @return rotation speed in rpm
 */
float calc_rpm_from_mm_s(float mm_per_sec);

/** starts rotation if no limit switch in the decired direction is pressed,
 * returns false if pressed.  //Prevents the motor form stepping over a limit
 * switch --> for example: if the motor is in the rear positon it cannot rotate
 * further backwards
 * @param direction: 1 for forward, -1 for backwards
 * @param LimitBackStatus: recives the current limit switch status at fucntion
 * call
 * @param LimitFrontStatus: recives the current limit switch status at fucntion
 * call
 * @return false if limit switch is pressed
 */
bool safeRotate(
    int direction, bool LimitBackStatus,
    bool LimitFrontStatus); // starts rotation if no limit switch in the decired
                            // direction is pressed, returns false if pressed

/**
 * @brief starts the motor to rotate in the direction and with the speed
 * according to the PID output
 * @param pid_output: PID output value in the range of -10 to 10
 * @param max_pid_speed_mms: the maximum speed the motor is allowed to travel in
 * mm per sec. pid output is mapped to the range of 0 to max_pid
 * @param LimitSwitchFront: recives the current limit switch status at fucntion
 * call --> to call safeRotate
 * @param LimitSwitchBack: recives the current limit switch status at fucntion
 * call --> to call safeRotate
 *
 */
void stepperControlPID(double pid_output, int max_pid_speed_mms,
                       bool LimitSwitchFront, bool LimitSwitchBack);

/**
 * @brief same Template function as above but with a limit for the output value
 * --> to avoid values greater than out_max
 * @return the mapped value but never greater than out_max
 */
double map_double_limit(double x, double in_min, double in_max, double out_min,
                        double out_max);

#endif // MY_MOTOR_CONTROL_H
