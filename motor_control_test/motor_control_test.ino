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

double kp = 5;
double ki = 8;
double kd = 0.1;

double steer1_angle = 0;

long int loopnum = 0;

PID steer1pid(&Input, &Output, &Setpoint, kp, ki, kd, DIRECT);
Servo steer1;

// Start connection to the sensor.
AS5X47 steer1_enc(STEER1_ENC_PIN);

void setup() {
  // put your setup code here, to run once:
  steer1.attach(3);
  Serial.begin(115200);
  delay(2000);

  steer1pid.SetOutputLimits(INPUT_MIN * (2.0/4.0), INPUT_MAX * (2.0/4.0));
  Input = (double) steer1_enc.readAngle();
  Setpoint = 180;
  steer1pid.SetSampleTime(5);
  steer1pid.SetMode(AUTOMATIC);
}

void loop() {
  // put your main code here, to run repeatedly:
  //drive = map(drive1, INPUT_MIN, INPUT_MAX, OUTPUT_MIN, OUTPUT_MAX);
  loopnum++;

  Input = (double) steer1_enc.readAngle();
  steer1pid.Compute();
  drive = map(Output, INPUT_MIN, INPUT_MAX, OUTPUT_MIN, OUTPUT_MAX);
  steer1.write(drive);
  if(loopnum % 300 == 0){
    Serial.print("\n=================\nInput: ");
    Serial.println(Input);

    Serial.print("\nOutput: ");
    Serial.println(Output);
    
    Serial.print("\Drive: ");
    Serial.println(drive);
  }
}

// int remapPIDOutput(double num){
//   return map((int) num, INPUT_MIN, INPUT_MAX, OUTPUT_MIN, OUTPUT_MAX);
// }