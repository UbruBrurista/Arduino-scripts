//initialization state variables
bool stCycleBrew = false;
bool stGrind = false;
bool stPrepareToBrew = false;
bool stBrew = false;
bool stDispose = false; 

//action state variables
bool brewAtHome = true;
bool brewMoving = false;
bool heaterPreheated = false;

//sensor variables
int grinderState = 6; //LOW
int heaterState = 7; //LOW
int pumpState = 8; //LOW
int valveOpen = 9; //LOW
int temp = A2; // analog, NTC
int grinderImp = A0; // analog, grinder sensor
int waterImp = A1; // analog, flowmeter?

//control variables
int tempPreheat = 85; // coming from RPi (what temp you want to brew at)
int coffeeImp; // coming from RPi? (how fine you want your coffee grounds)
int water; // coming from RPi (amount of water you want)

//control pins

//interrupt pins

//BU cycle counter

//functions
start() {
  stCycleBrew = true;
}




void setup() {
  Serial.begin(4800);
  // put your setup code here, to run once:
  pinMode(grinderState, OUTPUT);
  pinMode(heaterState, OUTPUT);
  pinMode(pumpState, OUTPUT);
  pinMode(valveOpen, OUTPUT);
  pinMode(temp, INPUT);
  pinMode(grinderImp, INPUT);
  pinMode(waterImp, INPUT);

  digitalWrite(grinderState, LOW);
  digitalWrite(heaterState, LOW);
  digitalWrite(pumpState, LOW);
  digitalWrite(valveOpen, LOW);

  start();
}

void loop() { // aka "start"
  // put your main code here, to run repeatedly:
  if (stGrind) {
    if (grinderState == LOW) {
      grinderImp = 0;
      digitalWrite(grinderState, HIGH);
    }
    else {
      if (analogRead(grinderImp) >= coffeeImp) {
        digitalWrite(grinderState, LOW);
        stGrind = false;
        stPrepareToBrew = true;
        heaterPreheated = false;
      }
    }
  }
  else if (stPrepareToBrew) {
    if (!brewMoving && brewAtHome) {
      goToWork();
      brewMoving = true;
    }
    else {
      if (heaterState == LOW && !heaterPreheated) {
        digitalWrite(heaterState, HIGH);
        // figure this part out later
      }
      else {
        if (digitalRead(temp) >= tempPreheat) {
          digitalWrite(heaterState, LOW);
          heaterPreheated = true;
          if (!brewAtHome && heaterPreheated) {
            stPrepareToBrew = false;
            stBrew = true;
          }
        }
      }
    }
  }
  else if (stBrew) {
    if (pumpState == LOW) {
      digitalWrite(pumpState, HIGH);
      digitalWrite(valveOpen, LOW);
      digitalWrite(heaterState, HIGH);
    }
    else {
      if (digitalRead(waterImp) >= water) {
        digitalWrite(pumpState, LOW);
        digitalWrite(heaterState, LOW);
        stBrew = false;
        stDispose = true;
      }
    }
  }
  else if (stDispose) {
    if (!brewMoving && !brewAtHome) {
      goHome();
      brewMoving = true;
    }
    else {
      if (brewAtHome()) {
        stDispose = false;
        stCycleBrew = false;
      }f
    }
  }
  else {
    temp = getTemp();
    pidOut = pidCalculate();
    updateTimer(pidOut);
  }
}


