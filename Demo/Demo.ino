#include <SoftwareSerial.h>

int wait_delay = 3000;
int last_interrupt_time = 0;

// Serial variables
SoftwareSerial mySerial(8, 9); // RX, TX
int lastByte = 0;
// Serial state variables
int COMMAND_FULL_CYCLE = 1;
int COMMAND_GO_HOME = 2;
int COMMAND_GO_WORK = 3;
int COMMAND_GRIND = 4;
int COMMAND_PUMP = 5;

// Brew Unit Variables
int enable_pin = 10;
int c_motor_pin = 11;
int d_motor_pin = 12;

// Interrupt Variables
int interrupt_pin = 2;

// Other pins
int grinderEnable = 6;
int pumpEnable = 5;
int flowPin = 0;
int hallPin = 1;
int disablePin = 3;
int waterLevelPin = 13;

//count variables
int flowCount = 0;
int grinderCount = 0;

//state variables
int WAIT_FOR_READ = 0;
int GRIND = 1;
int GO_WORK = 2;
int PUMP = 3;
int GO_HOME = 4;
int BU_WORK = 5;
int BU_HOME = 6;
int PREHEATING = 7;

int grinderLimit = 150;
int flowLimit = 200;

int state = WAIT_FOR_READ;
int next_state = -1;
int bu_state = BU_HOME;
unsigned long noInterruptUntil = 0;

// Control functions
void runGrinder() {
  Serial.print("State is: ");
  Serial.println(state);
  Serial.println("grinding");
  grinderCount = 0;
  state = GRIND;
  next_state = GO_WORK;
  digitalWrite(grinderEnable, HIGH);
}

void grinderChange() {
  Serial.print("State is: ");
  Serial.println(state);
  grinderCount++;
  Serial.print("Grinder count: ");
  Serial.println(grinderCount);

  if (grinderCount >= grinderLimit) {
    digitalWrite(grinderEnable, LOW);
    for (int i =0; i<7000; i++) {
      Serial.println(i);
    }
    state = GO_WORK;
    next_state = PUMP;
    goToWork();

  }
}

void goToWork() {
  state = GO_WORK;
  next_state = PUMP;
  delay(2000);
  Serial.print("goToWork: state is: ");
  Serial.println(state);
  Serial.println("going to work");
  digitalWrite(c_motor_pin, HIGH);
  digitalWrite(d_motor_pin, LOW);
  digitalWrite(enable_pin, HIGH);
}

void goToWork_Manual() {
  digitalWrite(c_motor_pin, HIGH);
  digitalWrite(d_motor_pin, LOW);
  digitalWrite(enable_pin, HIGH);
}

void disableMotorAfterOneCycle() { // added debouncing code since we're bypassing the Schmidt Trigger
  disableMotor();
  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time > wait_delay) {
    last_interrupt_time = interrupt_time;
    if (state == GO_WORK) {
      if (next_state == PUMP) {
        bu_state = BU_WORK;
        for (int j =0; j<7000; j++) {
          Serial.println(j);
        }
        state = PUMP;
        next_state = GO_HOME;
        runPump();
      }
      
    } else if (state == GO_HOME) {
      if (next_state == WAIT_FOR_READ) {
        bu_state = BU_HOME;
        state = WAIT_FOR_READ;
        next_state = -1;
      }
    }
  }
}

void runPump() {
  state = PUMP;
  next_state = GO_HOME;
  delay(3000);
  Serial.print("runPump: state is: ");
  Serial.println(state);
  Serial.println("pumping");
  flowCount = 0;
  digitalWrite(pumpEnable, HIGH);
}

void flowChange() {
  Serial.print("State is: ");
  Serial.println(state);
  flowCount++;
  Serial.print("Flow count: ");
  Serial.println(flowCount);

  if (flowCount >= flowLimit) {
    digitalWrite(pumpEnable, LOW);
    for (int k =0; k<10000; k++) {
      Serial.println(k);
    }
    state = GO_HOME;
    next_state = WAIT_FOR_READ;
    goToHome();
  }
}

