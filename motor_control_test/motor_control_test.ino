#include <Servo.h>
#include <PID_v1.h>
#include <AS5X47.h>

// constants for motor controllers
#define INPUT_MIN -1000
#define INPUT_MAX 1000
#define OUTPUT_MIN 1000
#define OUTPUT_MAX 2000

#define STEER1_ENC_PIN 10



long drive = 0;
long drive1 = 0;

double Setpoint, Input, Output;

double kp = 0.1;
double ki = 0.01;
double kd = 0.1;

double steer1_angle = 0;

long int loopnum = 0;

bool didCompute = false;

PID steer1pid(&Input, &Output, &Setpoint, kp, ki, kd, DIRECT);
Servo steer1;

// Start connection to the sensor.
AS5X47 steer1_enc(STEER1_ENC_PIN);

void setup() {
  // put your setup code here, to run once:
  steer1.attach(3);
  Serial.begin(115200);
  delay(2000);

  steer1pid.SetOutputLimits(1250, 1750);
  Input = (double) steer1_enc.readAngle();
  Setpoint = 180;
  steer1pid.SetMode(AUTOMATIC);
}

void loop() {
  // put your main code here, to run repeatedly:
  //drive = map(drive1, INPUT_MIN, INPUT_MAX, OUTPUT_MIN, OUTPUT_MAX);
  loopnum++;

  Input = (double) steer1_enc.readAngle();
  steer1pid.Compute();
  steer1.write(Output);
  if(loopnum % 1000 == 0){
    Serial.print("\n=================\nInput: ");
    Serial.println(Input);

    Serial.print("\nOutput: ");
    Serial.println(Output);
    
    Serial.print("\Computed?: ");
    Serial.println(didCompute);
  }
}

// int remapPIDOutput(double num){
//   return map((int) num, INPUT_MIN, INPUT_MAX, OUTPUT_MIN, OUTPUT_MAX);
// }