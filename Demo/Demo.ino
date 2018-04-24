#include <SoftwareSerial.h>

int wait_delay = 1000;
long motor_start = 0;
int last_interrupt_time = 0;

// Serial variables
//SoftwareSerial Serial3(52, 53); // RX, TX
int lastByte = 0;
// Serial state variables
int COMMAND_DISABLE_ALL = 1;
int COMMAND_FULL_CYCLE = 2;
int COMMAND_GO_HOME = 3;
int COMMAND_GO_WORK = 4;
int COMMAND_GRIND = 5;
int COMMAND_PUMP = 6;

// Communication
int pulse_pin = 20;
int start_stop_pin = 21;

int pulse_count = 0;
bool pulse_started = false;

// Brew Unit Variables
int enable_pin = 10;
int c_motor_pin = 11;
int d_motor_pin = 12;

// Interrupt Variables
int interrupt_pin = 18;

// Other pins
int grinderEnable = 6; // 48 on PCB
int pumpEnable = 5; // 51 on PCB
int boilerEnable = 4; // pin 2 on PCB
int flowPin = 2;
int hallPin = 3;
int disablePin = 19;
int boilerRead = 13; // pin 13 on PCB as well
int waterLevelPin = 14;
int NTC_sensorPin = A0;

//count variables
int flowCount = 0;
int grinderCount = 0;

//heating variables
float NTC_sensorVal;
float R1 = 32400.0; //resistor value on PCB
float R2; // NTC resistor value
float Vref = 5.0; // voltage from PowerSupply PCB
float analogVolts;
float e = 2.7182;
float temp;
bool preheating = false;
bool heating = false;
int lower_limit = 85;
int upper_limit = lower_limit + 5;
int boiler_flow_count = 0;

//state variables
int WAIT_FOR_READ = 0;
int PREHEATING = 1;
int GRIND = 2;
int GO_WORK = 3;
int PUMP = 4;
int GO_HOME = 5;
int BU_WORK = 6;
int BU_HOME = 7;
int HEATING = 8;
int DISPOSE = 9;
int WAIT_FOR_TYPE = 10;
int WAIT_FOR_SIZE = 11;
int WAIT_FOR_TEMP = 12;

// brew variables
int ESPRESSO = 1;
int AMERICANO = 2;

int desiredType;
int desiredSize;
int desiredTemp;

// Grinder and Flowmeter interrupt limits
int grinderLimit = 150;
int flowLimit = 200;

int state = WAIT_FOR_READ;
int next_state = -1;
int bu_state = BU_HOME;
unsigned long noInterruptUntil = 0;

// Control functions
float get_temp(float R) {
  return -30.74*log(R) + 351.93;
}

void read_NTC() {
    NTC_sensorVal = analogRead(NTC_sensorPin);
    analogVolts = (float)NTC_sensorVal*Vref/1023.0;
    R2 = R1/((Vref/analogVolts) - 1);
    temp = get_temp(R2);
}

void disableBoiler() {
  digitalWrite(boilerEnable, LOW);
  Serial.println("disabling boiler");
}

void disableBoiler_afterCycle() {
  heating = false;
  preheating = false;
  digitalWrite(boilerEnable, LOW);
  Serial.println("disable boiler after cycle");
}

void setPreheating() {
  heating = false;
  preheating = true;
}

void setHeating() {
  heating = true;
  preheating = false;
}

void runBoiler_preheat() {
  digitalWrite(boilerEnable, HIGH);
  delay(6);
  digitalWrite(boilerEnable, LOW);
}

//void runBoiler_maintain() {
//  if (temp < 80  && flowCount % 2 == 0) {
//    digitalWrite(boilerEnable, HIGH);
//    //delayMicroseconds(6500);
//    delayMicroseconds(16000);
//    digitalWrite(boilerEnable, LOW);
//    Serial.println("runBoiler_maintain");
//  }
//  else if (flowCount % 1 == 1) {
//    disableBoiler_afterCycle();
//  }
//}

void runGrinder() {
  Serial.print("State is: ");
  Serial.println(state);
  Serial.println("grinding");
  //preheating = false;
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
    for (int i =0; i<200; i++) {
      Serial.println(i);
    }
    //sendInterrupt();
    goToWork();
  }
}

void goToWork() {
  state = GO_WORK;
  next_state = PUMP;
  Serial.print("goToWork: state is: ");
  Serial.println(state);
  Serial.println("going to work");
  digitalWrite(c_motor_pin, HIGH);
  digitalWrite(d_motor_pin, LOW);
  digitalWrite(enable_pin, HIGH);
  motor_start = millis();
}

void goToWork_Manual() {
  digitalWrite(c_motor_pin, HIGH);
  digitalWrite(d_motor_pin, LOW);
  digitalWrite(enable_pin, HIGH);
  motor_start = millis();
}

