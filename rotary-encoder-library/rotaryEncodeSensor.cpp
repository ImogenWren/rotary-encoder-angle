

#include "rotaryEncodeSensor.h"


void rotaryEncodeSensor::begin() {
  if (encoderType == ONE_WIRE || encoderType == TWO_WIRE) {
    pinMode(interruptPinA, INPUT_PULLUP);
    //  attachInterrupt(digitalPinToInterrupt(ENCODER_WHITE), doEncoderA, CHANGE);  // interrupt 0 is pin 2
  }
  if (encoderType == TWO_WIRE) {
    pinMode(interruptPinB, INPUT_PULLUP);
    //  attachInterrupt(digitalPinToInterrupt(ENCODER_GREEN), doEncoderB, CHANGE);  // interrupt 0 is pin 2
  }
}

//void rotaryEncodeSensor::loopFunc(){
//  encoderPos += (A_state == B_state) ? +1 : -1;
//  rotaryEncodeSensor::encoderWrap();/
//}


void rotaryEncodeSensor::printPosStats() {
  char buffer[64];
  char timeString[16];
  char degString[12];
  char radString[12];
  dtostrf(secondsElapsed(), 8, 3, timeString);  // Convert float to char string as sprintf() does not work with floats in arduino
  dtostrf(encoderDeg, 8, 2, degString);         // Convert float to char string as sprintf() does not work with floats in arduino
  dtostrf(encoderRad, 8, 2, radString);
  sprintf(buffer, "%s -> %5i, %s deg, %s rad.", timeString, encoderPos, degString, radString);  // Do it this way to keep coloumns of data in line.
  Serial.println(buffer);
}



void rotaryEncodeSensor::printSignalStats() {
  char buffer[128];
  char timeString[16];
  char HzString[12];
  char dutyString[12];
  dtostrf(secondsElapsed(), 8, 3, timeString);  

  if (signal_Hz < 99) {                    
    dtostrf(signal_Hz, 4, 2, HzString);   // More precision for lower Hz vals
  } else {
    dtostrf(signal_Hz, 4, 0, HzString);   // tidy up Hz print for higher values
  }
  dtostrf(signal_duty, 4, 2, dutyString);
  sprintf(buffer, "Time: %s -> High: %4lu, Low: %4lu, Period: %3lu, Hz: %s, Duty: %s", timeString, signal_time_high, signal_time_low, signal_period, HzString, dutyString);  //
  Serial.println(buffer);
}

float rotaryEncodeSensor::secondsElapsed() {
  float seconds = millis() / 1000.0;
  return seconds;
}



void rotaryEncodeSensor::convertPosDeg() {
  encoderDeg = (360.0 / float(pulses_per_revolution * 2)) * float(encoderPos);
}

void rotaryEncodeSensor::convertPosRad() {
  encoderRad = ((2 * PI) / float(pulses_per_revolution * 2)) * float(encoderPos);
}


void rotaryEncodeSensor::analyseSignal() {
  if (ISR_triggered) {
    if (!A_state) {  // if A_state is currently low, then we are calculating the HIGH period
      signal_time_high = currentSampleTime_uS - previousSampleTime_uS;
      //  Serial.print("Completed HIGH cycle: ");
      // Serial.println(signal_time_high);
    } else {
      signal_time_low = currentSampleTime_uS - previousSampleTime_uS;
      //  Serial.print("Completed Low  cycle: ");
      //   Serial.println(signal_time_low);
    }
    // Serial.print("currentTime: ");
    // Serial.print(currentSampleTime_uS);
    // Serial.print(", PreviousTime: ");
    // Serial.print(previousSampleTime_uS);
    //  Serial.print(", Difference: ");
    //  Serial.println(currentSampleTime_uS - previousSampleTime_uS);
    signal_period = signal_time_low + signal_time_high;
    signal_Hz = 1000000.0 / float(signal_period);
    signal_duty = float(signal_time_high) / float(signal_period);

    ISR_triggered = false;
  }
}


void rotaryEncodeSensor::calculateSpeed_radSec() {
}

void rotaryEncodeSensor::calculateSpeed_rpm() {
}

void rotaryEncodeSensor::calculateSpeed_rps() {
}




void rotaryEncodeSensor::encoderWrap(void) {
  if (encoderPos > pulses_per_revolution) {
    encoderPos -= pulses_per_revolution * 2;
  } else if (encoderPos < (-1 * pulses_per_revolution)) {
    encoderPos += pulses_per_revolution * 2;
  }
}

void rotaryEncodeSensor::_isr_A() {
  A_state = digitalRead(interruptPinA);
  previousSampleTime_uS = currentSampleTime_uS;
  currentSampleTime_uS = micros();
  encoderLastPos = encoderPos;
  // could below be done outside of ISR?
  encoderPos += (A_state == B_state) ? +1 : -1;
  rotaryEncodeSensor::encoderWrap();
  ISR_triggered = true;
}