#include <Wire.h>
const uint8_t I2C_ADDRESS = 0x21; //Alex Board Protocol
// const uint8_t I2C_ADDRESS = 0x20; //Generic Board Protocol
uint8_t received;
uint8_t message;
const uint8_t COMMAND_ON_OFF = 0x01;
const uint8_t COMMAND_SPEED = 0x02;

const int FILTER_160M = 8;
const int FILTER_80m = 4;
const int FILTER_40m = 802;
const int FILTER_30m = 401;
const int FILTER_20m = 101;
const int FILTER_17m = 164;
const int FILTER_15m = 264;
const int FILTER_12m = 464;
const int FILTER_10m = 232;

int currentCW = 0;
boolean genericMode = false;

//#define debug

//Pin definitions
#define bpf_board_russian

#if defined bpf_board_russian
int bpf_pin1 = 5;  //10M
int bpf_pin2 = 4;  //12M
int bpf_pin3 = 3;  //15M
int bpf_pin4 = 2;  //17M
int bpf_pin5 = 9;  //20M
int bpf_pin6 = 10; //30M
int bpf_pin7 = 11; //40M
int bpf_pin8 = 12; //80M
int bpf_pin9 = 13; //160M
#endif

int ptt_pin = 8;
int tx_pin = 7;
int pa_pin = 6;

int currentBand = 0;
boolean transmit = 0;
boolean transmitAllowed = true;

void setup() {


#if defined bpf_board_russian
  pinMode(bpf_pin1, OUTPUT);
  pinMode(bpf_pin2, OUTPUT);
  pinMode(bpf_pin3, OUTPUT);
  pinMode(bpf_pin4, OUTPUT);
  pinMode(bpf_pin5, OUTPUT);
  pinMode(bpf_pin6, OUTPUT);
  pinMode(bpf_pin7, OUTPUT);
  pinMode(bpf_pin8, OUTPUT);
  pinMode(bpf_pin9, OUTPUT);
  digitalWrite(bpf_pin1, LOW);
  digitalWrite(bpf_pin2, LOW);
  digitalWrite(bpf_pin3, LOW);
  digitalWrite(bpf_pin4, LOW);
  digitalWrite(bpf_pin5, LOW);
  digitalWrite(bpf_pin6, LOW);
  digitalWrite(bpf_pin7, LOW);
  digitalWrite(bpf_pin8, LOW);
  digitalWrite(bpf_pin9, LOW);

#endif

  //Start i2c as slave
  Wire.begin(I2C_ADDRESS);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);

  pinMode(ptt_pin, INPUT);
  pinMode(tx_pin, OUTPUT);
  digitalWrite(tx_pin, LOW);
  pinMode(pa_pin, OUTPUT);
  digitalWrite(pa_pin, LOW);

#if defined debug
  Serial.begin(115200);