void goToHome() {
  state = GO_HOME;
  next_state = WAIT_FOR_READ;
  delay(2000);
  Serial.print("goToHome: state is: ");
  Serial.println(state);
  Serial.println("going home");
  //noInterruptUntil = millis() + 1000;
  digitalWrite(c_motor_pin, LOW);
  digitalWrite(d_motor_pin, HIGH);
  digitalWrite(enable_pin, HIGH);
}

void goToHome_Manual() {
  digitalWrite(c_motor_pin, LOW);
  digitalWrite(d_motor_pin, HIGH);
  digitalWrite(enable_pin, HIGH);
}

void disableMotor() {
  Serial.println("disabling motor");
  digitalWrite(enable_pin, LOW);
  digitalWrite(c_motor_pin, LOW);
  digitalWrite(d_motor_pin, LOW);
}

void disableAll() {
  Serial.println("disabling all");
  digitalWrite(enable_pin, LOW);
  digitalWrite(c_motor_pin, LOW);
  digitalWrite(d_motor_pin, LOW);
  digitalWrite(grinderEnable, LOW);
  digitalWrite(pumpEnable, LOW);
}

void interpretByte(int lastByte) {
  Serial.print("Interpreting byte: ");
  Serial.println(lastByte);
  if (lastByte == COMMAND_FULL_CYCLE) {
    state = GRIND;
    next_state = GO_WORK;
    runGrinder();

  } else if (lastByte == COMMAND_GO_WORK) {
    //state = GO_WORK;
    //next_state = WAIT_FOR_READ;
    goToWork_Manual();

  } else if (lastByte == COMMAND_GO_HOME) {
    //state = GO_HOME;
    //next_state = WAIT_FOR_READ;
    goToHome_Manual();
    
  } else if (lastByte == COMMAND_GRIND) {
    //state = GRIND;
    //next_state = WAIT_FOR_READ;
    runGrinder();
    
  } else if (lastByte == COMMAND_PUMP) {
    //state = PUMP;
    //next_state = WAIT_FOR_READ;
    runPump();
  }
}

void setup() {
  pinMode(enable_pin, OUTPUT);
  pinMode(c_motor_pin, OUTPUT);
  pinMode(d_motor_pin, OUTPUT);
  pinMode(grinderEnable, OUTPUT);
  pinMode(pumpEnable, OUTPUT);
  pinMode(interrupt_pin, INPUT);
  pinMode(waterLevelPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(interrupt_pin), disableMotorAfterOneCycle, FALLING);
  attachInterrupt(digitalPinToInterrupt(hallPin), grinderChange, FALLING);
  attachInterrupt(digitalPinToInterrupt(flowPin), flowChange, FALLING);
  attachInterrupt(digitalPinToInterrupt(disablePin), disableAll, FALLING);

  disableAll();
  
  Serial.begin(4800);
  while (!Serial) {
    ; 
  }

  mySerial.begin(9600);
  Serial.println(mySerial.available());

  Serial.println("Setup Complete!");
}

void loop() { // run over and over
  
  if(digitalRead(waterLevelPin) == LOW) {
    //Serial.println("You have enough water");
  }
  
  if (digitalRead(waterLevelPin) == HIGH) {
    disableAll();
    state = WAIT_FOR_READ;
    next_state = WAIT_FOR_READ;
  }
  
  if (state == WAIT_FOR_READ && mySerial.available()) {
    int incomingByte = mySerial.read();
    Serial.print("Read byte is: ");
    Serial.println(incomingByte, DEC);
    interpretByte(incomingByte);
  } 
  /*else if (state == PUMP) {
    state = PUMP;
    delay(3000);
    runPump();
  }
  */
  /************************
  Add something like this when you get around to adding the pump or something: 
    else if (state == PUMP) {
      state = PUMP_IN_PROGRESS; // you'll have to define these variables and startPump;
      next_state = START_BOILER:
      startPump();
    }
  ************************/

  if (mySerial.available() && mySerial.read() == 100) {
    disableAll();
    state = WAIT_FOR_READ;
  }
  
  
  /*
  switch (state) {
    case 1:
      delay(2000);
      runGrinder();
      break;
    case 2:
      delay(2000);
      goToWork();
      break;
    case 3:
      delay(2000);
      goToHome();
      break;
    case 4:
      delay(2000);
      runPump();
      break;
    case 0:
      break;
    default:
      break; 
  }
  */

}
