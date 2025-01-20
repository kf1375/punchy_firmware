#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <FastAccelStepper.h>

class MotorController {
public:
    MotorController(uint8_t stepPin, uint8_t dirPin);

    void begin();
    int32_t currentPosition();
    // Set speed (steps per second)
    void setSpeed(uint32_t speed);
    // Set acceleration (steps per second^2)
    void setAcceleration(uint32_t acceleration);
    void move(int32_t move);
    void moveTo(int32_t position);
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
