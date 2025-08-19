/*
arduino simulator - https://wokwi.com/projects/439378312162429953

General notes:
 - we are assuming that the incoming stick values vary from 0 - 1
 - yaw from the encoder is a float in degrees from -180 to +180
 - left stick is the drive stick
 - right stick is the facing angle stick
 - all angles are in radians, all distances are in meters
 - this library does not know the robot's x/y position in real world coordinates. it DOES know the robot's real-world angle though.

*/
#include "MPU9250.h" //MPU Library: https://github.com/hideakitai/MPU9250/tree/master/MPU9250

#define DEBUG //comment this out if you want to disable debug print statements and stuff
#define SKIP_IMU //debug option, skips IMU stuff

// pi/4 (45 degrees)
#define PI_4 0.785398

//deadzones for our control sticks.
#define LEFT_STICK_DEADZONE 0.1
#define RIGHT_STICK_DEADZONE 0.1

//this is a constant that multiplies how far we want to go on each time step. bigger number makes robot go faster.
#define ZOOM_ZOOM 100

//temp delete later. pressing button 7 hard codes the joystick positions to some pre-programmed stuff
#define BUTTON_PIN 7

//width is left/right length in meters, depth is front/back length in meters
//note that this code only supports square frames, maybe one day I'll get around to adding the math to make rectangular frames work (i like to lie to myself like this sometimes)
#define FRAME_WIDTH 0.446
#define FRAME_DEPTH 0.446
#define MAGNITUDE(x, y) (sqrt(x * x + y * y))


double leftStick[] = {0,0}; //raw left stick values
double rightStick[] = {0,0};//raw right stick values
bool reset = 0;             //tied to a button on the controller. set this to 1 when you wat to manually change field alignment

double leftStickAngle = 0;  //angle of left stick
double rightStickAngle = 0; //angle of right stick
double leftStickMag = 0;    //magnitude of left stick
double rightStickMag = 0;   //magnitude of right stick
double yaw = 0;             //yaw from IMU
double yawZero = 0;         //zero direction for the bot

float transGoalX = 0;    //where we want the bot to be on the x axis
float transGoalY = 0;    //where we want the bot to be on the y axis
float angleGoal = 0;     //what direction we want the bot to be facing

float frx_c = 0;           //current positions of all the swerve modules
float fry_c = 0;
float flx_c = 0;
float fly_c = 0;
float blx_c = 0;
float bly_c = 0;
float brx_c = 0;
float bry_c = 0;

float frx_g = 0;           //goal positions of all the swerve modules
float fry_g = 0;
float flx_g = 0;
float fly_g = 0;
float blx_g = 0;
float bly_g = 0;
float brx_g = 0;
float bry_g = 0;

float fr_az_g = 0;         //angle we want to set the azimuth of each module to
float fl_az_g = 0;
float br_az_g = 0;
float bl_az_g = 0;

float fr_dr_g = 0;         //how hard we want to drive each swerve module
float fl_dr_g = 0;
float br_dr_g = 0;
float bl_dr_g = 0;

MPU9250 mpu;

#ifdef DEBUG
  char debugmsg[100];       //used for sending debug messages
  int numloops = 0;
  bool printThisLoop = false;
#endif

void setup() {
    Serial.begin(115200);
    Wire.begin();
    delay(2000);
    Serial.print("Setup started...\n");

    #ifndef SKIP_IMU
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
    #endif

    pinMode(BUTTON_PIN, INPUT_PULLUP);  //temp debug button that I will prolly change later
}

