#include <Wire.h>
// const uint8_t I2C_ADDRESS = 0x21; //Alex Board Protocol
const uint8_t I2C_ADDRESS = 0x20; //Generic Board Protocol
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
const int FILTER_12m = 132;
const int FILTER_10m = 232;

int currentCW = 0;
boolean genericMode = false;

//#define debug

//Pin definitions
#define lpf_board_xf-lpf-hf 
#if defined lpf_board_xf-lpf-hf
int lpf_10m = 5;
int lpf_12m = 4;
int lpf_15m = 3;
int lpf_17m = 2;
int lpf_20m = 9;
int lpf_30m = 10;
int lpf_40m = 11;
int lpf_80m = 12;
int lpf_160m = 13;
#endif

int ptt_pin = 8;
int tx_pin = 7;
int pa_pin = 6;

int currentBand = 0;
boolean transmit = 0;
boolean transmitAllowed = true;

void setup() {

#if defined lpf_board_xf-lpf-hf
  pinMode(lpf_10m, OUTPUT);
  pinMode(lpf_12m, OUTPUT);
  pinMode(lpf_15m, OUTPUT);
  pinMode(lpf_17m, OUTPUT);
  pinMode(lpf_20m, OUTPUT);
  pinMode(lpf_30m, OUTPUT);
  pinMode(lpf_40m, OUTPUT);
  pinMode(lpf_80m, OUTPUT);
  pinMode(lpf_160m, OUTPUT);
  digitalWrite(lpf_10m, LOW);
  digitalWrite(lpf_12m, LOW);
  digitalWrite(lpf_15m, LOW);
  digitalWrite(lpf_17m, LOW);
  digitalWrite(lpf_20m, LOW);
  digitalWrite(lpf_30m, LOW);
  digitalWrite(lpf_40m, LOW);
  digitalWrite(lpf_80m, LOW);
  digitalWrite(lpf_160m, LOW);

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
    #if defined lpf_board_xf-lpf-hf
       digitalWrite(lpf_10m, LOW);
       digitalWrite(lpf_12m, LOW);
       digitalWrite(lpf_15m, LOW);
       digitalWrite(lpf_17m, LOW);
       digitalWrite(lpf_20m, LOW);
       digitalWrite(lpf_30m, LOW);
       digitalWrite(lpf_40m, LOW);
       digitalWrite(lpf_80m, LOW);
       digitalWrite(lpf_160m, HIGH);
      #endif
        break;
      case FILTER_80m:
      #if defined bpf_board_russian
       digitalWrite(lpf_10m, LOW);
       digitalWrite(lpf_12m, LOW);
       digitalWrite(lpf_15m, LOW);
       digitalWrite(lpf_17m, LOW);
       digitalWrite(lpf_20m, LOW);
       digitalWrite(lpf_30m, LOW);
       digitalWrite(lpf_40m, LOW);
       digitalWrite(lpf_80m, HIGH);
       digitalWrite(lpf_160m, LOW);
      #endif
        break;

      case FILTER_40m:
      #if defined bpf_board_russian
       digitalWrite(lpf_10m, LOW);
       digitalWrite(lpf_12m, LOW);
       digitalWrite(lpf_15m, LOW);
       digitalWrite(lpf_17m, LOW);
       digitalWrite(lpf_20m, LOW);
       digitalWrite(lpf_30m, LOW);
       digitalWrite(lpf_40m, HIGH);
       digitalWrite(lpf_80m, LOW);
       digitalWrite(lpf_160m, LOW);
      #endif
        break;

      case FILTER_30m:
      #if defined bpf_board_russian
       digitalWrite(lpf_10m, LOW);
       digitalWrite(lpf_12m, LOW);
       digitalWrite(lpf_15m, LOW);
       digitalWrite(lpf_17m, LOW);
       digitalWrite(lpf_20m, LOW);
       digitalWrite(lpf_30m, HIGH);
       digitalWrite(lpf_40m, LOW);
       digitalWrite(lpf_80m, LOW);
       digitalWrite(lpf_160m, LOW);
      #endif
        break;

      case FILTER_20m:
      #if defined bpf_board_russian
       digitalWrite(lpf_10m, LOW);
       digitalWrite(lpf_12m, LOW);
       digitalWrite(lpf_15m, LOW);
       digitalWrite(lpf_17m, LOW);
       digitalWrite(lpf_20m, HIGH);
       digitalWrite(lpf_30m, LOW);
       digitalWrite(lpf_40m, LOW);
       digitalWrite(lpf_80m, LOW);
       digitalWrite(lpf_160m, LOW);
      #endif
        break;

      case FILTER_17m:
      #if defined bpf_board_russian
       digitalWrite(lpf_10m, LOW);
       digitalWrite(lpf_12m, LOW);
       digitalWrite(lpf_15m, LOW);
       digitalWrite(lpf_17m, HIGH);
       digitalWrite(lpf_20m, LOW);
       digitalWrite(lpf_30m, LOW);
       digitalWrite(lpf_40m, LOW);
       digitalWrite(lpf_80m, LOW);
       digitalWrite(lpf_160m, LOW);
      #endif
        break;

      case FILTER_15m:
      #if defined bpf_board_russian
       digitalWrite(lpf_10m, LOW);
       digitalWrite(lpf_12m, LOW);
       digitalWrite(lpf_15m, HIGH);
       digitalWrite(lpf_17m, LOW);
       digitalWrite(lpf_20m, LOW);
       digitalWrite(lpf_30m, LOW);
       digitalWrite(lpf_40m, LOW);
       digitalWrite(lpf_80m, LOW);
       digitalWrite(lpf_160m, LOW);
      #endif

        break;
      case FILTER_12m:
      #if defined bpf_board_russian
       digitalWrite(lpf_10m, LOW);
       digitalWrite(lpf_12m, HIGH);
       digitalWrite(lpf_15m, LOW);
       digitalWrite(lpf_17m, LOW);
       digitalWrite(lpf_20m, LOW);
       digitalWrite(lpf_30m, LOW);
       digitalWrite(lpf_40m, LOW);
       digitalWrite(lpf_80m, LOW);
       digitalWrite(lpf_160m, LOW);
      #endif
        break;

       case FILTER_10m:
       #if defined bpf_board_russian
       digitalWrite(lpf_10m, HIGH);
       digitalWrite(lpf_12m, LOW);
       digitalWrite(lpf_15m, LOW);
       digitalWrite(lpf_17m, LOW);
       digitalWrite(lpf_20m, LOW);
       digitalWrite(lpf_30m, LOW);
       digitalWrite(lpf_40m, LOW);
       digitalWrite(lpf_80m, LOW);
       digitalWrite(lpf_160m, LOW);
       #endif
     
        break;

      
    }
  }
}
