/*
  ReadAngle

  Read the angle position from an AS5047 or AS5147 rotary encoder.
  Start the terminal with a speed of 9600 Bauds to see the measured angle.

  The encoder is connected as follows:
  AS5X47         Arduino Board
    5V <-------------> 5V (Red)
   GND <-------------> GND (Black)
  MOSI <-------------> MOSI (Blue, Pin 11 for Arduino Uno)
  MISO <-------------> MISO (Green, Pin 12 for Arduino Uno)
   SCK <-------------> SCK  (Yellow, Pin 13 for Arduino Uno)
   CSN <-------------> SS   Purple, Pin 2 in this example.
  See Arduino SPI Reference to see how to connect with other boards.
  https://www.arduino.cc/en/reference/SPI

  Created 18 april 2020
  By Adrien Legrand

  https://github.com/Adrien-Legrand/AS5X47

*/

#include <AS5X47.h>

// Define where the CSN Pin in connected. 
#define s1EncPin 2
#define d1EncPin 3
#define s2EncPin 4
#define d2EncPin 5
#define s3EncPin 6
#define d3EncPin 7
#define s4EncPin 8
#define d4EncPin 9

// Start connection to the sensor.
AS5X47 s1Enc(s1EncPin);
AS5X47 d1Enc(d1EncPin);
AS5X47 s2Enc(s2EncPin);
AS5X47 d2Enc(d2EncPin);
AS5X47 s3Enc(s3EncPin);
AS5X47 d3Enc(d3EncPin);
AS5X47 s4Enc(s4EncPin);
AS5X47 d4Enc(d4EncPin);

void setup() {
  // Initialize a Serial Communication in order to
  // print the measured angle.
  Serial.begin(9600);
}

void loop() {

  float angle = s1Enc.readAngle();
  Serial.print("s1Enc: ");
  Serial.println(angle);


  angle = d1Enc.readAngle();
  Serial.print("d1Enc: ");
  Serial.println(angle);


  angle = s2Enc.readAngle();
  Serial.print("s2Enc: ");
  Serial.println(angle);


  angle = d2Enc.readAngle();
  Serial.print("d2Enc: ");
  Serial.println(angle);


  angle = s3Enc.readAngle();
  Serial.print("s3Enc: ");
  Serial.println(angle);


  angle = d3Enc.readAngle();
  Serial.print("d3Enc: ");
  Serial.println(angle);


  angle = s4Enc.readAngle();
  Serial.print("s4Enc: ");
  Serial.println(angle);


  angle = d4Enc.readAngle();
  Serial.print("d4Enc: ");
  Serial.println(angle);


  Serial.print("\n\n================\n\n");
  delay(500);

}
