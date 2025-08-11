/*
General notes:
 - we are assuming that the incoming stick values vary from 0 - 1
 - yaw from the encoder is a float in degrees from -180 to +180
 - left stick is the drive stick
 - right stick is the facing angle stick

*/
#include "MPU9250.h" //MPU Library: https://github.com/hideakitai/MPU9250/tree/master/MPU9250

//deadzones for our control sticks.
#define LEFT_STICK_DEADZONE 0.2
#define RIGHT_STICK_DEDAZONE 0.2
#define BUTTON_PIN 7

double leftStick[] = {0,0}; //raw left stick values
double rightStick[] = {0,0};//raw right stick values
bool reset = 0;             //tied to a button on the controller. set this to 1 when you wat to manually change field alignment

double leftStickAngle = 0;  //angle of left stick
double rightStickAngle = 0; //angle of right stick
double leftStickMag = 0;    //magnitude of left stick
double rightStickMag = 0;   //magnitude of right stick
double yaw = 0;             //yaw from IMU
double yawZero = 0;         //zero direction for the bot
double azs[] = {0,0,0,0};   // [FL, FR, BL, BR] - encoder azimuths for swerve modules

float transGoalX = 0;    //where we want the bot to be on the x axis
float transGoalY = 0;    //where we want the bot to be on the y axis
float angleGoal = 0;

MPU9250 mpu;

void setup() {
    Serial.begin(115200);
    Wire.begin();
    delay(2000);

    if (!mpu.setup(0x68)) {
        while (1) {
            Serial.println("MPU connection failed. Please check your connection with `connection_check` example.");
            delay(5000);
        }
    }
    //pre-calculated calibration results here. this will be different for each individual unit, run calibration.ino from the IMU library to calculate these.
    mpu.setAccBias(-88.87, 8.09, -33.18);
    mpu.setGyroBias(3.35, -9.06, 0.45);
    mpu.setMagBias(121.26, -7.29, -380.02);
    mpu.setMagScale(0.93, 0.99, 1.09);

    delay(5000);  //the IMU takes a couple of seconds to settle on an accurate reading on bootup, so we need to wait a bit otherwise the bot will annoyingly drift at the start.
    yawZero = mpu.getYaw(); //set the initial 0 position to our starting position

    pinMode(BUTTON_PIN, INPUT_PULLUP);  //temp debug button that I will prolly change later
}

void loop() {
  if(reset){  //if the driver pressed the reset button then we set the zero position to our current position and ignore everything else. boohoo you can't drive while holding down the reset button, cry me a river.
    readYaw();
    yawZero = yaw;
    reset = 0;  //make sure to clear the reset flag, otherwise we just brick the bot until reboot lol
  } else {
    //update our control and sensor inputs
    readSticks();
    readYaw();

    //calculate magnitudes for sticks
    leftStickMag = sqrt(pow(leftStick[0], 2) + pow(leftStick[1], 2));
    rightStickMag = sqrt(pow(rightStick[0], 2) + pow(rightStick[1], 2));
    
    //if sticks are in the deadzone then we don't have to update any commands. otherwise, calculate commands
    if(leftStickMag > LEFT_STICK_DEADZONE){
      //leftStickAngle = atan(leftStick[0]/leftStick[1]); - //not actually necessary since left stick is drive and we break it up into x and y anyway
      transGoalX = leftStick[0];
      transGoalY = leftStick[1];
    }
    if(rightStickMag > RIGHT_STICK_DEDAZONE){
      rightStickAngle = atan(rightStick[0]/rightStick[1]);
      angleGoal = rightStickAngle;
    }
  }
}

int readSticks() {
  //TODO - replace this code with something that catually reads the sticks
  leftStick[0] = 0;
  leftStick[1] = 0;
  rightStick[0] = 0;
  rightStick[1] = 0;
  reset = 0;
  return 1;
}

float readYaw() {
  //TODO - replace this code with something that catually reads the sticks
  yaw = 0;
}

int setAz(int motorID, double angle) {
  //TODO - make code that actually drives the motors lol
  return 1;
}

int setDrive(int motorID, double drive) {
  //TODO - make code that actually drives the motors lol
  return 1;
}
