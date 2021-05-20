/*
  Example for enabling multiple receiving pins.
  Do NOT forget to adjust RCSWITCH_MAX_RX_PINS in RCSwitch.h according to your configuration!
  Connect your receive module to the pins and open serial monitor at 115200bps.
*/

#define PIN_315 14  // NodeMCU D5
#define PIN_433 13  // NodeMCU D7

#include "RCSwitch.h"

// Do not instantiate multiple RCSwitch (can cause crash), use single instance and write multiple enableReceive() for multi receiving.
RCSwitch mySwitch = RCSwitch();

static char *bin2tristate(char *bin) {
  char returnValue[50];
  for (int i=0; i<50; i++) {
    returnValue[i] = '\0';
  }
  int pos = 0;
  int pos2 = 0;
  while (bin[pos]!='\0' && bin[pos+1]!='\0') {
    if (bin[pos]=='0' && bin[pos+1]=='0') {
      returnValue[pos2] = '0';
    } else if (bin[pos]=='1' && bin[pos+1]=='1') {
      returnValue[pos2] = '1';
    } else if (bin[pos]=='0' && bin[pos+1]=='1') {
      returnValue[pos2] = 'F';
    } else {
      return "not applicable";
    }
    pos = pos+2;
    pos2++;
  }
  returnValue[pos2] = '\0';
  return returnValue;
}

void output(unsigned char pin, unsigned long decimal, unsigned int length, unsigned int delay, unsigned int* raw, unsigned int protocol) {
  Serial.println("-------------------------------------");
  if(pin == PIN_315)
    Serial.println("315MHz RECV");
  else if(pin == PIN_433)
    Serial.println("433MHz RECV");
  if (decimal == 0) {
    Serial.print("Unknown encoding.");
  } else {
    char* b = mySwitch.dec2binWzerofill(decimal, length);
    char* tristate = bin2tristate(b);

    Serial.print("Decimal: ");
    Serial.print(decimal);
    Serial.print(" (");
    Serial.print( length );
    Serial.print("Bit) Binary: ");
    Serial.print( b );
    Serial.print(" Tri-State: ");
    Serial.print( tristate );
    Serial.print(" PulseLength: ");
    Serial.print(delay);
    Serial.print(" microseconds");
    Serial.print(" Protocol: ");
    Serial.println(protocol);

    //Spark.publish("tristate-received", String(delay) + " " + String(tristate));
  }

  Serial.print("Raw data: ");
  for (int i=0; i<= length*2; i++) {
    Serial.print(raw[i]);
    Serial.print(",");
  }
  Serial.println();
  Serial.println();
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_315, INPUT);
  pinMode(PIN_433, INPUT);
  mySwitch.enableReceive(PIN_315);
  mySwitch.enableReceive(PIN_433);
  Serial.println("Listening");
}

void loop() {
  if (mySwitch.available()) {
    int receivedPin = mySwitch.getReceivedPin();
    output(
        receivedPin,
        mySwitch.getReceivedValue(receivedPin),
        mySwitch.getReceivedBitlength(receivedPin),
        mySwitch.getReceivedDelay(receivedPin),
        mySwitch.getReceivedRawdata(receivedPin),
        mySwitch.getReceivedProtocol(receivedPin)
        );
    mySwitch.resetAvailable();
  }
}
