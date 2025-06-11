#include <SPI.h>
#include "mcp2515_can.h"
#include <Arduino.h>

const int SPI_CS_PIN = 9;           // Samma som i fungerande sändare
const int FAN_PIN = A3;             // Fläktstyrning
const int KILL_PIN = 8;
mcp2515_can CAN(SPI_CS_PIN);        // Seed Studio CAN-klassen

void setup() {
  Serial.begin(115200);

  pinMode(FAN_PIN, OUTPUT);
  pinMode(KILL_PIN, INPUT);  // KILL-knapp som är aktiv låg
  digitalWrite(FAN_PIN, LOW);       // Fläkt av från start
  

  // Initiera CAN med 100 kbps
  if (CAN.begin(CAN_100KBPS) == CAN_OK) {
    Serial.println("CAN init OK");
  } else {
    Serial.println("CAN init FAIL");
    while (1);
  }

  // Acceptera alla CAN-ID:n
  CAN.init_Mask(0, 0, 0x7FF);
  CAN.init_Filt(0, 0, 0x000);
}

void loop() {
  unsigned char len = 0;
  unsigned char buf[8];
  unsigned long rxId;

  if (CAN_MSGAVAIL == CAN.checkReceive()) {
    CAN.readMsgBuf(&len, buf);
    rxId = CAN.getCanId();

    Serial.print("Meddelande från ID: 0x");
    Serial.println(rxId, HEX);
    Serial.print("Data (len ");
    Serial.print(len);
    Serial.print("): ");

    //delay(1000); // Vänta 1 sekund innan nästa meddelande

    for (int i = 0; i < len; i++) {
      Serial.print(buf[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    if (digitalRead(KILL_PIN) == LOW) {
      if (rxId == 0x459) {

        if (buf[1] == 0x04) {
          digitalWrite(FAN_PIN, HIGH);
          Serial.println("Fläkt PÅ");
          byte data[] = {0x00}; // Skicka data för att slå på fläkten
          CAN.sendMsgBuf(0x460, 0, 1, data);
        } 
        else if (buf[1] == 0x00) {
          digitalWrite(FAN_PIN, LOW);
          Serial.println("Fläkt AV");
          byte data[] = {0x00}; // Skicka data för att slå på fläkten
          CAN.sendMsgBuf(0x460, 0, 1, data);
        }
      }
    }
    else {
      digitalWrite(FAN_PIN, LOW); // Stäng av fläkten om KILL-knappen är nedtryckt
      Serial.println("KILL-knapp nedtryckt, fläkt AV");
      byte data[] = {0x01};
      CAN.sendMsgBuf(0x460, 0, 1, data);
    }
  }
}
