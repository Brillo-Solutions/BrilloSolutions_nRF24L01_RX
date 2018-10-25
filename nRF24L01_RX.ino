/*Written By: Er. Dapinder Singh Virk
  Email ID: brillosolutions@gmail.com
  Version: 1.0.0
  Date: October 25, 2018
  Outputs: Receives toggled 1-Byte data at every iteration and turns ON/OFF LED. */

#include <SPI.h>
#define LED_pin 8
#define CE_pin 9
#define CSN_pin 10
#define MOSI_pin 11
#define MISO_pin 12
#define SCK_pin 13

#define CONFIG 0x00
#define STATUS 0x07
#define RX_PW_P0 0x11

#define R_REGISTER B00000000
#define FLUSH_RX B11100010
#define R_RX_PAYLOAD B01100001
void setup() 
{
  Serial.begin(9600);
  SPI.begin();
  pinMode(LED_pin, OUTPUT); 
  pinMode(CE_pin, OUTPUT);                                  //  Setting CE pin as output.
  pinMode(CSN_pin, OUTPUT);                                 //  Setting CSN pin as output.
  pinMode(MOSI_pin, OUTPUT);                                //  Setting MOSI pin as output.
  pinMode(MISO_pin, INPUT);                                 //  Setting MISO pin as input.
  pinMode(SCK_pin, OUTPUT);                                 //  Setting SCK pin as output.
  digitalWrite(CSN_pin, HIGH);                              //  Setting CSN = 1 to make SPI idle.
  writeReg(CONFIG, 0x0B);                                   //  Setting PRIM_RX = 1 and PWR_UP = 1 in CONFIG register to putting radio in RX and standby mode (I) or (II).
  writeReg(RX_PW_P0, 0x01);                                 //  Setting the 1 byte length in RX payload for data pipe 0 using RX_PW_P0 register that is to be received.                 
}

void loop() 
{
  digitalWrite(CE_pin, HIGH);                               // Making radio ready to get into TX/RX mode.        
  uint8_t rxdByte = getPayload();                     
  if (rxdByte == 0x55)                                      // Receiving payload that has been dis-assembled from packet and checking for data.
    digitalWrite(LED_pin, HIGH);
  else if(rxdByte == 0xAA)
    digitalWrite(LED_pin, LOW);
  Serial.println(rxdByte, HEX);
  digitalWrite(CE_pin, LOW);                                // Falling back to standby mode (I) or (II).
}

void writeReg(byte ADDR, byte CONTENT)
{
  digitalWrite(CSN_pin, LOW);                               // Setting CSN = 0 to make SPI active.
  SPI.transfer(32 + ADDR);                                  // Address of register where content is to be written.
  SPI.transfer(CONTENT);                                    // Actual content that is to be written.
  digitalWrite(CSN_pin, HIGH);                              // Setting CSN = 1 to make SPI idle.
}

byte readReg(byte ADDR)
{
  byte regVal;
  digitalWrite(CSN_pin, LOW);                               // Setting CSN = 0 to make SPI active.
  SPI.transfer(ADDR);                                       // Address of register from where content is to be read.
  regVal = SPI.transfer(R_REGISTER);                        // Reading the content of register with command: R_REGISTER.
  digitalWrite(CSN_pin, HIGH);                              // Setting CSN = 1 to make SPI idle.
  return regVal;
}

void flushRx()
{
  digitalWrite(CSN_pin, LOW);                               // Setting CSN = 0 to make SPI active.
  SPI.transfer(FLUSH_RX);                                   // Making RX FIFO empty by using command: FLUSH_RX.
  digitalWrite(CSN_pin, HIGH);                              // Setting CSN = 1 to make SPI idle. 
}

byte getPayload()
{
  byte rxPayload;
  flushRx();                                                // Flushing the RX FIFO.
  digitalWrite(CSN_pin, LOW);                               // Setting CSN = 0 to make SPI active.
  SPI.transfer(R_RX_PAYLOAD);                               // Targetting RX FIFO address by using command: R_RX_PAYLOAD.
  rxPayload = SPI.transfer(R_REGISTER);                     // Reading the content of RX FIFO with command: R_REGISTER.
  digitalWrite(CSN_pin, HIGH);                              // Setting CSN = 1 to make SPI idle.
  writeReg(STATUS, 0x70);                                   // Clearing all interrupts including RX_DR in STATUS register.
  return (rxPayload);
}
