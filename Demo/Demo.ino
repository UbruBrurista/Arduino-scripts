/*
  Software serial multple serial test

 Receives from the hardware serial, sends to software serial.
 Receives from software serial, sends to hardware serial.

 The circuit:
 * RX is digital pin 10 (connect to TX of other device)
 * TX is digital pin 11 (connect to RX of other device)

 Note:
 Not all pins on the Mega and Mega 2560 support change interrupts,
 so only the following can be used for RX:
 10, 11, 12, 13, 50, 51, 52, 53, 62, 63, 64, 65, 66, 67, 68, 69

 Not all pins on the Leonardo and Micro support change interrupts,
 so only the following can be used for RX:
 8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI).

 created back in the mists of time
 modified 25 May 2012
 by Tom Igoe
 based on Mikal Hart's example

 This example code is in the public domain.

 */
#include <SoftwareSerial.h>

SoftwareSerial mySerial(6, 7); // RX, TX
int START_BYTE = 1;
int lastByte = 0;
bool waitForRead = false;

//control pins
int enable = 3;
int c_motor = 4;
int d_motor = 5;

//interrupt pin
int interrupt_pin = 2;

//debouncing variables
int debounce_delay = 100;

//cycle counter
int count = 0;

//state variables
bool goingHome = false;
bool goingToWork = false;
bool motorDisabled = false;

//control functions
void goHome() {
  goingToWork = false;
  goingHome = true;
  motorDisabled = false;
  digitalWrite(c_motor, LOW);
  digitalWrite(d_motor, HIGH);
  digitalWrite(enable, HIGH);
  //wait for interrupt, then call disableMotor
}

void goToWork() {
  goingHome = false;
  goingToWork = true;
  motorDisabled = false;
  digitalWrite(c_motor, HIGH);
  digitalWrite(d_motor, LOW);
  digitalWrite(enable, HIGH);
  //wait for interrupt, then call disableMotor
}

void disableMotor() {
  motorDisabled = true;
  goingHome = false;
  goingToWork = false;
  digitalWrite(enable, LOW);
  digitalWrite(c_motor, LOW);
  digitalWrite(d_motor, LOW);
}

void disableMotorAfterOneCycle() { // added debouncing code since we're bypassing the Schmidt Trigger
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time > debounce_delay) 
  {
      
      Serial.println("Count is: ");
      Serial.println(count);
      Serial.println("goingHome is: ");
      Serial.println(goingHome);
      Serial.println("goingToWork is: ");
      Serial.println(goingToWork);
     if (goingToWork) {
        goingToWork = false;
        goingHome = true;
      }
      else if (goingHome) {
        goingHome = false;
        goingToWork = true;
      }
      count++;
      if (count > 1) {
        disableMotor();
        waitForRead = true;
        count = 0;
      }
  }
  last_interrupt_time = interrupt_time;
}

void start() {
  goingToWork = true;
  waitForRead = false;
}

void setup() {
  //pinMode(4, OUTPUT);
  pinMode(enable, OUTPUT);
  pinMode(c_motor, OUTPUT);
  pinMode(d_motor, OUTPUT);
  pinMode(interrupt_pin, INPUT);

  digitalWrite(enable, LOW);
  digitalWrite(c_motor, LOW);
  digitalWrite(d_motor, LOW);
 
  attachInterrupt(digitalPinToInterrupt(2), disableMotorAfterOneCycle, FALLING);
  //goHome();

  
  // Open serial communications and wait for port to open:
  Serial.begin(4800);
  waitForRead = true;
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  Serial.println("Goodnight moon!");

  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);
  mySerial.println("Hello, world?");
}

void loop() { // run over and over
  if (waitForRead && mySerial.available()) {
    Serial.println("reading");
    int incomingByte = mySerial.read();
    if (incomingByte == '\n') {
      if (lastByte == START_BYTE) {
        start();
      }
    } else {
      lastByte = incomingByte;
    }
    Serial.print("result is: ");
    Serial.println(incomingByte);
    
  } else if (goingToWork) {
    goToWork();
  }
  else if (goingHome) {
    goHome();
  }
}

