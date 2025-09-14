#include <Servo.h>
#include <PID_v1.h>
#include <AS5X47.h>
#include "HCPCA9685.h"

// constants for motor controllers
#define INPUT_MINIMUM -180     //0 to 360 is the range of the encoder
#define INPUT_MAXIMUM 180
#define OUTPUT_MINIMUM 150  //after doing some testing, it looks like the rangefor the PCA9685 is 0-400.
#define OUTPUT_MAXIMUM 250  //so 200 is dead center with 0 motor drive, below is reverse and above is forward

#define s1EncPin 3  //pin # for STEER1 encoder and PWM channel

#define I2CAdd 0x40       //I2C address of servo shield

HCPCA9685 HCPCA9685(I2CAdd);

long drive = 0;
long drive1 = 0;

double Setpoint, Input, Output;
double mappedOutput = 200;

double kp = 2;
double ki = 0;
double kd = 0.05;

const long interval = 100;
unsigned long previousMillis = 0;

PID steer1pid(&Input, &Output, &Setpoint, kp, ki, kd, REVERSE);
Servo steer1;
AS5X47 steer1_enc(s1EncPin);

void setup() {
  // put your setup code here, to run once:
  steer1.attach(3);
  Serial.begin(115200);
  delay(1000);

  steer1pid.SetOutputLimits(INPUT_MINIMUM, INPUT_MAXIMUM);
  Input = (double) steer1_enc.readAngle();
  Setpoint = 180;
  steer1pid.SetSampleTime(5);
  steer1pid.SetMode(AUTOMATIC);

  HCPCA9685.Init(SERVO_MODE);
  HCPCA9685.Sleep(false);

}

void loop() {
  // HCPCA9685.Servo(STEER1_PIN, (int) drive);

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    Input = steer1_enc.readAngle();
    Serial.print("\n\ns1Enc: ");
    Serial.println(Input);

    steer1pid.Compute();
    Serial.print("Output: ");
    Serial.println(Output);

    mappedOutput = map(Output, INPUT_MINIMUM, INPUT_MAXIMUM, OUTPUT_MINIMUM, OUTPUT_MAXIMUM);
    Serial.print("Mapped Output: ");
    Serial.println(mappedOutput);

    HCPCA9685.Servo(s1EncPin, (int) mappedOutput);

  }
    

  // }


}