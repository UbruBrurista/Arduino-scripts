#include <math.h>

int disableButton = 3;
int pumpButton = 0;
int boilerButton = 7;
int boilerRead = 13;

int pumpPin = 5;
int boilerPin = 4;

int sensorPin = A0;
float sensorVal;
float R1 = 32400.0; //potentiometer value
float R2; // actually 32.4k in our ckt
float Vref = 5.0;
float analogVolts;
float e = 2.7182;
float temp;

bool heating = false;

//equations
//float R = 91864.000*pow(e, 0.032*temp);

float get_temp(float R) {
  return -30.74*log(R) + 351.93;
}

void runPump() {
  digitalWrite(pumpPin, HIGH);
  Serial.println("runPump");
}

void runBoiler_preheat() {
  delay(2);
  digitalWrite(boilerPin, HIGH);
  delay(1);
  digitalWrite(boilerPin, LOW);
 
  Serial.println("runBoiler_preheat");
}

void runBoiler_maintain() {
  //delay(3);
  delay(3);
  digitalWrite(boilerPin, HIGH);
  //delay(0.1);
  delay(2);
  digitalWrite(boilerPin, LOW);
 
  Serial.println("runBoiler_maintain");
}

void setHeatingTrue() {
    heating = true;
}

void disableAll() {
  heating = false;
  digitalWrite(pumpPin, LOW);
  digitalWrite(boilerPin, LOW);
  Serial.println("disableAll");
}

void disableBoiler() {
  digitalWrite(boilerPin, LOW);
  Serial.println("disable boiler");
}

void setup() {
// put your setup code here, to run once:
  pinMode(sensorPin, INPUT);
  pinMode(disableButton, INPUT);
  pinMode(pumpButton, INPUT);
  pinMode(boilerButton, INPUT);
  pinMode(boilerRead, INPUT);
  pinMode(boilerPin, OUTPUT);
  pinMode(pumpPin, OUTPUT);
  digitalWrite(boilerPin, LOW);
  digitalWrite(pumpPin, LOW);

  attachInterrupt(digitalPinToInterrupt(disableButton), disableAll, FALLING);
  attachInterrupt(digitalPinToInterrupt(boilerButton), setHeatingTrue, FALLING);
  attachInterrupt(digitalPinToInterrupt(pumpButton), runPump, FALLING);

  heating = true;
  disableAll();

  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  sensorVal = analogRead(sensorPin);
  
  analogVolts = (float)sensorVal*Vref/1023.0;

  //R1 = (R2*Vref/analogVolts) - R2;

  R2 = R1/((Vref/analogVolts) - 1);

  temp = get_temp(R2);

  Serial.print(80);
  Serial.print(",");
  Serial.print(85);
  Serial.print(",");
  Serial.println(temp);

  if (heating) {
    
    if (digitalRead(boilerRead) == LOW) {
      if (temp < 80) {
        runBoiler_preheat();
      }
      else if (temp > 80 && temp < 85) {
        runPump();
        runBoiler_maintain();
        //runBoiler_preheat();
      }
      else if (temp >= 85) {
        disableBoiler();
      }
    }
    
  }

  if (temp >= 85) {
    disableBoiler();
  }

  delay(16.6);
  /*
  digitalWrite(boilerPin, HIGH);
  delay(5);
  digitalWrite(boilerPin, LOW);
  delay(1000);
  */
}
