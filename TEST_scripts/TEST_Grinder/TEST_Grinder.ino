#include <SoftwareSerial.h>
/*  NOT COMPLETE!!!!!!
 *  
 *  TEST: GRINDER
 *  1: Turn on
 *  2: Turn off
 *  
 *  ENABLE PIN: PIN 9
 *  
 *  **** TO DO: ******
 *  - Add in grinder impulse sensor code.....
 *  
 *  
 *  Below 15 Hz = coffee inside
 *  Above 15 Hz = no coffee left
 */

// Serial variables
SoftwareSerial mySerial(6, 7); // RX, TX
int lastByte = 0;

int COMMAND_ON = 1;
int COMMAND_OFF = 2;

// Sensor Variables
int grinderImp = A0; // analog, grinder sensor


// Grinder Variables
int enable_pin = 9;

// State variables
int WAIT_FOR_READ = 0;
int TURN_ON = 1;
int TURN_OFF = 2;

int state = WAIT_FOR_READ;

int current_f = 0;
int max_frequency = .066; // 15Hz

// Control functions
void turnOn() {
  Serial.println("Turning on.....");
  digitalWrite(enable_pin, HIGH);
}

void turnOff() {
  Serial.println("
  digitalWrite(enable_pin, LOW);
}

void interpretByte(int lastByte) {
  if (lastByte == COMMAND_ON) {
    state = TURN_ON;
    
    turnOn();
  } else if (lastByte == COMMAND_OFF) {
    state = TURN_OFF;
  
    turnOff();
  }
}

void setup() {
  pinMode(enable_pin, OUTPUT);

  pinMode(grinderImp, INPUT);


  turnOff();
  
  Serial.begin(4800);
//  while (!Serial) {
//    ; 
//  }

  mySerial.begin(9600);

  Serial.println("Setup Complete!");

  state = WAIT_FOR_READ;
}

void loop() { // run over and over
  if (state == WAIT_FOR_READ) {
    if(mySerial.available()) {
     // Serial.println("reading");
      int incomingByte = mySerial.read();
      if (incomingByte == '\n') {
        interpretByte(lastByte);
      // Serial.println("Interpretting...");
      } else {
        lastByte = incomingByte;
      }
     // Serial.print("Read byte is: ");
      Serial.println(incomingByte);
    }
  }
}
