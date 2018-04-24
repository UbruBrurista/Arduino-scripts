#include <math.h>

int sensorPin = A0;
float sensorVal;
float R1 = 32680.0;
float R2;
float Vref = 5.0;
float analogVolts;
float e = 2.7182;
float temp;

float get_temp(float R) {
  return -30.74*log(R) + 351.93;
}

void setup() {
  // put your setup code here, to run once:
  pinMode(sensorPin, INPUT);
  
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  sensorVal = analogRead(sensorPin);
  
  analogVolts = (float)sensorVal*Vref/1023.0;

  //R1 = (R2*Vref/analogVolts) - R2;

  R2 = R1/((Vref/analogVolts) - 1);

  temp = get_temp(R2);

  Serial.print("Temp is: ");
  Serial.println(temp);

}