void loop() {
  //start by updating the imu
  #ifndef SKIP_MPU
    mpu.update();
  #endif

  if(reset){  //if the driver pressed the reset button then we set the zero position to our current position and ignore everything else. boohoo you can't drive while holding down the reset button, cry me a river.
    #ifdef DEBUG
      Serial.print("resetting...\n");
    #endif

    #ifndef SKIP_IMU
      readYaw();  
    #endif
    yawZero = yaw;
    angleGoal = yawZero;
    transGoalX = 0;
    transGoalY = 0;
    reset = 0;  //make sure to clear the reset flag, otherwise we just brick the bot until reboot lol
  } else {
    #ifdef DEBUG
      numloops++;
      //delay(100);
      if(numloops % 100 == 0) {
        //sprintf(debugmsg, "\n\n\n===========\nMain Loop#%d:\n===========\n", numloops);
        printThisLoop = true;
        Serial.print("\n\n\n===========\nMain Loop#");
        Serial.print(numloops);
        Serial.print("\n===========\n");
      }
    #endif
    //update our control and sensor inputs
    readSticks();
    #ifndef SKIP_IMU
      readYaw();
    #endif

    //calculate magnitudes for sticks
    leftStickMag = MAGNITUDE(leftStick[0], leftStick[1]);
    rightStickMag = MAGNITUDE(rightStick[0], rightStick[1]);
    
    //if sticks are in the deadzone then we don't have to update any commands. otherwise, calculate commands
    if(leftStickMag > LEFT_STICK_DEADZONE){
      //leftStickAngle = atan(leftStick[0]/leftStick[1]); - //not actually necessary since left stick is drive and we break it up into x and y anyway
      transGoalX = leftStick[0] * ZOOM_ZOOM;
      transGoalY = leftStick[1] * ZOOM_ZOOM;
      #ifdef DEBUG
        if(printThisLoop){
          Serial.print("transGoalX: ");
          Serial.println(transGoalX);
        }
      #endif
    } else {
      leftStick[0] = 0;
      leftStick[1] = 0;
    }
    if(rightStickMag > RIGHT_STICK_DEADZONE){
      rightStickAngle = atan(rightStick[0]/rightStick[1]);
      angleGoal = rightStickAngle + yawZero;
    } else {
      rightStick[0] = 0;
      rightStick[1] = 0;
    }
    //calculate the positions of each swerve module rn
    calculateWheelPositions(0, 0, &flx_c, &fly_c, &frx_c, &fry_c, &blx_c, &bly_c, &brx_c, &bly_c, yaw + yawZero);
    #ifdef DEBUG
      if(printThisLoop){
        Serial.println("========after first calc============");
        Serial.print("blx_c:");
        Serial.println(blx_c);

        Serial.print("bly_c:");
        Serial.println(bly_c);

        Serial.print("brx_c:");
        Serial.println(brx_c);

        Serial.print("bry_c:");
        Serial.println(bry_c);

        //====================

        Serial.print("blx_g:");
        Serial.println(blx_g);

        Serial.print("bly_g:");
        Serial.println(bly_g);

        Serial.print("brx_g:");
        Serial.println(brx_g);

        Serial.print("bry_g:");
        Serial.println(bry_g);
      }
    #endif

    //calculate where we want the swerve modules to end up
    calculateWheelPositions(transGoalX, transGoalY, &flx_g, &fly_g, &frx_g, &fry_g, &blx_g, &bly_g, &brx_g, &bly_g, angleGoal + yawZero);

    //we have our current position on this time step and also where we want to be on the next time step.
    //now we want to point our modules FROM where they are right now TO where we want them to be. We also want to calculate how hard we drive the wheels here.
    calculatePointingVector(frx_c, fry_c, frx_g, fry_g, &fr_az_g, &fr_dr_g);
    calculatePointingVector(flx_c, fly_c, flx_g, fly_g, &fl_az_g, &fl_dr_g);
    calculatePointingVector(brx_c, bry_c, brx_g, bry_g, &br_az_g, &br_dr_g);
    calculatePointingVector(blx_c, bly_c, blx_g, bly_g, &bl_az_g, &bl_dr_g);
    #ifdef DEBUG
      if(printThisLoop){
        Serial.println("========after calcing vecs============");
        Serial.print("frx_c:");
        Serial.println(frx_c);

        Serial.print("fry_c:");
        Serial.println(fry_c);

        Serial.print("flx_c:");
        Serial.println(flx_c);

        Serial.print("fly_c:");
        Serial.println(fly_c);

        Serial.print("blx_c:");
        Serial.println(blx_c);

        Serial.print("bly_c:");
        Serial.println(bly_c);

        Serial.print("brx_c:");
        Serial.println(brx_c);

        Serial.print("bry_c:");
        Serial.println(bry_c);

        //====================

        Serial.print("frx_g:");
        Serial.println(frx_g);

        Serial.print("fry_g:");
        Serial.println(fry_g);

        Serial.print("flx_g:");
        Serial.println(flx_g);

        Serial.print("fly_g:");
        Serial.println(fly_g);

        Serial.print("blx_g:");
        Serial.println(blx_g);

        Serial.print("bly_g:");
        Serial.println(bly_g);

        Serial.print("brx_g:");
        Serial.println(brx_g);

        Serial.print("bry_g:");
        Serial.println(bry_g);
      
        Serial.print("Extra debug=========\n");
        brx_c = 0;
        bry_c = MAGNITUDE(FRAME_WIDTH, FRAME_DEPTH);
        Serial.print("Mag: ");
        Serial.println(bry_c);
        rot(&brx_c, &bry_c, (-3 * PI_4));
        Serial.print("brx_c:");
        Serial.println(brx_c);
        Serial.print("bry_c:");
        Serial.println(bry_c);

        blx_c = 0;
        bly_c = MAGNITUDE(FRAME_WIDTH, FRAME_DEPTH);
        rot(&blx_c, &bly_c, (3 * PI_4));
        Serial.print("blx_c:");
        Serial.println(blx_c);
        Serial.print("bly_c:");
        Serial.println(bly_c);

      }
    #endif
  }
  printThisLoop = false;
}

