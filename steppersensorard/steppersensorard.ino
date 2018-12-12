#include <SpeedyStepper.h>

const int MOTOR_STEP_PIN = 8;
const int MOTOR_DIRECTION_PIN = 9;
const int stepperSpeed = 50 * 16;
const int maxSteps = 70 * 16;  //TODO
const int trigPin = 11;
const int trigPin2 = 0;
const int trigPin3 = 2;
const int trigPin4 = 4;
const int echoPin = 10;
const int echoPin2 = 1;
const int echoPin3 = 3;
const int echoPin4 = 5;
const int ultraPin = 12;
const int micPin = 13;
const int microInPin = 7;
const int distThresh = 40;
const int velThresh = 200; // TODO

//measurement variables
long duration, duration2, duration3, duration4;
float distance, distance4, distance2, distance3;
float vel, vel2, vel3, vel4;
unsigned long lastVT;
int dT = 100;
long lastDist = 0;
long lastDist2 = 0;
long lastDist3 = 0;
long lastDist4 = 0;

int min_d = 0;
int max_v = 0;

//motor variables
int currentSteps = 70 * 16;
bool openFlag = true;
bool closedFlag = false;

bool scaredFlag = false;
bool timidFlag = false;

//scared variables
unsigned long timeSinceScare;
unsigned long scaredInt = 3000;


//timid variables
unsigned long timidTime;
unsigned long timidInt = 7000;

SpeedyStepper stepper;

void setup()
{
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);

  pinMode(trigPin3, OUTPUT);
  pinMode(echoPin3, INPUT);

  pinMode(trigPin4, OUTPUT);
  pinMode(echoPin4, INPUT);

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
  //**************** MEASUREMENTS ********************
  //Collect distance sensor data
  if (millis() - lastVT >= dT) {
    digitalWrite(trigPin, LOW);
    digitalWrite(trigPin2, LOW);
    digitalWrite(trigPin3, LOW);
    digitalWrite(trigPin4, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin2, HIGH);
    digitalWrite(trigPin3, HIGH);
    digitalWrite(trigPin4, HIGH);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    digitalWrite(trigPin2, LOW);
    digitalWrite(trigPin3, LOW);
    digitalWrite(trigPin4, LOW);

    duration = pulseIn(echoPin, HIGH);
    duration2 = pulseIn(echoPin2, HIGH);
    duration3 = pulseIn(echoPin3, HIGH);
    duration4 = pulseIn(echoPin4, HIGH);

    distance = (duration / 2) / 29.1;
    distance2 = (duration2 / 2) / 29.1;
    distance3 = (duration3 / 2) / 29.1;
    distance4 = (duration4 / 2) / 29.1;
    float dists[] = {distance, distance2, distance3, distance4};

    float diff = distance - lastDist;
    float diff2 = distance2 - lastDist2;
    float diff3 = distance3 - lastDist3;
    float diff4 = distance4 - lastDist4;

    vel = abs(diff);  //calculate the "velocity" for every dT
    vel2 = abs(diff2);
    vel3 = abs(diff3);
    vel4 = abs(diff4);
    float vels[] = {vel, vel2, vel3, vel4};
    Serial.println(vel);

    for ( int i = 0; i < 4; i++ ) {
      if ( vels[i] > max_v ) {
        max_v = vels[i];
      }
      if (dists[i] < min_d) {
        min_d = dists[i];
      }
    }
    if (abs(distance - lastDist) >= velThresh) {
      Serial.println("SO FAST.");
    }
    lastDist = distance; //save current distance for next iteration
    lastDist2 = distance;
    lastDist3 = distance;
    lastDist4 = distance;
    lastVT = millis();
  }
  //************************* TEST MOTOR POSITION ******************************
  if (currentSteps == maxSteps) {
    openFlag = true;
  }
  if (currentSteps == 0) {
    closedFlag = true;
  }
  if (currentSteps > 0 && currentSteps < maxSteps) {
    openFlag = false;
    closedFlag = false;
  }
  
  //****************************** BEHAVIORS ***********************************
  //STOIC:
  if (min_d > distThresh && max_v < velThresh) {
    Serial.println("STOIC");
    digitalWrite(ultraPin, LOW);
    digitalWrite(micPin, LOW);
    timidFlag = false;
    if (!openFlag) {
      stepper.moveRelativeInSteps(maxSteps - currentSteps); /// open from current to full
      currentSteps += (maxSteps - currentSteps);
    }
  }

  //TIMID
  if (min_d < distThresh && max_v < velThresh) {
    if (millis() - timidTime > timidInt) {
      Serial.println("TIMID");
      digitalWrite(ultraPin, HIGH);
      digitalWrite(micPin, LOW);
      //open slowly, close, peek
      timidFlag = false;
      if (!openFlag && !closedFlag) {
        stepper.setSpeedInStepsPerSecond(30 * 16);
        stepper.setAccelerationInStepsPerSecondPerSecond(30 * 16);
        int peekInt = (maxSteps - currentSteps) / 4;
        stepper.moveRelativeInSteps(peekInt); ///TODO
        currentSteps += peekInt;
      }
    }
    //TRUSTING:
    else if (millis() - timidTime < timidInt) {
      Serial.println("TRUSTING");
      digitalWrite(ultraPin, LOW);
      digitalWrite(micPin, HIGH);
      if (timidFlag == false) {
        timidTime = millis();
        timidFlag = true;
        if (!openFlag && !closedFlag) {
          stepper.setSpeedInStepsPerSecond(30 * 16);
          stepper.setAccelerationInStepsPerSecondPerSecond(30 * 16);
          int trustMotInt = (maxSteps - currentSteps) * (3/4);
          stepper.moveRelativeInSteps(trustMotInt); ///TODO
          currentSteps += trustMotInt;
        }
      }
    }
  }

  //SCARED:
  if (min_d <= 10 or max_v > velThresh) {    //add microphone input
    Serial.println("SCARED");
    digitalWrite(ultraPin, HIGH);
    digitalWrite(micPin, HIGH);
    if (!closedFlag){
      stepper.moveRelativeInSteps(-currentSteps); ///close all the way to 0
      currentSteps += -currentSteps;
    }
    Serial.println("motors done");
    scaredFlag = true;
    timidFlag = false;
  }
  //reset signal to not scared
  if (millis() - timeSinceScare > scaredInt) {
    //Serial.println("not scared anymore");
    scaredFlag = false;
  }
}




