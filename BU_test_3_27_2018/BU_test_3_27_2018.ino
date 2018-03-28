//control pins
int enable = 3;
int c_motor = 4;
int d_motor = 5;

//interrupt pin
int interrupt_pin = 2;

//debouncing variables
int debounce_delay = 50;

//cycle counter
int count = 0;

//state variables
bool goingHome = false;
bool goingToWork = true;
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
     if (goingToWork) {
        Serial.println("goingHome");
        goingToWork = false;
        goingHome = true;
      }
      else if (goingHome) {
        Serial.println("goingToWork");
        goingHome = false;
        goingToWork = true;
      }
      count++;
      if (count > 1) {
        Serial.println("disablingMotor");
        disableMotor();
      }
  }
  last_interrupt_time = interrupt_time;
}

//main functions
void setup() {
  Serial.begin(4800);
  // put your setup code here, to run once:
  pinMode(enable, OUTPUT);
  pinMode(c_motor, OUTPUT);
  pinMode(d_motor, OUTPUT);
  pinMode(interrupt_pin, INPUT);

  digitalWrite(enable, LOW);
  digitalWrite(c_motor, LOW);
  digitalWrite(d_motor, LOW);
 
  //attachInterrupt(digitalPinToInterrupt(2), disableMotorAfterOneCycle, FALLING);
  goHome();
}

void loop() {
  // put your main code here, to run repeatedly:
  return;
  if (goingToWork) {
    goToWork();
  }
  else if (goingHome) {
    goHome();
  }
  
  
  
  Serial.print("Count is: ");
  Serial.println(count);
  Serial.print("goingHome is: ");
  Serial.println(goingHome);
  Serial.print("goingToWork is: ");
  Serial.println(goingToWork);
  
}
