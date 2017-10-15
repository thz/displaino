/******************************************************************************

  ProjectName: ESP8266_Basic                      ***** *****
  SubTitle   : ESP8266 Template                  *     *     ************
                                                *   **   **   *           *
  Copyright by Pf@nne                          *   *   *   *   *   ****    *
                                               *   *       *   *   *   *   *
  Last modification by:                        *   *       *   *   ****    *
  - Pf@nne (pf@nne-mail.de)                     *   *     *****           *
                                                 *   *        *   *******
  Date    : 29.03.2016                            *****      *   *
  Version : alpha 0.200                                     *   *
  Revison :                                                *****

********************************************************************************/
/*
ToDo
FS Abfrage ob Feld vorhanden, sonst ggf. format
Wenn cfg und WiFi.SSID() gleich dann ohne Vorgabe starten, soll schneller gehen 
WEB config with password
Erledigt: Reset nach nicht ge�nderter Config wird nicht durchgef�hrt
WEB-Status Tabelle rechts
TopicTree 
Erledigt: OTA WEB
OTA Arduino
Erledigt: OTA onDemand
*/  
  

#pragma once

//FileSystem
  #include <FS.h>                   	//this needs to be first, or it all crashes and burns...
  #include <ArduinoJson.h>          	//https://github.com/bblanchon/ArduinoJson
 
//ESP8266
  #include <ESP8266WiFi.h>			    //https://github.com/esp8266/Arduino	
  #include <EEPROM.h>

  #include <ESP8266WebServer.h>
  #include <WiFiClient.h> 
  #include <WiFiUdp.h>
  #include <DNSServer.h>
  
//MQTT Client
  #include <PubSubClient.h>
  
//ESP8266_Basic
  #include "ESP8266_Basic_webServer.h"
  #include "ESP8266_Basic_data.h"
  
//Sensoren
  //1Wire direct
  #include "OneWire.h"
  #include <DallasTemperature.h>

class ESP8266_Basic{

public:
  ESP8266_Basic();
  WiFiUDP udp;  
  Topics topic;
  MyScreen MyOLEDDisplay[10];
  WeatherIcon MyWeatherIcon[5]; // Max. six days
  CFG cfg;
  void start_WiFi_connections();
  void handle_connections();
  void cfgChange_Callback();
  void mqttBroker_Callback(char* topic, byte* payload, unsigned int length);

  void mqttSubscribe(const char *topic);

  bool pub(int e1, char* Payload);
  bool pub(int e1, int e2, char* Payload);
  bool pub(int e1, int e2, int e3, char* Payload);
  unsigned int localPort = 2390;                // local port to listen for UDP packets
  bool config_running;

  //AktSen
  void handle_Measurement();
  
private:
  WiFiClient wifi_client;
  PubSubClient mqtt_client;
  File cfgFile;
  ESP8266_Basic_webServer webServer;  

  
  //AktSen
  long lastMeasure_time;
  long updateMeasure_time = 2000;
  void run_oneWire();
 
  //WiFi-Manager-Control---------------
  void startConfigServer();
  void startAccessPoint();
  
  bool start_WiFi();  
  bool start_MQTT();
  bool WIFI_reconnect;
  
  //Config-Control---------------------
  void read_cfg();
  bool read_cfgFile();
  void write_cfg();
  void write_cfgFile();
  void resetSettings();  

  
  //MyData-Control---------------------
  bool read_MyFile();
  void write_MyFile();
  MyFile myFile;
  
  //MQTT-Control-----------------------
  String buildE1(int e1);
  String buildE2(int e1, int e2);
  String buildE3(int e1, int e2, int e3);
  //String buildE4(int e1, int e2, int e3, int e4);
  //bool pub(int e1, int e2, int e3, int e4, char* Payload);
  TdissectResult dissectPayload(String subTopic, String subValue);
  
  void pubConfig();
  
  //helpers----------------------------
  void checkFlash();
  String IPtoString(IPAddress ip);
  IPAddress charToIP(char* IP);
  void printCFG();    
};