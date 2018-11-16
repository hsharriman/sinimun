/*
HC-SR04 Ping distance sensor]
VCC to arduino 5v GND to arduino GND
Echo to Arduino pin 13 Trig to Arduino pin 12
Red POS to Arduino pin 11
Green POS to Arduino pin 10
560 ohm resistor to both LED NEG and GRD power rail
More info at: http://goo.gl/kJ8Gl
Original code improvements to the Ping sketch sourced from Trollmaker.com
Some code and wiring inspired by http://en.wikiversity.org/wiki/User:Dstaub/robotcar
*/
#include <SpeedyStepper.h>
const int MOTOR_STEP_PIN = 3;
const int MOTOR_DIRECTION_PIN = 4;
SpeedyStepper stepper;
#define trigPin 13
#define echoPin 12
float READ_DELAY = 1000; // Currently getting a weird behavior where speed becomes inaccurate at lower delays. No idea why.
float BLINK_DELAY = 250; // in milliseconds
int NOT_PEEKING = 3000; // in milliseconds, time between peeks when scared
const int VEL_THRESHHOLD = 5; // In cm per second
int PEEK_STEPS = 30; // Total guess at this point, number of steps it opens while peeking
int PEEKTIME = 300; // Time in milliseconds it spends peeking
const int stepperSpeed = 50;
const int max_steps = 100; // Total guess at this point, total amount it can open when happy
bool LEDFlag = false;
bool scaredFlag = false;
int currentSteps = 0;
bool shutFlag = true;
bool firstRunthrough = true;
bool peeking = false;

  
void setup() {
  Serial.begin (9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(11, OUTPUT);
  stepper.connectToPins(MOTOR_STEP_PIN, MOTOR_DIRECTION_PIN);
  

}

void loop() {
  stepper.setSpeedInStepsPerSecond(stepperSpeed);
  stepper.setAccelerationInStepsPerSecondPerSecond(stepperSpeed);
  static long last_distance = 0; // "Set the first "previous difference" to 0
  static unsigned long previousReadt = 0;
  static unsigned long previousLEDt = 0;
  static float vel = 0;
  long duration, distance;
  float difference;
  int current_steps = 0;

  if (READ_DELAY <= millis() - previousReadt){
  
  digitalWrite(trigPin, LOW);  // Added this line
  delayMicroseconds(2); // Added this line
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10); // Added this line
  digitalWrite(trigPin, LOW);
  
  
  duration = pulseIn(echoPin, HIGH);
  distance = (duration/2) / 29.1;
  difference = abs(distance - last_distance);
  Serial.print(difference);
  Serial.println(" cm difference");
  last_distance = distance;

  unsigned long currentReadt = millis();
  unsigned long t_elapsed = currentReadt - previousReadt;
  Serial.print(t_elapsed);
  Serial.println(" milliseconds");
  vel = 1000 * difference / READ_DELAY; // In cm per second
  previousReadt = currentReadt;
  Serial.print(vel);
  Serial.println(" cm per second");
  }
if (vel >= VEL_THRESHHOLD){
  scaredFlag = true;
}
else{
  scaredFlag = false;
}


  
if (scaredFlag){
  
  if (!shutFlag && !peeking){
    stepper.moveRelativeInSteps(-currentSteps);
    shutFlag = true;
  }
  else{
    peeking = true;
    if (firstRunthrough){
    unsigned long startTime = millis();
    firstRunthrough = false;
    unsigned long current = millis();
    bool doneYet = false;
    
    
    if (current < startTime+NOT_PEEKING){
      int a =1; // I dont know I just need it to do nothing
     }
    else if (current<startTime+(PEEKTIME/2) && shutFlag){
      stepper.setSpeedInStepsPerSecond(stepperSpeed);
      stepper.setAccelerationInStepsPerSecondPerSecond(stepperSpeed);
      stepper.moveRelativeInSteps(PEEK_STEPS);
      shutFlag = false;
    }
    else if (current<startTime+PEEKTIME && !shutFlag){
      stepper.setSpeedInStepsPerSecond(stepperSpeed);
      stepper.setAccelerationInStepsPerSecondPerSecond(stepperSpeed);
      stepper.moveRelativeInSteps(-PEEK_STEPS);
      shutFlag = true;
      doneYet = true;
    }
    else if (doneYet){
      firstRunthrough = true;
      doneYet = false; 
    }
    }
  }
  
  
}
// OK so that was all if we are still scared. If we aren't...
else{
  if (shutFlag){
    stepper.setSpeedInStepsPerSecond(stepperSpeed);
    stepper.setAccelerationInStepsPerSecondPerSecond(stepperSpeed);
    stepper.moveRelativeInSteps(max_steps);
    currentSteps += max_steps;
    shutFlag = false;
    
  }
  else if (currentSteps < max_steps){
    stepper.setSpeedInStepsPerSecond(stepperSpeed);
    stepper.setAccelerationInStepsPerSecondPerSecond(stepperSpeed);
    stepper.moveRelativeInSteps(max_steps-PEEK_STEPS);
    currentSteps = max_steps;
  }
}
 


  // Printing
  if (distance >= 200 || distance <= 0){
    //Serial.println("Out of range");
    //Serial.println(distance);
  }
  else {
    //Serial.print(distance);
    //Serial.println(" cm");
  }
}
