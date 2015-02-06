#include <Wire.h>
#include <Adafruit_MotorShield_kill.h>
#include "utility/Adafruit_PWMServoDriver.h"

const int firstSetPoint = 6;
const int secondSetPoint = 3;
boolean weJustPoweredOn = true;
boolean firstPointSet = false;
boolean secondPointSet = false;

int motorSwitch = 7;

int turns = 0;

int distance;
int currentLocation;
int destination;

int calibrationStepSize = 1;

Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_StepperMotor *myMotor = AFMS.getStepper(200, 2);

void setup(){
 
  Serial.begin(115200);
  Serial.setTimeout(50);
  AFMS.begin();
  myMotor->setSpeed(300);
  myMotor->killPins(firstSetPoint, secondSetPoint, motorSwitch);
    
  pinMode(firstSetPoint, INPUT);
  pinMode(secondSetPoint, INPUT);
  pinMode(motorSwitch, INPUT);
  
}

void loop(){
        
	// We can disable the motors with the switch;
	if(digitalRead(motorSwitch) == 0){

		if(weJustPoweredOn){
			//We want to calibrate our motors;

			if(!firstPointSet){

				//Right, we need to work out where our first point is...
				myMotor->step(calibrationStepSize, BACKWARD, DOUBLE);
                                
				if(digitalRead(firstSetPoint) == 0){
                                        
          Serial.println("First point set...");
                                    
					currentLocation = 0;
					firstPointSet = true;
					myMotor->forceStep(5, FORWARD, DOUBLE);

					distance += 5;

				}

			} else if(!secondPointSet){

				myMotor->step(calibrationStepSize, FORWARD, DOUBLE);
				distance += calibrationStepSize;

				if(digitalRead(secondSetPoint) == 0){
    
					currentLocation = 100;
					secondPointSet = true;
          weJustPoweredOn = false;
          moveToPosition(50, false);                                    
                                        
				}

			}
                          
		} else if(Serial.available() > 0) {
      //Now we can start handling the Serial data telling us where to go.
      
      int des = Serial.parseInt();
      
      if(des < 100 && des > 0){
          moveToPosition(des, false);
      }
      
      while (Serial.read() > 0);
            
     }
           
	}

}

void moveToPosition(int des, boolean force){
	//Steps to take        
  int sPos = (distance / 100) * currentLocation;
  int desPos = (distance / 100) * des;
  
  int s2T = 0 - (sPos - desPos);
  
  int stepsLeft;
  
	if(s2T < 0){ 
		if(!force){
			stepsLeft = myMotor->step(-s2T, BACKWARD, DOUBLE);	
		} else {
			myMotor->forceStep(-s2T, BACKWARD, DOUBLE);
		}

	} else {

		if(!force){
			stepsLeft = myMotor->step(s2T, FORWARD, DOUBLE);
		} else {
			myMotor->forceStep(s2T, FORWARD, DOUBLE);
		} 

	}

        if(stepsLeft > 0){
           
           if(digitalRead(firstSetPoint) == 0){
              //We're at the left side of the track, we need to move to 1% of the tracks width
              currentLocation = 0;
              moveToPosition(1, true);
           } else if(digitalRead(secondSetPoint) == 0){
              //We're at the right side of the track, we need to move to 99% of the track width   
              currentLocation = 100;
              moveToPosition(99, true);
           }
          
        }

	currentLocation = des;

}