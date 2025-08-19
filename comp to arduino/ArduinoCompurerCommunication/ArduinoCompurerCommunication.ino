#include "MPU9250.h"
#include <string.h>

const int BUTTON_PIN = 7;

MPU9250 mpu;
float accX0 = 0;
float accY0 = 0;
float accZ0 = 0;
float yaw0 = 0;
float pitch0 = 0;
float roll0 = 0;

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
String commandString = "";
boolean isConnected = false;


void setup() {
  Serial.begin(115200);
  Wire.begin();
  delay(2000);
  //pinMode(LED_BUILTIN,OUTPUT);

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
}

void loop() {
  if(stringComplete){
    stringComplete = false;
    commandHandler();
  }
  if (mpu.update()) {
      static uint32_t prev_ms = millis();
      if (millis() > prev_ms + 50) {
          //Serial.println("hi");
          Serial.println(mpu.getYaw() - yaw0, 2);
          prev_ms = millis();
      }
  }
  inputString = "";
}

void commandHandler() {
    if(inputString.length()>0) {
      commandString = inputString.substring(1,8);
    }
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}

void GetID(char* str, char* returnMsg) {
    /* Serial Comms Protocol:
     * # XXX YYY...Y \n
     * All messages start with #
     * XXX is the identifier. Always 3 chars
     * YYYYY is the payload. total message must be under 256 bytes but otherwise may be any length
     * All messages end with \n
     * All messages are sent in string format, so things that say they are a float are acrually just a string that you have to
     *  convert to a float
     * Example message: "#YAW21.28\n"
     *      this is a message from the arduino to the computer saying the yaw value is 21.28 degrees.
     * 
     * Valid IDs:
     * J1X - joystick 1 x value (float)
     * J1Y - joystick 1 y value (float)
     * J2X - joystick 2 x value (float)
     * J2Y - joystick 2 y value (float)
     * RST - reset button value (bool)
     * ACK - acknowledgement (int)
     * YAW - yaw value from the encoder in degrees (float)
     * DBG - debug message (string)
     * 
     */
    if (str[0] != '#' || str[strlen(str) - 1] != '\n' || strlen(str) > 256){
        //if the message does not start and end correctly then the message is invalid and we toss it.
        strcpy(returnMsg, "INV");
    } else
    {
      
        //return str.Substring(1, 3);
    }
}

