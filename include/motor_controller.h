#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <FastAccelStepper.h>

class MotorController {
public:
    MotorController(uint8_t stepPin, uint8_t dirPin);

    void begin();
    // Set speed (steps per second)
    void setSpeed(uint32_t speed);
    // Set acceleration (steps per second^2)
    void setAcceleration(uint32_t acceleration);
    void moveSteps(int32_t steps);
    void enableMotor();
    void disableMotor();
    bool isRunning();
    
private:
    FastAccelStepperEngine engine; 
    FastAccelStepper *stepper;      // Stepper instance
    uint8_t stepPin;
    uint8_t dirPin;
};

#endif // MOTOR_CONTROLLER_H
