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

#define SAMPLE_TIME 1 //sample time for PID loop in ms
#define ROLLING_AVERAGE_LENGTH 5 //rolling avg window for encoder readings

HCPCA9685 HCPCA9685(I2CAdd);

long drive = 0;
long drive1 = 0;

double Setpoint, Input, Output;
double mappedOutput = 200;

double kp = 5;
double ki = 0;
double kd = 0;

const long print_interval = 100;
unsigned long printMillis = 0;

const long PID_interval = SAMPLE_TIME;
unsigned long PID_Millis = 0;

const long Direction_interval = 2000;
unsigned long Direction_Millis = 0;

int loopNum = 0;
float s1EncReadings[ROLLING_AVERAGE_LENGTH];

PID steer1pid(&Input, &Output, &Setpoint, kp, ki, kd, REVERSE);
Servo steer1;
AS5X47 steer1_enc(s1EncPin);

double average(float array[ROLLING_AVERAGE_LENGTH]){
  double avg = 0;
  for(int i = 0; i < ROLLING_AVERAGE_LENGTH; i++){
    avg += array[i];
  }
  return (avg / ROLLING_AVERAGE_LENGTH);
}

void setup() {
  // put your setup code here, to run once:
  steer1.attach(3);
  Serial.begin(115200);
  delay(1000);

  steer1pid.SetOutputLimits(INPUT_MINIMUM, INPUT_MAXIMUM);
  Input = (double) steer1_enc.readAngle();
  Setpoint = 180;
  steer1pid.SetSampleTime(SAMPLE_TIME);
  steer1pid.SetMode(AUTOMATIC);

  HCPCA9685.Init(SERVO_MODE);
  HCPCA9685.Sleep(false);

  for(int i = 0; i < ROLLING_AVERAGE_LENGTH; i++){
    s1EncReadings[i] = Input;;
  }

}

void loop() {
  steer1pid.Compute();  // the pid library automatically handles interval control so it doesnt need to go in the timed loop

  unsigned long currentMillis = millis();
  if (currentMillis - PID_Millis >= PID_interval) {
    PID_Millis = currentMillis;

    loopNum++;
    float reading = steer1_enc.readAngle(); //read encoder

    //so, fucking, sometimes the encoder just lies and gives some wacky bullshit number that's way off from reality
    //so this check is basically saying "if the enc is moving way too fast, ignore it"
    //imma just say the upper limit for the wheel to be turning is 1000rpm. if we're faster than that assume its the encoder doing a meme
    //1000rpm = 6000deg/sec = 6deg/ms = 30deg/5ms bc this loop is running once every 5 ms
    if(reading == 0){
      // Serial.print("\n!!!!!!!!!!! Encoder Error Detected !!!!!!!!!!!!\n");
      // Serial.print("Bad Reading: ");
      // Serial.println(reading);
      // Serial.print("Average: ");
      // Serial.println(Input);
      s1EncReadings[loopNum % ROLLING_AVERAGE_LENGTH] = Input;
    } else if(reading > 359.8 && reading < 360) {
      // Serial.print("\n!!!!!!!!!!! Encoder Error Detected !!!!!!!!!!!!\n");
      // Serial.print("Bad Reading: ");
      // Serial.println(reading);
      // Serial.print("Average: ");
      // Serial.println(Input);
      s1EncReadings[loopNum % ROLLING_AVERAGE_LENGTH] = Input;
    } else {
      s1EncReadings[loopNum % ROLLING_AVERAGE_LENGTH] = reading;
    }
    Input = average(s1EncReadings);

    mappedOutput = map(Output, INPUT_MINIMUM, INPUT_MAXIMUM, OUTPUT_MINIMUM, OUTPUT_MAXIMUM);
    HCPCA9685.Servo(s1EncPin, (int) mappedOutput);
  }

  if (currentMillis - printMillis >= print_interval) {
    printMillis = currentMillis;

    Serial.print("\n\nReal s1Enc: ");
    Serial.println(s1EncReadings[0]);
    Serial.print("s1Enc Average: ");
    Serial.println(Input);
    Serial.print("Output: ");
    Serial.println(Output);
    Serial.print("Mapped Output: ");
    Serial.println(mappedOutput);
  }

  // if (currentMillis - Direction_Millis >= Direction_interval) {
  //   Direction_Millis = currentMillis;

  //   if(Setpoint == 150){
  //     Setpoint = 200;
  //   } else {
  //     Setpoint = 150;
  //   }
  // }
}