#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>
#include <Servo.h>
#define MQ2pin 0
#define Threshold 400
float sensorValue; 
String BT_input;

#define RST_PIN   9
#define SS_PIN    10

byte readCard[4];
char* myTags[100] = {};
int tagsCount = 0;
String tagID = "";
boolean successRead = false;
boolean correctTag = false;
int proximitySensor;
boolean doorOpened = false;
int value = 0;

// Create instances
MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo myServo; // Servo motor

void setup() {
  // Initiating
  Serial.begin(9600);
  SPI.begin();        // SPI bus
  mfrc522.PCD_Init(); //  MFRC522
  myServo.attach(8);  // Servo motor

  myServo.write(180); // Initial lock position of the servo motor
  // Prints the initial message
  Serial.println("MQ2 warming up!");
  Serial.println("-No Master Tag!-");
  Serial.println("    SCAN NOW");
  // Waits until a master card is scanned
  while (!successRead) {
    successRead = getID();
    if ( successRead == true) {
      myTags[tagsCount] = strdup(tagID.c_str()); // Sets the master tag into position 0 in the array
      Serial.println("Master Tag Set!");
      tagsCount++;
    }
  }
  successRead = false;
  printNormalModeMessage();
}

void loop() {

   sensorValue = analogRead(MQ2pin); 
    BT_input = Serial.readString();
  if(BT_input=="1")
  {
    Serial.println("Door Unlocked");
    myServo.write(90);
  }
  if(sensorValue > Threshold)
  {
    Serial.println("Smoke detected!");
    Serial.println("Door Unlocked");
    myServo.write(90);
  }
  
  // If door is closed...
  if (value == 0) {
    if ( ! mfrc522.PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue
      return;
    }
    if ( ! mfrc522.PICC_ReadCardSerial()) {   //Since a PICC placed get Serial and continue
      return;
    }
    tagID = "";
    // The MIFARE PICCs that we use have 4 byte UID
    for ( uint8_t i = 0; i < 4; i++) {  //
      readCard[i] = mfrc522.uid.uidByte[i];
      tagID.concat(String(mfrc522.uid.uidByte[i], HEX)); // Adds the 4 bytes in a single String variable
    }
    tagID.toUpperCase();
    mfrc522.PICC_HaltA(); // Stop reading

    correctTag = false;
    // Checks whether the scanned tag is the master tag
    if (tagID == myTags[0]) {
      Serial.println("Program mode:");
      Serial.println("Add/Remove Tag");
      while (!successRead) {
        successRead = getID();
        if ( successRead == true) {
          for (int i = 0; i < 100; i++) {
            if (tagID == myTags[i]) {
              myTags[i] = "";
              Serial.println("  Tag Removed!");
              printNormalModeMessage();
              return;
            }
          }
          myTags[tagsCount] = strdup(tagID.c_str());
          Serial.println("   Tag Added!");
          printNormalModeMessage();
          tagsCount++;
          return;
        }
      }
    }
    successRead = false;
    // Checks whether the scanned tag is authorized
    for (int i = 0; i < 100; i++) {
      if (tagID == myTags[i]) {
        Serial.println(" Access Granted!");
        myServo.write(90); // Unlocks the door
        printNormalModeMessage();
        correctTag = true;
      }
    }
    if (correctTag == false) {
      Serial.println(" Access Denied!");
      printNormalModeMessage();
    }
  }
  // If door is open...
  else {
    Serial.println(" Door Opened!");
    while (!doorOpened) {
      if (value == 0) {
        doorOpened = true;
      }
    }
    doorOpened = false;
    delay(500);
    myServo.write(180); // Locks the door
    printNormalModeMessage();
  }
}

uint8_t getID() {
  // Getting ready for Reading PICCs
  if ( ! mfrc522.PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue
    return 0;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {   //Since a PICC placed get Serial and continue
    return 0;
  }
  tagID = "";
  for ( uint8_t i = 0; i < 4; i++) {  // The MIFARE PICCs that we use have 4 byte UID
    readCard[i] = mfrc522.uid.uidByte[i];
    tagID.concat(String(mfrc522.uid.uidByte[i], HEX)); // Adds the 4 bytes in a single String variable
  }
  tagID.toUpperCase();
  mfrc522.PICC_HaltA(); // Stop reading
  return 1;
}

void printNormalModeMessage() {
  delay(1500);
  Serial.println("-Access Control-");
  Serial.print(" Scan Your Tag!");
}
