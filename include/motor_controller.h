#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <FastAccelStepper.h>

#define STEPS_PER_REVOLUTION 200

class MotorController {
public:
    MotorController(uint8_t stepPin, uint8_t dirPin, uint8_t enablePin);

    void begin();
    int32_t currentPosition();
    // Get Speed (RPM)
    uint32_t speed();

    void setCurrentPosition(int32_t position);
    // Set speed (RPM)
    void setSpeed(uint32_t speed);
    // Set acceleration (steps per second^2)
    void setAcceleration(uint32_t acceleration);
    void move(int32_t move);
    void moveTo(int32_t position);
    void runForward();
    void runBackward();
    void enableMotor();
    void disableMotor();
    bool isRunning();
    
private:
    FastAccelStepperEngine m_engine; 
    FastAccelStepper *m_stepper;      // Stepper instance
    uint8_t m_stepPin;
    uint8_t m_dirPin;
    uint8_t m_enablePin;
};

#endif // MOTOR_CONTROLLER_H
