int enable = 3;
int c_motor = 4;
int d_motor = 5;

int interrupt_pin = 2;

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

void setup() {
  // put your setup code here, to run once:
  pinMode(enable, OUTPUT);
  pinMode(c_motor, OUTPUT);
  pinMode(d_motor, OUTPUT);
  pinMode(interrupt_pin, INPUT);
  
  digitalWrite(enable, LOW);
  digitalWrite(c_motor, LOW);
  digitalWrite(d_motor, LOW);

  attachInterrupt(digitalPinToInterrupt(interrupt_pin), disableMotor, RISING);

  goToWork();
  
}

void loop() {
  // put your main code here, to run repeatedly:

}