#endif
}
void requestEvent()
{
  if (received == COMMAND_ON_OFF) {

  } else {
    Wire.write(0);
  }
}
int started = 0;
void loop() {

  int ptt = digitalRead(ptt_pin);
  if (ptt == HIGH && currentCW == 1) {
    processPTT(ptt);
  } else if (ptt == LOW && currentCW == 1) {
    processPTT(ptt);
  }
  delay(50);
}
void toggleTxmit(int ptt) {
  if (transmitAllowed == true) {
    transmit = ptt;
    if (ptt == 1) {
      digitalWrite(tx_pin, HIGH);
      digitalWrite(pa_pin, HIGH);
    } else {
      digitalWrite(tx_pin, LOW);
      digitalWrite(pa_pin, LOW);
    }
  }
}
void receiveEvent(int bytes) {

  int byteCount = 0;
  int command = 0;
  uint8_t byte1 = 0;
  uint8_t byte2 = 0;
  uint8_t byte3 = 0;
  int freqHigh [8] = {0, 0, 0, 0, 0, 0, 0, 0};
  while (0 < Wire.available()) {
    byte x = Wire.read();
    if (genericMode == false) {
      if (byteCount == 0) {
        byte1 = x;
      }
      if (byteCount == 1) {
        byte2 = x;
        command = command + (byte2 * 100);
      }
      if (byteCount == 2) {
        byte3 = x;
        command = command + byte3;
      }
      if (byte1 == 2 && byte2 == 2 && byte3 == 3) {
        genericMode = true;
        byte1 = 0;
        //#if defined debug
        //        Serial.println("Generic Mode set to true");
        //#endif
      }
      byteCount = byteCount + 1;
    } else {

      if (byteCount == 0) {
        byte1 = x;
      }
      if (byteCount == 1) {
        byte2 = x;
        command = command + (byte2 * 100);
      }
      if (byteCount == 2) {
        byte3 = x;
        command = command + byte3;
      }

      if (byteCount > 0) {
        freqHigh[byteCount - 1] = x;
      }

      byteCount = byteCount + 1;
    }
  }

#if defined debug
  Serial.print("Command Status: ");
  Serial.println(command);
  Serial.print("Byte1 Status: ");
  Serial.println(byte1);
  Serial.print("Byte2 Status: ");
  Serial.println(byte2);
  Serial.print("Byte3 Status: ");
  Serial.println(byte3);
  Serial.print("freq Received: ");
  Serial.print(freqHigh[0]);
  Serial.print(freqHigh[1]);
  Serial.print(freqHigh[2]);
  Serial.print(freqHigh[3]);
  Serial.print(freqHigh[4]);
  Serial.print(freqHigh[5]);
  Serial.print(freqHigh[6]);
  Serial.println(freqHigh[7]);
#endif
  int pttTrig = digitalRead(ptt_pin);
  if (byte1 == 3 && pttTrig == HIGH && byte2 > 0) {
    currentCW = byte3;
    processPTT(byte2);
    command = 0;
  } else if (byte1 == 3 && pttTrig == HIGH && byte3 > 0) {
    currentCW = byte3;
    processPTT(byte3);
    command = 0;
  } else if (byte1 == 3) {
    currentCW = byte3;
    processPTT(0);

  } else {
    currentCW = 0;
    processPTT(0);
  }
  if (byte1 == 4) {
    processFrequency(freqHigh);
    command = 0;

  }
  if (command != 0) {
#if defined debug
    Serial.print("Command Sended: ");
    Serial.println(command);
#endif
    processCommand(command);
  }

}
void processPTT(int command) {
  digitalWrite(tx_pin, command);
  digitalWrite(pa_pin, command);
}
void processFrequency(int command) {

}
void processCommand(int command) {
  started =  1;
  if (currentBand != command) {
    currentBand = command;
    switch (command) {
      case FILTER_160M:
    #if defined bpf_board_russian
       digitalWrite(bpf_pin1, LOW);
       digitalWrite(bpf_pin2, LOW);
       digitalWrite(bpf_pin3, LOW);
       digitalWrite(bpf_pin4, LOW);
       digitalWrite(bpf_pin5, LOW);
       digitalWrite(bpf_pin6, LOW);
       digitalWrite(bpf_pin7, LOW);
       digitalWrite(bpf_pin8, LOW);
       digitalWrite(bpf_pin9, HIGH);
      #endif
        break;
      case FILTER_80m:
      #if defined bpf_board_russian
       digitalWrite(bpf_pin1, LOW);
       digitalWrite(bpf_pin2, LOW);
       digitalWrite(bpf_pin3, LOW);
       digitalWrite(bpf_pin4, LOW);
       digitalWrite(bpf_pin5, LOW);
       digitalWrite(bpf_pin6, LOW);
       digitalWrite(bpf_pin7, LOW);
       digitalWrite(bpf_pin8, HIGH);
       digitalWrite(bpf_pin9, LOW);
      #endif
        break;

      case FILTER_40m:
      #if defined bpf_board_russian
       digitalWrite(bpf_pin1, LOW);
       digitalWrite(bpf_pin2, LOW);
       digitalWrite(bpf_pin3, LOW);
       digitalWrite(bpf_pin4, LOW);
       digitalWrite(bpf_pin5, LOW);
       digitalWrite(bpf_pin6, LOW);
       digitalWrite(bpf_pin7, HIGH);
       digitalWrite(bpf_pin8, LOW);
       digitalWrite(bpf_pin9, LOW);
      #endif
        break;

      case FILTER_30m:
      #if defined bpf_board_russian
       digitalWrite(bpf_pin1, LOW);
       digitalWrite(bpf_pin2, LOW);
       digitalWrite(bpf_pin3, LOW);
       digitalWrite(bpf_pin4, LOW);
       digitalWrite(bpf_pin5, LOW);
       digitalWrite(bpf_pin6, HIGH);
       digitalWrite(bpf_pin7, LOW);
       digitalWrite(bpf_pin8, LOW);
       digitalWrite(bpf_pin9, LOW);
      #endif
        break;

      case FILTER_20m:
      #if defined bpf_board_russian
       digitalWrite(bpf_pin1, LOW);
       digitalWrite(bpf_pin2, LOW);
       digitalWrite(bpf_pin3, LOW);
       digitalWrite(bpf_pin4, LOW);
       digitalWrite(bpf_pin5, HIGH);
       digitalWrite(bpf_pin6, LOW);
       digitalWrite(bpf_pin7, LOW);
       digitalWrite(bpf_pin8, LOW);
       digitalWrite(bpf_pin9, LOW);
      #endif
        break;

      case FILTER_17m:
      #if defined bpf_board_russian
       digitalWrite(bpf_pin1, LOW);
       digitalWrite(bpf_pin2, LOW);
       digitalWrite(bpf_pin3, LOW);
       digitalWrite(bpf_pin4, HIGH);
       digitalWrite(bpf_pin5, LOW);
       digitalWrite(bpf_pin6, LOW);
       digitalWrite(bpf_pin7, LOW);
       digitalWrite(bpf_pin8, LOW);
       digitalWrite(bpf_pin9, LOW);
      #endif
        break;

      case FILTER_15m:
      #if defined bpf_board_russian
       digitalWrite(bpf_pin1, LOW);
       digitalWrite(bpf_pin2, LOW);
       digitalWrite(bpf_pin3, HIGH);
       digitalWrite(bpf_pin4, LOW);
       digitalWrite(bpf_pin5, LOW);
       digitalWrite(bpf_pin6, LOW);
       digitalWrite(bpf_pin7, LOW);
       digitalWrite(bpf_pin8, LOW);
       digitalWrite(bpf_pin9, LOW);
      #endif

        break;
      case FILTER_12m:
      #if defined bpf_board_russian
       digitalWrite(bpf_pin1, LOW);
       digitalWrite(bpf_pin2, HIGH);
       digitalWrite(bpf_pin3, LOW);
       digitalWrite(bpf_pin4, LOW);
       digitalWrite(bpf_pin5, LOW);
       digitalWrite(bpf_pin6, LOW);
       digitalWrite(bpf_pin7, LOW);
       digitalWrite(bpf_pin8, LOW);
       digitalWrite(bpf_pin9, LOW);
      #endif
        break;

       case FILTER_10m:
       #if defined bpf_board_russian
       digitalWrite(bpf_pin1, HIGH);
       digitalWrite(bpf_pin2, LOW);
       digitalWrite(bpf_pin3, LOW);
       digitalWrite(bpf_pin4, LOW);
       digitalWrite(bpf_pin5, LOW);
       digitalWrite(bpf_pin6, LOW);
       digitalWrite(bpf_pin7, LOW);
       digitalWrite(bpf_pin8, LOW);
       digitalWrite(bpf_pin9, LOW);
       #endif
     
        break;

      
    }
  }
}
