/*  
  OpenMQTTGateway  - ESP8266 or Arduino program for home automation 

   Act as a wifi or ethernet gateway between your 433mhz/infrared IR signal  and a MQTT broker 
   Send and receiving command by MQTT
 
   This files enables to set your parameter for the radiofrequency gateways (ZgatewayRF and ZgatewayRF2) with RCswitch and newremoteswitch library
  
    Copyright: (c)Florian ROBERT
  
    This file is part of OpenMQTTGateway.
    
    OpenMQTTGateway is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenMQTTGateway is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef config_RF_h
#define config_RF_h

#include <Arduino.h>
#include <ArduinoJson.h>

#ifdef ZgatewayRF315
extern void setupRF315();
extern void RF315toMQTT();
extern void MQTTtoRF315(char* topicOri, char* datacallback);
extern void MQTTtoRF315(char* topicOri, JsonObject& RFdata);
#endif

/**
 * minimumRssi minimum RSSI value to enable receiver
 */
int minimumRssi = 0;
#endif


/*-------------------RF315 topics & parameters----------------------*/
//315Mhz MQTT Subjects and keys
#define subjectMQTTtoRF315    "/commands/MQTTto315"
#define subjectRF315toMQTT    "/315toMQTT"
#define subjectGTWRF315toMQTT "/315toMQTT"
#define RF315protocolKey      "315_" // protocol will be defined if a subject contains RFprotocolKey followed by a value of 1 digit
#define RF315bitsKey          "RF315BITS_" // bits  will be defined if a subject contains RFbitsKey followed by a value of 2 digits
#define repeatRF315wMQTT      false // do we repeat a received signal by using mqtt with RF gateway
#define RF315pulselengthKey   "PLSL_" // pulselength will be defined if a subject contains RFprotocolKey followed by a value of 3 digits
// subject monitored to listen traffic processed by other gateways to store data and avoid ntuple
#define subjectMultiGTWRF315 "+/+/315toMQTT"
//RF number of signal repetition - Can be overridden by specifying "repeat" in a JSON message.
#define RF315_EMITTER_REPEAT 20
//#define RF315_DISABLE_TRANSMIT //Uncomment this line to disable RF transmissions. (RF Receive will work as normal.)



/*-------------------CC1101 frequency----------------------*/
//Match frequency to the hardware version of the radio if ZradioCC1101 is used.
#ifndef CC1101_FREQUENCY_315
#  define CC1101_FREQUENCY_315 315
#endif
// Allow ZGatewayRF Module to change receive frequency of CC1101 Transceiver module
#ifdef ZradioCC1101
float receiveMhz315 = CC1101_FREQUENCY_315;
#endif

/*-------------------PIN DEFINITIONS----------------------*/
#ifndef RF315_RECEIVER_GPIO
#  ifdef ESP8266
#    define RF315_RECEIVER_GPIO 10 // D3 on nodemcu // put 4 with rf bridge direct mod
#  elif ESP32
#    define RF315_RECEIVER_GPIO 13 // D27 on DOIT ESP32
#  elif __AVR_ATmega2560__
#    define RF315_RECEIVER_GPIO 1 //1 = D3 on mega
#  else
#    define RF315_RECEIVER_GPIO 1 //1 = D3 on arduino
#  endif
#endif

#ifndef RF315_EMITTER_GPIO
#  ifdef ESP8266
#    define RF315_EMITTER_GPIO 13 // RX on nodemcu if it doesn't work with 3, try with 4 (D2) // put 5 with rf bridge direct mod
#  elif ESP32
#    define RF315_EMITTER_GPIO 2 // D12 on DOIT ESP32
#  elif __AVR_ATmega2560__
#    define RF315_EMITTER_GPIO 4
#  else
//IMPORTANT NOTE: On arduino UNO connect IR emitter pin to D9 , comment #define IR_USE_TIMER2 and uncomment #define IR_USE_TIMER1 on library <library>IRremote/boarddefs.h so as to free pin D3 for RF RECEIVER PIN
//RF PIN definition
#    define RF315_EMITTER_GPIO 4 //4 = D4 on arduino
#  endif
#endif
