
#include "io.h"
#include <Arduino.h>

void io_setup() {

  pinMode(ENA_PIN, OUTPUT);
  digitalWrite(ENA_PIN,
               HIGH); // GND to disable stepper motor -> safe energy and
                      // avoid heating up the motor if not used
                      // High to do the positioning during setup. HINT: this has
                      // to be changed, otherwise the positioning os only
                      // possible during initial setup
}