#include <SoftwareSerial.h>

// Serial variables
SoftwareSerial mySerial(8, 9); // RX, TX
int lastByte = 0;

int COMMAND_FULL_CYCLE = 1;
int COMMAND_GO_HOME = 2;
int COMMAND_GO_WORK = 3;

// Brew Unit Variables
int enable_pin = 3;
int c_motor_pin = 4;
int d_motor_pin = 5;

// Interrupt Variables
int interrupt_pin = 2;
int debounce_delay = 100;
int count = 0;

//state variables
int WAIT_FOR_READ = 0;
int GO_WORK = 1;
int GO_HOME = 2;

int state = WAIT_FOR_READ;
int next_state = -1;

// Control functions
void goToHome() {
  digitalWrite(c_motor_pin, LOW);
  digitalWrite(d_motor_pin, HIGH);
  digitalWrite(enable_pin, HIGH);
}

void goToWork() {
  digitalWrite(c_motor_pin, HIGH);
  digitalWrite(d_motor_pin, LOW);
  digitalWrite(enable_pin, HIGH);
}

void disableMotor() {
  digitalWrite(enable_pin, LOW);
  digitalWrite(c_motor_pin, LOW);
  digitalWrite(d_motor_pin, LOW);
}

void disableMotorAfterOneCycle() { // added debouncing code since we're bypassing the Schmidt Trigger
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time > debounce_delay) {
    int next_next_state = -1;

    if (state == GO_WORK) {
      if (next_state == GO_HOME) {
        goToWork();
        next_next_state = WAIT_FOR_READ;
      } else if (next_state == WAIT_FOR_READ) {
        disableMotor();
      }

    } else if (state == GO_HOME) {
      if (next_state == WAIT_FOR_READ) {
        disableMotor();
      }
    }

    state = next_state;
    next_state = next_next_state;
  }

  last_interrupt_time = interrupt_time;
}

void interpretByte(int lastByte) {
  if (lastByte == COMMAND_FULL_CYCLE) {
    state = GO_WORK;
    /**********************************
    // Change the next line to something like `next_state = START)PUMP;` when we need to add that 
    // (you'll need to define START_PUMP too)
    **********************************/
    next_state = GO_HOME;

    goToWork();

  } else if (lastByte == COMMAND_GO_WORK) {
    state = GO_WORK;
    next_state = WAIT_FOR_READ;
    goToWork();

  } else if (lastByte == COMMAND_GO_HOME) {
    state = GO_HOME;
    next_state = WAIT_FOR_READ;
    goToHome();
  }
}

void setup() {
  pinMode(enable_pin, OUTPUT);
  pinMode(c_motor_pin, OUTPUT);
  pinMode(d_motor_pin, OUTPUT);
  pinMode(interrupt_pin, INPUT);
  attachInterrupt(digitalPinToInterrupt(2), disableMotorAfterOneCycle, FALLING);

//  disableMotor();
  
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
       // interpretByte(lastByte);
      // Serial.println("Interpretting...");
      } else {
        lastByte = incomingByte;
      }
      Serial.print("Read byte is: ");
      Serial.println(incomingByte, DEC);
    }
  }
  /************************
  Add something like this when you get around to adding the pump or something: 
    else if (state == START_PUMP) {
      state = PUMP_IN_PROGRESS; // you'll have to define these variables and startPump;
      next_state = START_BOILER:
      startPump();
    }
  ************************/

}
