int pin = 2;

void printfn() {
  Serial.println("debounce");
}

void setup() {
  // put your setup code here, to run once:
  pinMode(pin, INPUT);
  attachInterrupt(digitalPinToInterrupt(pin), printfn, RISING);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  
}
