
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
String commandString = "";



boolean isConnected = false;


void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN,OUTPUT);
}

void loop() {

if(stringComplete)
{
  stringComplete = false;
  getCommand();
  
if(commandString.equals("$wag Money"))
  {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.write("yoloswag\n");
  }
  inputString = "";
}

}

void getCommand()
{
  if(inputString.length()>0)
  {
     commandString = inputString.substring(1,11);
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

