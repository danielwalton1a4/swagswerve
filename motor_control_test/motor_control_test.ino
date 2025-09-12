// #include <Servo.h>
// #include <PID_v1.h>
#include <AS5X47.h>
// #include "HCPCA9685.h"

// constants for motor controllers
// #define INPUT_MINIMUM -1000
// #define INPUT_MINIMUM 1000
// #define OUTPUT_MINIMUM 150  //110 and 590 come from SERVO_TRIM_MIN and max in HCPCA9685.h
// #define OUTPUT_MAXIMUM 250

#define s1EncPin 3  //pin # for STEER1 encoder and PWM channel

// #define I2CAdd 0x40       //I2C address of servo shield

// HCPCA9685 HCPCA9685(I2CAdd);

// long drive = 0;
// long drive1 = 0;

// // double Setpoint, Input, Output;

// double kp = 2;
// double ki = 0;
// double kd = 0.1;

// double steer1_angle = 0;

// long int loopnum = OUTPUT_MINIMUM;
// long int realloopnum = 0;
// long int prevInput = 0;

// const long interval = 100;
// unsigned long previousMillis = 0;

// PID steer1pid(&Input, &Output, &Setpoint, kp, ki, kd, REVERSE);
// Servo steer1;
AS5X47 steer1_enc(s1EncPin);

void setup() {
  // put your setup code here, to run once:
  // steer1.attach(3);
  Serial.begin(9600);
  // delay(2000);

  // steer1pid.SetOutputLimits(INPUT_MINIMUM, INPUT_MINIMUM);
  // Input = (double) steer1_enc.readAngle();
  // Setpoint = 180;
  // steer1pid.SetSampleTime(5);
  // steer1pid.SetMode(AUTOMATIC);

  // HCPCA9685.Init(SERVO_MODE);
  // HCPCA9685.Sleep(false);

}

void loop() {
  // HCPCA9685.Servo(STEER1_PIN, (int) drive);
  
  float angle = steer1_enc.readAngle();
  Serial.print("s1Enc: ");
  Serial.println(angle);
  delay(50);

  // unsigned long currentMillis = millis();

  // if (currentMillis - previousMillis >= interval) {
  //   // save the last time you blinked the LED
  //   previousMillis = currentMillis;
    

  // }


}