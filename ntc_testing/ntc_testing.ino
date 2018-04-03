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

bool preheating = false;
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
  //delay(1);
  digitalWrite(boilerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(boilerPin, LOW);
  Serial.println("runBoiler_preheat");
}

void runBoiler_maintain() {
  //delay(1);
  digitalWrite(boilerPin, HIGH);
  delayMicroseconds(6500);
  digitalWrite(boilerPin, LOW);
  Serial.println("runBoiler_maintain");
}

void setPreheating() {
    heating = false;
    preheating = true;
}

void disableAll() {
  preheating = false;
  heating = false;
  digitalWrite(pumpPin, LOW);
  digitalWrite(boilerPin, LOW);
  Serial.println("disableAll");
}

void disableBoiler() {
  heating = false;
  preheating = false;
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
  attachInterrupt(digitalPinToInterrupt(boilerButton), setPreheating, FALLING);
  attachInterrupt(digitalPinToInterrupt(pumpButton), runPump, FALLING);

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

  Serial.print(85);
  Serial.print(",");
  Serial.print(90);
  Serial.print(",");
  Serial.println(temp);

  if (preheating) {
    if (digitalRead(boilerRead) == LOW) {
      runBoiler_preheat();
      if (temp >= 85) {
        preheating = false;
        heating = true;
      }
    }
  }

  else if (heating) {
    if (digitalRead(boilerRead) == LOW) {
      runPump();
      runBoiler_maintain();
      if (temp >= 90) {
        disableBoiler();
      }
    }
    
  }

  if (temp >= 90) {
    disableBoiler();
  }

  delay(7);
  /*
  digitalWrite(boilerPin, HIGH);
  delay(5);
  digitalWrite(boilerPin, LOW);
  delay(1000);
  */
}
