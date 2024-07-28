

#ifndef MY_MOTOR_CONTROL_H
#define MY_MOTOR_CONTROL_H

#include <Arduino.h>
#include <MobaTools.h>

extern const int maxSpeed; // Declare maxSpeed variable

class StepperMotor {
public:
    StepperMotor(int stepPin, int dirPin);
    void attach(int stepPin, int dirPin);
    void setMaxSpeed(int maxSpeed);
    void setSpeed(int speed);
    void rotate(int direction);
    void stop();
    long int currentPosition();
    void setZero();
};

extern MoToStepper myStepper;

void FrontStop();
void BackStop();
void OverLStop();
void StepperControllerSetup();
void StepperControl(int SpeedPIDpercentage, int maxSpeed);
void searchLimitSwitchBack();
void searchLimitSwitchFront();
void calibrateLimitSwitches();

#endif // MY_MOTOR_CONTROL_H