void disableMotorAfterOneCycle() { // added debouncing code since we're bypassing the Schmidt Trigger
  Serial.println("disableMotorAfterOneCycle");
  unsigned long interrupt_time = millis();
  if (interrupt_time - motor_start > wait_delay) {
      disableMotor();
      last_interrupt_time = interrupt_time;
      if (state == GO_WORK) {
        if (next_state == PUMP) {
          bu_state = BU_WORK;
          for (int j =0; j<200; j++) {
            Serial.println(j);
          }
          state = PUMP;
          next_state = GO_HOME;
          runPump();
        }
        else if (next_state == PUMP) {
          state = PUMP;
          next_state = GO_HOME;
          runPump();
        }
        
      } else if (state == GO_HOME) {
        Serial.print("NEXT STATE: ");
        Serial.println(next_state);
        if (next_state == WAIT_FOR_READ) {
          bu_state = BU_HOME;
          state = WAIT_FOR_READ;
          next_state = -1;
          sendInterrupt();
        }
        else if (next_state == GO_WORK) {
          bu_state = BU_HOME;
          state = GO_WORK;
          next_state = PUMP;
          goToWork();
        }
      }
  }
}

void runPump() {
  state = PUMP;
  next_state = GO_HOME;
  //delay(3000);
  Serial.print("runPump: state is: ");
  Serial.println(state);
  Serial.println("pumping");
  setHeating();
  //heating = true;
  flowCount = 0;
  digitalWrite(pumpEnable, HIGH);
}

void flowChange() {
  Serial.print("State is: ");
  Serial.println(state);
  flowCount++;
  boiler_flow_count++;
  Serial.print("Flow count: ");
  Serial.println(flowCount);

  read_NTC();

  Serial.print(lower_limit);
  Serial.print(",");
  Serial.print(upper_limit);
  Serial.print(",");
  Serial.println(temp);

  Serial.println(heating);
  Serial.println(digitalRead(boilerRead));
  
  Serial.println(upper_limit);
  
//  if (temp >= upper_limit || flowCount >= (flowLimit*0.8)) {
  if (temp >= upper_limit) {
    disableBoiler();
  } else {
    if (digitalRead(boilerEnable) == HIGH) {
      digitalWrite(boilerEnable, LOW);
    }
    else {
      if (boiler_flow_count >= 1 && temp < upper_limit) {
        digitalWrite(boilerEnable, HIGH);
        boiler_flow_count = 0;
      }
    }
  }
  
  

  if (flowCount >= flowLimit) {
    digitalWrite(pumpEnable, LOW);
    disableBoiler_afterCycle();
    for (int k =0; k<100; k++) {
      Serial.println(k);
    }
    state = GO_HOME;
    Serial.print("Desired Type: ");
    Serial.println(desiredType);
    if (desiredType == ESPRESSO) {
      next_state = WAIT_FOR_READ;
    } else if (desiredType == AMERICANO) {
      Serial.println("------------------------------------------------------- else if");
      next_state = GO_WORK;
      desiredType = ESPRESSO; // Sends it to just go back
      flowLimit = 200 + ((desiredType-1) * 100);
    }
    goToHome();
    //sendInterrupt();
  }
}

void goToHome() {
  state = GO_HOME;
  //delay(2000);
  Serial.print("goToHome: state is: ");
  Serial.println(state);
  Serial.println("going home");
  digitalWrite(c_motor_pin, LOW);
  digitalWrite(d_motor_pin, HIGH);
  digitalWrite(enable_pin, HIGH);
  motor_start = millis();
}

void goToHome_Manual() {
  digitalWrite(c_motor_pin, LOW);
  digitalWrite(d_motor_pin, HIGH);
  digitalWrite(enable_pin, HIGH);
  motor_start = millis();
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
  disableBoiler_afterCycle();
  state = WAIT_FOR_READ;
}

void boilerInterrupt() {
  Serial.println("boilerInterrupt");
}

void interpretByte(int lastByte) {
  Serial.print("Interpreting byte: ");
  Serial.println(lastByte);

  if (state == WAIT_FOR_TYPE) {
    desiredType = lastByte;
    state = WAIT_FOR_SIZE;
    return;
  } else if (state == WAIT_FOR_SIZE) {
    desiredSize = lastByte;
    state = WAIT_FOR_TEMP;
    flowLimit = 100 + (desiredSize - 1)*100;
    return;
  } else if (state == WAIT_FOR_TEMP) {
    desiredTemp = 79 + lastByte;
    lower_limit = desiredTemp;
    upper_limit = lower_limit + 5;

    Serial.print("Desired Type: ");
    Serial.println(desiredType);
    Serial.print("Desired Size: ");
    Serial.println(desiredSize);
    Serial.print("Desired Temp: ");
    Serial.println(desiredTemp);
    
    state = PREHEATING;
    next_state = GRIND;
    preheating = true;
    return;
  }

  if (state != WAIT_FOR_READ) {
    return;
  }

  // Run full cycle
  if (lastByte == COMMAND_FULL_CYCLE) {
    state = WAIT_FOR_TYPE;
    return;
//    state = PREHEATING;
//    next_state = GRIND;
//    preheating = true;
  }
  // Go to work
  else if (lastByte == COMMAND_GO_WORK) {
    //state = GO_WORK;
    //next_state = WAIT_FOR_READ;
    goToWork_Manual();

  }
  //Go to home
  else if (lastByte == COMMAND_GO_HOME) {
    //state = GO_HOME;
    //next_state = WAIT_FOR_READ;
    goToHome_Manual();
    
  }
  // Run grinder
  else if (lastByte == COMMAND_GRIND) {
    //state = GRIND;
    //next_state = WAIT_FOR_READ;
    runGrinder();
    
  }
  // Run pump
  else if (lastByte == COMMAND_PUMP) {
    //state = PUMP;
    //next_state = WAIT_FOR_READ;
    flowLimit = 2000;
    runPump();
  }

}

