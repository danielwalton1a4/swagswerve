//MPU Library: https://github.com/hideakitai/MPU9250/tree/master/MPU9250

#include "MPU9250.h"
const int BUTTON_PIN = 7;

MPU9250 mpu;
float accX0 = 0;
float accY0 = 0;
float accZ0 = 0;
float yaw0 = 0;
float pitch0 = 0;
float roll0 = 0;

void setup() {
    Serial.begin(115200);
    Wire.begin();
    delay(2000);

    if (!mpu.setup(0x68)) {  // change to your own address
        while (1) {
            Serial.println("MPU connection failed. Please check your connection with `connection_check` example.");
            delay(5000);
        }
    }
    mpu.setAccBias(-88.87, 8.09, -33.18);
    mpu.setGyroBias(3.35, -9.06, 0.45);
    mpu.setMagBias(121.26, -7.29, -380.02);
    mpu.setMagScale(0.93, 0.99, 1.09);

    pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
    if (mpu.update()) {
        if(isPressed()){
            set0();
        }
        static uint32_t prev_ms = millis();
        if (millis() > prev_ms + 25) {
            print_roll_pitch_yaw();
            //print_accs();
            prev_ms = millis();
        }
    }
}

void print_roll_pitch_yaw() {
    Serial.print("Yaw, Pitch, Roll: ");
    Serial.print(mpu.getYaw() - yaw0, 2);
    Serial.print(", ");
    Serial.print(mpu.getPitch() - pitch0, 2);
    Serial.print(", ");
    Serial.println(mpu.getRoll() - roll0, 2);
}

void print_accs() {
    Serial.print("Acc X, Y, Z: ");
    Serial.print(mpu.getAccX() - accX0, 2);
    Serial.print(", ");
    Serial.print(mpu.getAccY() - accY0, 2);
    Serial.print(", ");
    Serial.println(mpu.getAccZ() - accZ0, 2);
}

bool isPressed(){
    return !digitalRead(BUTTON_PIN);
}

void set0(){
    accX0 = mpu.getAccX();
    accY0 = mpu.getAccX();
    accZ0 = mpu.getAccX();
    yaw0 = mpu.getYaw();
    pitch0 = mpu.getPitch();
    roll0 = mpu.getRoll();
}
