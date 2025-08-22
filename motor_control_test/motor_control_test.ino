#include <Servo.h>
#include <PID_v1.h>

#define INPUT_MIN -1000
#define INPUT_MAX 1000
#define OUTPUT_MIN 1000
#define OUTPUT_MAX 2000

long drive = 0;
long drive1 = 0;

double Setpoint, Input, Output;
double kp = 2;
double ki = 5;
double kd = 1;
PID steer1pid(&Input, &Output, &Setpoint, kp, ki, kd, DIRECT);
Servo steer1;

void setup() {
  // put your setup code here, to run once:
  steer1.attach(3);
  Serial.begin(115200);
  delay(2000);

}

void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);
  drive1 = -450;
  drive = map(drive1, INPUT_MIN, INPUT_MAX, OUTPUT_MIN, OUTPUT_MAX);
  Serial.print("drive: ");
  Serial.println(drive);
  steer1.write(drive);
  
}