void start_stop_pulse() {
  if (pulse_started) {
    doneReading();
  } else {
    waitForRead();
  }
}

void count_pulse() {
  pulse_count++;
}

void doneReading() {
  pulse_started = false;
  Serial.print("Byte: ");
  Serial.println(pulse_count);
  
  detachInterrupt(digitalPinToInterrupt(pulse_pin));
  if (state == WAIT_FOR_READ && pulse_count == COMMAND_DISABLE_ALL) {
    disableAll();
  } else {
    interpretByte(pulse_count);
  }
}

void sendInterrupt() {
  Serial.println("SENDING INTERRUPT");
  Serial.print("CURRENT OUTPUT: ");
  Serial.println(digitalRead(pulse_pin));
  pinMode(pulse_pin, OUTPUT);
  digitalWrite(pulse_pin, LOW);
  delay(2);
  Serial.print("CURRENT OUTPUT: ");
  Serial.println(digitalRead(pulse_pin));
  digitalWrite(pulse_pin, HIGH);
  delay(10);
  Serial.print("CURRENT OUTPUT: ");
  Serial.println(digitalRead(pulse_pin));
  digitalWrite(pulse_pin, LOW);
  Serial.print("CURRENT OUTPUT: ");
  Serial.println(digitalRead(pulse_pin));
}

void waitForRead() {
  //Serial.println("waiting for read, pulse_count = ");
  pulse_started = true;
  pulse_count = 0;
  //Serial.println(pulse_count);
  pinMode(pulse_pin, INPUT);
  attachInterrupt(digitalPinToInterrupt(pulse_pin), count_pulse, RISING);
}

void setup() {
  pinMode(enable_pin, OUTPUT);
  pinMode(c_motor_pin, OUTPUT);
  pinMode(d_motor_pin, OUTPUT);
  pinMode(grinderEnable, OUTPUT);
  pinMode(pumpEnable, OUTPUT);
  pinMode(boilerEnable, OUTPUT);
  pinMode(interrupt_pin, INPUT);
  pinMode(waterLevelPin, INPUT);
  pinMode(boilerRead, INPUT);
  pinMode(start_stop_pin, INPUT);

  attachInterrupt(digitalPinToInterrupt(interrupt_pin), disableMotorAfterOneCycle, RISING); // changed to RISING b/c removed diodes
  attachInterrupt(digitalPinToInterrupt(hallPin), grinderChange, FALLING);
  attachInterrupt(digitalPinToInterrupt(flowPin), flowChange, FALLING);
  attachInterrupt(digitalPinToInterrupt(disablePin), disableAll, FALLING);
  attachInterrupt(digitalPinToInterrupt(start_stop_pin), start_stop_pulse, RISING);

  disableAll();
}

void loop() { // run over and over

  if (preheating) {
    NTC_sensorVal = analogRead(NTC_sensorPin);
  
    analogVolts = (float)NTC_sensorVal*Vref/1023.0;

    //R1 = (R2*Vref/analogVolts) - R2;

    R2 = R1/((Vref/analogVolts) - 1);

    temp = get_temp(R2);
  
    Serial.print(lower_limit);
    Serial.print(",");
    Serial.print(upper_limit);
    Serial.print(",");
    Serial.println(temp);

    if (preheating) {
      if (digitalRead(boilerRead) == LOW) {
        runBoiler_preheat();
//        if (temp >= 0) {
        if (temp >= lower_limit) {
          preheating = false;
          disableBoiler_afterCycle();
          state = GRIND;
          next_state = GO_WORK;
          runGrinder();
          //sendInterrupt();
        }
      }
    }  
//    else if (heating) {
//      if (digitalRead(boilerRead) == LOW) {
//        runBoiler_maintain();
//        if (temp >= upper_limit) {
//          disableBoiler();
//        }
//      }
//    }
  
    if (temp >= upper_limit) {
      disableBoiler();
    }
    
    //delay(5);
  }
  
  if(digitalRead(waterLevelPin) == LOW) {
    //Serial.println("You have enough water");
  }
  
  if (digitalRead(waterLevelPin) == HIGH) {
    Serial.println("Add Water");
    disableAll();
    state = WAIT_FOR_READ;
    next_state = WAIT_FOR_READ;
  }
  // if (state == WAIT_FOR_READ && Serial3.available()) {
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
