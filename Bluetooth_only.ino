//Libraries and C files to include
#include <SPI.h>
#include <Arduino.h>
#include "Adafruit_BluefruitLE_UART.h"
#include "Adafruit_BLE.h"

//Bluetooth variables
#define BUFSIZE                        128   // Size of the read buffer for incoming data
#define VERBOSE_MODE                   true 
#define BLUEFRUIT_HWSERIAL_NAME Serial1
#define BLUEFRUIT_UART_MODE_PIN -1
Adafruit_BluefruitLE_UART ble(BLUEFRUIT_HWSERIAL_NAME, BLUEFRUIT_UART_MODE_PIN);

//ADC variables
int Sample = 1;
char answer = 'a';
int n;
int val = 0;
float voltage = 0.0;
byte upperByte; //ADC upper byte
byte lowerByte; //ADC lower byte
int SegmentSize = 1800;//10002
int SPIBuffSize = 100;
int StorageSpace = SegmentSize *2 + SPIBuffSize * 2;

//Storage Variables
int * ptrdata;
int peakstartindex;
int upperThresh = 28002;//Threshold to mark start of primary wave 1.4V 28002
int lowerThresh = 23830;//Theshold to mark end of button pressing 1.2V 23830
int LeftoverStart = 0;
int indxbuff = 0;
char buf[16];
long lval;
int doneFlag = 0;

void setup() {

Serial.begin(115200);                 //Arduino Serial Monitor

SPI.begin();                          //SPI begin for SPI.h function
BLUEFRUIT_HWSERIAL_NAME.begin(9600);  //Hardware Serial Monitor

//RAM initialization

 /* Initialise the module */
//Serial.println(F("Welcome to the shit show"));

 /* Disable command echo and berbose from Bluefruit (only need for debugging*/
ble.echo(false);
ble.verbose(false);
ble.info();
//Wait until it is disconected
while (! ble.isConnected()) {
  Serial.println("connect bluetooth");
  delay(5000);
} 
ble.info();

//Serial.print("AT+BAUDRATE");
//Serial.println(115200);
//
//ble.print("AT+BAUDRATE");
//ble.println(false);
}

void loop(){
  if (doneFlag == 0) {
    Serial.println("start data transfer");
      ptrdata = (int*) malloc(sizeof((StorageSpace + 200)));
      if (ptrdata == NULL){
      Serial.println("space not allocated");
      }
      
     for(int i = 1; i < (SegmentSize + SPIBuffSize) ; i++){
     
      ptrdata[indxbuff] = highByte(i); //Send 00000000
      ptrdata[indxbuff + 1] = lowByte(i); //Send 00000000
      indxbuff = indxbuff + 2;
    }
    indxbuff = 0;
  
    for(int n = 1; n < 1000; n++){
      ble.print("AT+BLEUARTTX=");
      ble.println(n);
    }
//      for(int n = 1; n < (SegmentSize + SPIBuffSize); n++){  //This is what sends the bluetooth data
//        ble.print("AT+BLEUARTTX=");
//        //delay(1);
//        lval = (ptrdata[indxbuff]<<8) + ptrdata[indxbuff + 1];
//        ltoa(lval,buf,2);
//        ble.println(buf);
//  //      Serial.println(buf);
//  //      Serial.println((ptrdata[indxbuff]<<8) + ptrdata[indxbuff + 1]);
//  //      ble.print("AT+BLEUARTTX=");
//  //      delay(1);
//  //      ble.println(",");
//  //      delay(1);
//        indxbuff = indxbuff + 2;
//      }
      indxbuff = 0;
  
      free(ptrdata);
      Serial.println("end data transfer");
      doneFlag = 1;
      delay(10000);
      //Begin Bluetooth
      char inputs[BUFSIZE+1];
  
      if ( getUserInput(inputs, BUFSIZE)){
        // Send characters to Bluefruit
        Serial.print("[Send] ");
        Serial.println(inputs);
        ble.print("AT+BLEUARTTX=");
        ble.println(inputs);
  
        // check response stastus
        if (! ble.waitForOK() ) {
          Serial.println(F("Failed to send?"));
        }
      }
    
      // Check for incoming characters from Bluefruit
      ble.println("AT+BLEUARTRX");
      ble.readline();
      if (strcmp(ble.buffer, "OK") == 0) {
        // no data
        return;
      }
    
      // Some data was found, its in the buffer
      Serial.print(F("[Recv] ")); 
      answer = *ble.buffer;
      Serial.println(ble.buffer);
      ble.waitForOK();
}
}

bool getUserInput(char buffer[], uint8_t maxSize)
{
  // timeout in 100 milliseconds
  TimeoutTimer timeout(100);

  memset(buffer, 0, maxSize);
  while( (!Serial.available()) && !timeout.expired() ) { delay(1); }

  if ( timeout.expired() ) return false;

  delay(2);
  uint8_t count=0;
  do
  {
    count += Serial.readBytes(buffer+count, maxSize);
    delay(2);
  } while( (count < maxSize) && (Serial.available()) );

  return true;
}
