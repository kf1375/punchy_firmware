
#include <Arduino.h>
#include <MobaTools.h>

#include "StepperController.h"


// Definition of Pin Nummers and Variabel

  //From main.ccp

extern bool startExtension;
extern bool base_sin; //Base Force in dN
extern bool f_sin; //Frequency in pbm
extern bool amp_sin; //Amplitude Force in dN
extern bool y_sin; //Sin output variable

// Stepper Motor
const int stepPin = 32;
const int dirPin = 33;
const int enablPin = 35;

// const int limitRightPin = 12;
// const int limitLeftPin = 13;

const int LimitFront = 25;
const int LimitBack = 26;
const int OverLoad = 27;



// Definition der Schrittwinkel und Geschwindigkeit
const float stepAngle = 1.8;              // Schrittwinkel des Motors
const int stepsPerRev = 4000 / stepAngle; // Schritte pro Umdrehung

const int maxSpeed = 2000;         // Define maxSpeed variable
const int calibrationSpeed = 500; // Speed during calibration
const int maxMMperSec = 5;          // Defines the Speed of the Stepper in mm per sec (thread pitch is 10mm)

const int revolutionsCalibration = 2; // how many rev does to motor perform during calibration
long int totalSteps = 0;

// More Stepper Motor definitions
MoToStepper myStepper(stepsPerRev, STEPDIR); // create a stepper instance, STEPDIR is only valid value (otherwise the motor doesnßt spin)

// Serial
int incomingByte = 0; // for incoming serial data
int MotorValue = 0;

// Functions

// Stepper Motor
// Limit and OverLoad Switches. Hint: Dont use Serial.Print in Interrup because it is to slow

    void resetPIDvalues(){
      startExtension = false;
      base_sin = 0; //Base Force in dN
      f_sin = 0; //Frequency in pbm
      amp_sin = 0; //Amplitude Force in dN
      y_sin = 0; //Sin output variable
    }

    void FrontStop()
    {
    //  StepperControl(0, maxSpeed);
    //  totalSteps = myStepper.currentPosition();
      myStepper.stop();
      resetPIDvalues();
    // Serial.print("Front");
     }
    void BackStop(){
    //  StepperControl(0, maxSpeed);
      myStepper.stop();
      resetPIDvalues();
      myStepper.setZero();
    }
    void OverLStop(){
    //  StepperControl(0, maxSpeed);
      myStepper.stop();
      resetPIDvalues();
    }

   
    int getMMperSec(int stepsSec){
      return stepsSec / (stepsPerRev / 10);
    }

    void StepperControl(int SpeedPIDpercentage, int maxSpeed){  //Controlls the motor with given PID value

        if(SpeedPIDpercentage >= -100 && SpeedPIDpercentage <= 100){ //Checks if PID values are within range
          int direction = 0;
          int mag = abs(SpeedPIDpercentage);    //Calculates the absolut value
          // Serial.print("Errechnete Stepps/sec: ");
          // Serial.println(map(mag, 0, 100, 0, maxSpeed));
          // Serial.print("Speed mm/sec;");
          // Serial.println(getMMperSec(map(mag, 0, 100, 0, maxSpeed)));

          myStepper.setSpeed(2 * map(mag, 0, 100, 0, maxSpeed)); // NOTE: for Tetsting speed is increased by 2! maps the PID value to the given motor Speed Range

          if(SpeedPIDpercentage < 0){     //Defines the direction of the stepper motor
            direction = -1;
          }else if(SpeedPIDpercentage > 0) {
            direction = 1;
          }else{
            direction = 0;
          }
          myStepper.rotate(direction);

          // Serial.print("der Motor fährt mit:");
          // Serial.println(SpeedPIDpercentage);
        }else{
          myStepper.stop();
          // Serial.println("PID value out of -100 to 100 Range!");
        }
    }

    void searchLimitSwitchBack(){  // turns the motor forward and then backwards. Back limit switch interrupt is used to stop the motor and set the Zero position

      myStepper.setSpeed(calibrationSpeed);
      myStepper.move(revolutionsCalibration * stepsPerRev);   //moves away from the limit switch 

      while (myStepper.stepsToDo() > 0) {
        // Serial.println("Calibration in Progress: Moving forward");  // wait until stepper reaches limit switch
      }
      myStepper.rotate(-1);  //Interrup will stop the movement and call .setZero 
   
  }

    void searchLimitSwitchFront(){

      myStepper.setSpeed(calibrationSpeed);
      myStepper.rotate(1);
      
    }
    void calibrateLimitSwitches(){
      
      if(digitalRead(LimitBack) == HIGH && digitalRead(LimitFront) == HIGH){
        
        searchLimitSwitchFront();
        searchLimitSwitchBack();

      }else{
        // Serial.println("Back Limit Switch reached");
      }

    }

    void StepperControllerSetup()
    {

      // Stepper Motor
      myStepper.attach(stepPin, dirPin);
      myStepper.setMaxSpeed(int(maxSpeed));
      myStepper.setSpeed(100); // 20 rev/min (if stepsPerRev is set correctly)

      //Input Pins
      pinMode(LimitFront, INPUT_PULLUP);
      pinMode(LimitBack, INPUT_PULLUP);
      pinMode(OverLoad, INPUT_PULLUP);
      pinMode(enablPin, OUTPUT);
      

      //Interrupts for Limit Switches
      attachInterrupt(digitalPinToInterrupt(LimitFront), FrontStop, FALLING);  //Interrupt to call funtion to stop Motor
      attachInterrupt(digitalPinToInterrupt(LimitBack), BackStop, FALLING);
      attachInterrupt(digitalPinToInterrupt(OverLoad), OverLStop, FALLING);

}



/* //Vorwärts Rückwärts

// Funktion zum Drehen des Motors mit einer bestimmten Geschwindigkeit
void rotateMotor(int);

void rotateMotor(int speed) {
  digitalWrite(dirPin, (speed > 0) ? HIGH : LOW); // Setze die Drehrichtung

  // Führe die Schritte aus, um den Motor zu drehen
  for (int i = 0; i < abs(speed); i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(1000); // Pausiere für die gewünschte Geschwindigkeit
    digitalWrite(stepPin, LOW);
    delayMicroseconds(1000);
  }

  Serial.println("Motor gestoppt");
}

void setup() {
  // Setze die Pins als Output
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);

  // Setze die Pins als Input mit internem Pullup-Widerstand
  pinMode(limitRightPin, INPUT_PULLUP);
  pinMode(limitLeftPin, INPUT_PULLUP);

  // Starte die Serielle Kommunikation für Debugging (optional)
  Serial.begin(9600);
}

void loop() {
  // Überprüfe den Status des rechten Limitschalters
  if (digitalRead(limitRightPin) == LOW) {
    // Wenn der rechte Limitschalter aktiviert ist, drehe den Motor nach links
    rotateMotor(motorSpeed);
  }

  // Überprüfe den Status des linken Limitschalters
  if (digitalRead(limitLeftPin) == LOW) {
    // Wenn der linke Limitschalter aktiviert ist, drehe den Motor nach rechts
    rotateMotor(-motorSpeed);
  }
}

*/


