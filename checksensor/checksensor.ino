#include <SpeedyStepper.h>

const int MOTOR_STEP_PIN = 8;
const int MOTOR_DIRECTION_PIN = 9;
const int stepperSpeed = 50 * 16;
const int max_steps = 100 * 16;
const int trigPin = 11;
const int echoPin = 10;
const int ultraPin = 12;
const int micPin = 13;
const int microInPin = 7;
const int distThresh = 40;
const int velThresh = 200; // TODO

//measurement variables
long duration;
float distance, vel;
unsigned long lastVT;
int dT = 100;
long lastDist = 0;
int currentSteps = 0;
bool scaredFlag = false;
bool timTrig = false;
bool trustTrig = false;
//scared variables
unsigned long timeSinceScare;
unsigned long scaredInt = 3000;


//timid variables
unsigned long timidTime;
unsigned long timidInt = 10000;

SpeedyStepper stepper;



void setup()
{
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ultraPin, OUTPUT);
  pinMode(micPin, OUTPUT);
  pinMode(microInPin, INPUT);
  Serial.begin(9600);
  stepper.connectToPins(MOTOR_STEP_PIN, MOTOR_DIRECTION_PIN);
}


void loop()
{
  stepper.setSpeedInStepsPerSecond(stepperSpeed);
  stepper.setAccelerationInStepsPerSecondPerSecond(stepperSpeed);

  //Collect distance sensor data
  if (millis() - lastVT >= dT) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    duration = pulseIn(echoPin, HIGH);
    distance = (duration / 2) / 29.1;
    float diff = distance - lastDist;
    vel = abs(diff);  //calculate the "velocity" for every dT
    Serial.println(vel);
    if (abs(distance - lastDist) >= velThresh) {
      Serial.println("SO FAST.");
    }
    lastDist = distance; //save current distance for next iteration
    lastVT = millis();
  }

  //STOIC:
  if (distance > distThresh && vel < velThresh) {
    Serial.println("STOIC");
    digitalWrite(ultraPin, LOW);
    digitalWrite(micPin, LOW);
    timTrig = false;
    trustTrig = false;
    //stepper.moveRelativeInSteps(100 * 16); /// TODO
  }

  //TIMID
  if (distance < distThresh && vel < velThresh && timTrig == true) {
    timidTime = millis();
    timTrig = false;
  }
 //TRUSTING:
  if (trustTrig == true && millis() - timidTime > timidInt) {
    Serial.println("TRUSTING");
    digitalWrite(ultraPin, LOW);
    digitalWrite(micPin, HIGH);
    timTrig = false;
  }
  //TIMID
  if (millis() - timidTime < timidInt) {
    if (distance < distThresh && vel < velThresh) {
      Serial.println("TIMID");
      digitalWrite(ultraPin, HIGH);
      digitalWrite(micPin, LOW);
      trustTrig = true;
    }
  }


  //SCARED:
  if (distance <= 10 or vel > velThresh) {    //add microphone input
    Serial.println("Just spooked");
    digitalWrite(ultraPin, HIGH);
    digitalWrite(micPin, HIGH);
    timeSinceScare = millis();
    stepper.moveRelativeInSteps(100 * 16);   ///TODO
    stepper.moveRelativeInSteps(-100 * 16); ///TODO
    Serial.println("motors done");
    timTrig = true;
    trustTrig = false;
  }
}

