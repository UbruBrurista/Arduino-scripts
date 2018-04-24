int disable_pin = 19;

void printfn() {
  Serial.println("press");
}

void setup() {
  // put your setup code here, to run once:
  pinMode(disable_pin, INPUT);
  attachInterrupt(digitalPinToInterrupt(disable_pin), printfn, FALLING);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:

}