int readSticks() {
  //TODO - replace this code with something that catually reads the sticks
  if(!digitalRead(BUTTON_PIN)) {
    #ifdef DEBUG
      if(printThisLoop){
        Serial.print("btn pressed\n");
      }
    #endif
    leftStick[0] = 0.2;
    leftStick[1] = 0.2;
    rightStick[0] = 0.2;
    rightStick[1] = 0.2;
    reset = 0;
  } else {
    leftStick[0] = 0;
    leftStick[1] = 0;
    rightStick[0] = 0;
    rightStick[1] = 0;
    reset = 0;
  }
  return 1;
}

void rot(float* x, float* y, float theta) {
  float tempx = *x * cos(theta) - *y * sin(theta);
  float tempy = *x * sin(theta) + *y * cos(theta);
  *x = tempx;
  *y = tempy;
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

void calculateWheelPositions(float x, float y, float *flx, float* fly, float* frx, float* fry, float* blx, float* bly, float* brx, float* bry, float angle) {
  //calculates the XY positions of each module given the robot's XY and angle. Results are passed by reference
  //first we initialize all vectors to a vector with radius
  float swerveRadius = MAGNITUDE(FRAME_WIDTH, FRAME_DEPTH);
  *flx = 0;
  *fly = swerveRadius;
  *frx = 0;
  *fry = swerveRadius;
  *brx = 0;
  *bry = swerveRadius;
  *blx = 0;
  *bly = swerveRadius;

  //now we rotate all the vectors into position.
  rot(flx, fly, angle - PI_4);
  rot(frx, fry, angle + PI_4);
  rot(blx, bly, angle + (3 * PI_4));
  rot(brx, bry, angle - (3 * PI_4));

  //now we translate everything by the xy position
  *flx = *flx + x;
  *fly = *fly + y;
  *frx = *frx + x;
  *fry = *fry + y;
  *brx = *brx + x;
  *bry = *bry + y;
  *blx = *blx + x;
  *bly = *bly + y;
}

void calculatePointingVector(float sx, float sy, float ex, float ey, float* angle, float* magnitude){
  //given a start point and end point, find the angle and distance between the two points
  float dx = ex - sx;
  float dy = ey - sy;
  *angle = atan(dy/dx);
  *magnitude = MAGNITUDE(dx, dy);
}
