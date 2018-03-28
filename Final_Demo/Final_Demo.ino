#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>

#define RST_PIN         9          // Configurable, see typical pin layout above
#define SS_PIN          10         // Configurable, see typical pin layout above
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
/* Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15*/

SoftwareSerial mySerial(2, 3); // RX, TX //Init soft serial for debug purposes
int incomingByte = 0;
void setup() {
  // put your setup code here, to run once:
    Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  InitRFID();
  Serial.println("Debug Init");

  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600); //Start serial on pins 2 and 3
    pinMode(4, OUTPUT);

}

void loop() {
    if (mySerial.available()) {
    incomingByte = mySerial.read();
    Serial.print("result is: ");
    Serial.println(incomingByte, DEC);
  } //If data is available, read it as a byte and convert to a dec
  if( incomingByte != 2)
  {
      digitalWrite(4, HIGH);
      delay(1000);                       // wait for a second
      digitalWrite(4, LOW);    // turn the LED off by making the voltage LOW
      delay(1000);   
  }
    if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Dump debug info about the card; PICC_HaltA() is automatically called
  mfrc522.PICC_DumpToSerial(&(mfrc522.uid));  

}

void InitRFID()
{
    SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  mfrc522.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
}

