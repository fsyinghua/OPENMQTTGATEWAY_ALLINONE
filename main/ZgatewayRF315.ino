/*  
  OpenMQTTGateway  - ESP8266 or Arduino program for home automation 

   Act as a wifi or ethernet gateway between your 433mhz/infrared IR signal  and a MQTT broker 
   Send and receiving command by MQTT
 
  This gateway enables to:
 - receive MQTT data from a topic and send RF 433Mhz signal corresponding to the received MQTT data
 - publish MQTT data to a different topic related to received 433Mhz signal

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
#include "User_config.h"

#ifdef ZgatewayRF315

#  ifdef ZradioCC1101
#    include <ELECHOUSE_CC1101_SRC_DRV.h>
#  endif

#  include <RCSwitch.h> // library for controling Radio frequency switch

RCSwitch mySwitch315 = RCSwitch();

#  if defined(ZmqttDiscovery) && !defined(RF315_DISABLE_TRANSMIT)
void RF315toMQTTdiscovery(SIGNAL_SIZE_UL_ULL MQTTvalue) { //on the fly switch creation from received RF315 values
  char val[11];
  sprintf(val, "%lu", MQTTvalue);
  Log.trace(F("switchRF315Discovery" CR));
  char* switchRF315[8] = {"switch", val, "", "", "", val, "", ""};
  //component type,name,availability topic,device class,value template,payload on, payload off, unit of measurement

  Log.trace(F("CreateDiscoverySwitch: %s" CR), switchRF315[1]);
  createDiscovery(switchRF315[0],
                  subjectRF315toMQTT, switchRF315[1], (char*)getUniqueId(switchRF315[1], switchRF315[2]).c_str(),
                  will_Topic, switchRF315[3], switchRF315[4],
                  switchRF315[5], switchRF315[6], switchRF315[7],
                  0, "", "", true, subjectMQTTtoRF315,
                  "", "", "", "", false);
}
#  endif

void setupRF315() {
  //RF315 init parameters
  Log.notice(F("RF315_EMITTER_GPIO: %d " CR), RF315_EMITTER_GPIO);
  Log.notice(F("RF315_RECEIVER_GPIO: %d " CR), RF315_RECEIVER_GPIO);
#  ifdef ZradioCC1101 //receiving with CC1101
  ELECHOUSE_cc1101.Init();
  ELECHOUSE_cc1101.SetRx(receiveMhz);
#  endif
#  ifdef RF315_DISABLE_TRANSMIT
  mySwitch315.disableTransmit();
#  else
  mySwitch315.enableTransmit(RF315_EMITTER_GPIO);
#  endif
  mySwitch315.setRepeatTransmit(RF315_EMITTER_REPEAT);
  mySwitch315.enableReceive(RF315_RECEIVER_GPIO);
  Log.trace(F("ZgatewayRF315 setup done" CR));
}

void RF315toMQTT() {
    int receivedPin = mySwitch.getReceivedPin();  //get muti 

  if (mySwitch315.available() && receivedPin == RF315_RECEIVER_GPIO) {
#  ifdef ZradioCC1101 //receiving with CC1101
    const int JSON_MSG_CALC_BUFFER = JSON_OBJECT_SIZE(5);
#  else
    const int JSON_MSG_CALC_BUFFER = JSON_OBJECT_SIZE(4);
#  endif
    StaticJsonBuffer<JSON_MSG_CALC_BUFFER> jsonBuffer;
    JsonObject& RF315data = jsonBuffer.createObject();
    Log.trace(F("Rcv. RF315" CR));
#  ifdef ESP32
    Log.trace(F("RF315 Task running on core :%d" CR), xPortGetCoreID());
#  endif
 
    SIGNAL_SIZE_UL_ULL MQTTvalue = mySwitch315.getReceivedValue(receivedPin);
    RF315data.set("value", (SIGNAL_SIZE_UL_ULL)MQTTvalue);
    RF315data.set("protocol", (int)mySwitch315.getReceivedProtocol(receivedPin));
    RF315data.set("length", (int)mySwitch315.getReceivedBitlength(receivedPin));
    RF315data.set("delay", (int)mySwitch315.getReceivedDelay(receivedPin));
#  ifdef ZradioCC1101 // set Receive off and Transmitt on
    RF315data.set("mhz", receiveMhz);
#  endif
    mySwitch315.resetAvailable();

    if (!isAduplicateSignal(MQTTvalue) && MQTTvalue != 0) { // conditions to avoid duplications of RF315 -->MQTT
#  if defined(ZmqttDiscovery) && !defined(RF315_DISABLE_TRANSMIT) //component creation for HA
      if (disc)
        RF315toMQTTdiscovery(MQTTvalue);
#  endif
      pub(subjectRF315toMQTT, RF315data);
      // Casting "receivedSignal[o].value" to (unsigned long) because ArduinoLog doesn't support uint64_t for ESP's
      Log.trace(F("Store val: %u" CR), (unsigned long)MQTTvalue);
      storeSignalValue(MQTTvalue);
      if (repeatRF315wMQTT) {
        Log.trace(F("Pub RF315 for rpt" CR));
        pub(subjectMQTTtoRF315, RF315data);
      }
    }
  }
}

#  ifdef simpleReceiving
void MQTTtoRF315(char* topicOri, char* datacallback) {
#    ifdef ZradioCC1101 // set Receive off and Transmitt on
  ELECHOUSE_cc1101.SetTx(receiveMhz);
  mySwitch315.disableReceive();
  mySwitch315.enableTransmit(RF315_EMITTER_GPIO);
#    endif
  SIGNAL_SIZE_UL_ULL data = STRTO_UL_ULL(datacallback, NULL, 10); // we will not be able to pass values > 4294967295 on Arduino boards

  // RF315 DATA ANALYSIS
  //We look into the subject to see if a special RF315 protocol is defined
  String topic = topicOri;
  int valuePRT = 0;
  int valuePLSL = 0;
  int valueBITS = 0;
  int pos = topic.lastIndexOf(RF315protocolKey);
  if (pos != -1) {
    pos = pos + +strlen(RF315protocolKey);
    valuePRT = (topic.substring(pos, pos + 1)).toInt();
  }
  //We look into the subject to see if a special RF315 pulselength is defined
  int pos2 = topic.lastIndexOf(RF315pulselengthKey);
  if (pos2 != -1) {
    pos2 = pos2 + strlen(RF315pulselengthKey);
    valuePLSL = (topic.substring(pos2, pos2 + 3)).toInt();
  }
  int pos3 = topic.lastIndexOf(RF315bitsKey);
  if (pos3 != -1) {
    pos3 = pos3 + strlen(RF315bitsKey);
    valueBITS = (topic.substring(pos3, pos3 + 2)).toInt();
  }

  if ((cmpToMainTopic(topicOri, subjectMQTTtoRF315)) && (valuePRT == 0) && (valuePLSL == 0) && (valueBITS == 0)) {
    Log.trace(F("MQTTtoRF315 dflt" CR));
    mySwitch315.setProtocol(1, 350);
    mySwitch315.send(data, 24);
    // Acknowledgement to the GTWRF315 topic
    pub(subjectGTWRF315toMQTT, datacallback);
  } else if ((valuePRT != 0) || (valuePLSL != 0) || (valueBITS != 0)) {
    Log.trace(F("MQTTtoRF315 usr par." CR));
    if (valuePRT == 0)
      valuePRT = 1;
    if (valuePLSL == 0)
      valuePLSL = 350;
    if (valueBITS == 0)
      valueBITS = 24;
    Log.notice(F("RF315 Protocol:%d" CR), valuePRT);
    Log.notice(F("RF315 Pulse Lgth: %d" CR), valuePLSL);
    Log.notice(F("Bits nb: %d" CR), valueBITS);
    mySwitch315.setProtocol(valuePRT, valuePLSL);
    mySwitch315.send(data, valueBITS);
    // Acknowledgement to the GTWRF315 topic
    pub(subjectGTWRF315toMQTT, datacallback); // we acknowledge the sending by publishing the value to an acknowledgement topic, for the moment even if it is a signal repetition we acknowledge also
  }
#    ifdef ZradioCC1101 // set Receive on and Transmitt off
  ELECHOUSE_cc1101.SetRx(receiveMhz);
  mySwitch315.disableTransmit();
  mySwitch315.enableReceive(RF315_RECEIVER_GPIO);
#    endif
}
#  endif

#  ifdef jsonReceiving
void MQTTtoRF315(char* topicOri, JsonObject& RF315data) { // json object decoding
  if (cmpToMainTopic(topicOri, subjectMQTTtoRF315)) {
    Log.trace(F("MQTTtoRF315 json" CR));
    SIGNAL_SIZE_UL_ULL data = RF315data["value"];
    if (data != 0) {
      int valuePRT = RF315data["protocol"] | 1;
      int valuePLSL = RF315data["delay"] | 350;
      int valueBITS = RF315data["length"] | 24;
      int valueRPT = RF315data["repeat"] | RF315_EMITTER_REPEAT;
      Log.notice(F("RF315 Protocol:%d" CR), valuePRT);
      Log.notice(F("RF315 Pulse Lgth: %d" CR), valuePLSL);
      Log.notice(F("Bits nb: %d" CR), valueBITS);
#    ifdef ZradioCC1101 // set Receive off and Transmitt on
      float trMhz = RF315data["mhz"] | CC1101_FREQUENCY_315;
      if (validFrequency((int)trMhz)) {
        ELECHOUSE_cc1101.SetTx(trMhz);
        Log.notice(F("Transmit mhz: %F" CR), trMhz);
        mySwitch315.disableReceive();
        mySwitch315.enableTransmit(RF315_EMITTER_GPIO);
      }
#    endif
      mySwitch315.setRepeatTransmit(valueRPT);
      mySwitch315.setProtocol(valuePRT, valuePLSL);
      mySwitch315.send(data, valueBITS);
      Log.notice(F("MQTTtoRF315 OK" CR));
      pub(subjectGTWRF315toMQTT, RF315data); // we acknowledge the sending by publishing the value to an acknowledgement topic, for the moment even if it is a signal repetition we acknowledge also
      mySwitch315.setRepeatTransmit(RF315_EMITTER_REPEAT); // Restore the default value
    } else {
#    ifdef ZradioCC1101 // set Receive on and Transmitt off
      float tempMhz = RF315data["mhz"];
      if (tempMhz != 0 && validFrequency((int)tempMhz)) {
        receiveMhz = tempMhz;
        Log.notice(F("Receive mhz: %F" CR), receiveMhz);
        pub(subjectGTWRF315toMQTT, RF315data); // we acknowledge the sending by publishing the value to an acknowledgement topic, for the moment even if it is a signal repetition we acknowledge also
      } else {
        pub(subjectGTWRF315toMQTT, "{\"Status\": \"Error\"}"); // Fail feedback
        Log.error(F("MQTTtoRF315 Fail json" CR));
      }
#    else
#      ifndef ARDUINO_AVR_UNO // Space issues with the UNO
      pub(subjectGTWRF315toMQTT, "{\"Status\": \"error\"}"); // Fail feedback
#      endif
      Log.error(F("MQTTtoRF315 Fail json" CR));
#    endif
    }
  }
#    ifdef ZradioCC1101 // set Receive on and Transmitt off
  ELECHOUSE_cc1101.SetRx(receiveMhz);
  mySwitch315.disableTransmit();
  mySwitch315.enableReceive(RF315_RECEIVER_GPIO);
#    endif
}
#  endif
#endif

#ifdef ZradioCC1101
bool validFrequency(int mhz) {
  //  CC1101 valid frequencies 300-348 MHZ, 387-464MHZ and 779-928MHZ.
  if (mhz >= 300 && mhz <= 348)
    return true;
  if (mhz >= 387 && mhz <= 464)
    return true;
  if (mhz >= 779 && mhz <= 928)
    return true;
  return false;
}
#endif
