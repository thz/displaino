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
#include "ESP8266_Basic.h"

ESP8266_Basic::ESP8266_Basic() : webServer(), 
                                 mqtt_client(wifi_client)
								 {
  //Callbacks								 
  webServer.set_saveConfig_Callback(std::bind(&ESP8266_Basic::cfgChange_Callback, this));
  mqtt_client.setCallback(std::bind(&ESP8266_Basic::mqttBroker_Callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

//===============================================================================
//  AktSen Control 
//===============================================================================
//===> updateMeasurement <-----------------------------------------------------
void ESP8266_Basic::handle_Measurement(){
  if (mqtt_client.connected()){
    long now = millis();
    if (now - lastMeasure_time > updateMeasure_time) {
      lastMeasure_time = now;
      run_oneWire();
    }
  }
}
//===> run 1Wire <-----------------------------------------------------
void ESP8266_Basic::run_oneWire(){
  OneWire oneWire(2);                   //GPIO2
  DallasTemperature DS18B20(&oneWire);
  DS18B20.begin();

  DS18B20.requestTemperatures(); 
  for (int i = 0; i < DS18B20.getDeviceCount(); i++) {
    String str_temp = String(DS18B20.getTempCByIndex(i));
	char temp[7];
    strcpy(temp, str_temp.c_str());  
	pub(2,1,i, temp);
  }
}

//===============================================================================
//  MQTT Control 
//===============================================================================
//===> publish Topics <--------------------------------------------------------
bool ESP8266_Basic::pub(int e1, char* Payload){
  String strTopic = buildE1(e1);
  mqtt_client.publish(strTopic.c_str(), Payload); 
  mqtt_client.loop();
}
bool ESP8266_Basic::pub(int e1, int e2, char* Payload){
  String strTopic = buildE2(e1, e2);
  mqtt_client.publish(strTopic.c_str(), Payload); 
  mqtt_client.loop();
}
bool ESP8266_Basic::pub(int e1, int e2, int e3, char* Payload){
  String strTopic = buildE3(e1, e2, e3);
  mqtt_client.publish(strTopic.c_str(), Payload); 
  mqtt_client.loop();
}
/*
bool ESP8266_Basic::pub(int e1, int e2, int e3, int e4, char* Payload){
  String strTopic = buildE4(e1, e2, e3, e4);
  mqtt_client.publish(strTopic.c_str(), Payload); 
  mqtt_client.loop();
}
*/
//===> build Topics <----------------------------------------------------------
String ESP8266_Basic::buildE1(int e1){
  String strTopic = cfg.mqttDeviceName;
  strTopic += "/";
  strTopic += topic.pub.E1.item[e1];
  return strTopic;
}
String ESP8266_Basic::buildE2(int e1, int e2){
  String strTopic = buildE1(e1);
  strTopic += "/";
  strTopic += topic.pub.E2.item[e1][e2];
  return strTopic;
}
String ESP8266_Basic::buildE3(int e1, int e2, int e3){
  String strTopic = buildE2(e1, e2);
  strTopic += "/";
  strTopic += topic.pub.E3.item[e1][e2][e3];
  return strTopic;
}
/*
String ESP8266_Basic::buildE4(int e1, int e2, int e3, int 43){
  String strTopic = buildE1(e1);
  strTopic += "/";
  strTopic += buildE2(e1, e2);
  strTopic += "/";
  strTopic += buildE3(e1, e2, e3);
  strTopic += "/";
  strTopic += topic.pub.E4.item[e1][e2][e3][e4];
  return strTopic;
}
*/

//===> incomming subscribe <---------------------------------------------------
void ESP8266_Basic::mqttBroker_Callback(char* topic, byte* payload, unsigned int length) {

  char value[50] = "";

  for (int i = 0; i < length; i++) {
    value[i] = payload[i];
  }

  Serial.print("incoming subscribe: ");
  Serial.print(topic);
  Serial.print(" | ");
  Serial.println(value);
  
  TdissectResult dissectResult;    
  dissectResult = dissectPayload(topic, value);

  if (dissectResult.found){
    if (dissectResult.itemPath == "1/0/0"){
	  if (strcmp(value, "Reboot") == 0){
	    ESP.restart();
	  }
	}
    if (dissectResult.itemPath == "1/0/1"){
	  pubConfig();
	}
    if (dissectResult.itemPath == "1/0/2"){
	  //UpdateFirmware()
	  webServer.updateFirmware();
	}
	
    if (dissectResult.itemPath == "3/1/0"){
	  //Write Field_01
	  strcpy(myFile.Field_01, value);
	  write_MyFile();
	  updateMeasure_time = String(value).toInt();
	}
    if (dissectResult.itemPath == "3/0/0"){
	  //Read Field_01
	  strcpy(myFile.Field_01, "");
	  read_MyFile();
	  Serial.println(myFile.Field_01);
	}
	// OLED-Display-Daten
	char subOLEDTopic[6] = "2/0/";  //Search for recieved screen 2/0/0-2/0/9
	for (int i = 0; i < 10; i++ )
	{
	   subOLEDTopic[4] = i + '0';
  	   if (dissectResult.itemPath == subOLEDTopic )
	   {
	      //Store MQTT Payload to MyScreen struct
	      for (int j=0; j < 6 ; j++ ) // -99.9/0
		  {
		 	MyOLEDDisplay[i].Screen[j] = value[j];
		  }
		}
	 }

	// Weather Icon - Code from Yahoo
	char subCodeTopic[6] = "3/0/";  //Search for recieved screen 3/0/0-3/0/5
	for (int i = 0; i < 6; i++ )
	{
	   subCodeTopic[4] = i + '0';   
  	   if (dissectResult.itemPath == subCodeTopic )
	   {
	      //Store MQTT Payload to WeatherIcon struct
	      for (int j=0; j < 4 ; j++ )  // Two digits from Yahoo Weather-Code + /0
		  {
		 	MyWeatherIcon[i].weatherCode[j] = value[j];
		  }
		}
	 }
  
	// Weather Forecast Date -  from Yahoo
	char subDateTopic[6] = "3/1/";  //Search for recieved screen 3/1/0-3/1/5
	for (int i = 0; i < 6; i++ )
	{
	   subDateTopic[4] = i + '0';   
  	   if (dissectResult.itemPath == subDateTopic )
	   {
	      //Store MQTT Payload to WeatherIcon struct
	      for (int j=0; j < 17; j++ )  // May 15 characters from Yahoo Weather-Code + /0
		  {
		 	MyWeatherIcon[i].forecastDate[j] = value[j];
		  }
		}
	 }
  
	// Weather Forecast condition -  from Yahoo
	char subConditionTopic[6] = "3/2/";  //Search for recieved screen 3/1/0-3/1/5
	for (int i = 0; i < 6; i++ )
	{
	   subConditionTopic[4] = i + '0';   
  	   if (dissectResult.itemPath == subConditionTopic )
	   {
	      //Store MQTT Payload to WeatherIcon struct
	      for (int j=0; j < 32 ; j++ )  // Max 32 characters from Yahoo Weather-Code + /0
		  {
		 	MyWeatherIcon[i].forecastDate[j] = value[j];
		  }
		}
	 }
   
  }
}

//===> dissect incomming subscribe <-------------------------------------------
TdissectResult ESP8266_Basic::dissectPayload(String subTopic, String subValue){
  TdissectResult dissectResult;   
  String Topics[4];
  for (int i = 0; i < 4; i++) {
    Topics[i] = "";
  }
 
  String str = subTopic;
  if (str.startsWith("/")) str.remove(0, 1); 
  if (str.endsWith("/")) str.remove(str.length()-1,str.length()); 
  dissectResult.topic = str;
  dissectResult.value = subTopic;
   
  int index = -1;
  int i = 0;
  do{
   index = str.indexOf("/");
   if (index != -1){
      Topics[i] = str.substring(0, index);	  
	  str.remove(0, index +1); 
	  i++;
	  if (str.indexOf("/") == -1 and str.length() > 0){    //best of the rest
	    index = -1;
		Topics[i] = str;
		i++;
	  }
	}else{
	}
  } while (index != -1); 
  int depth = i;
  
  //find item index
  String itemPath = "";
  if (depth > 1 and Topics[1] != ""){
    for (int i = 0; i < topic.sub.E1.count; i++) {
	  if (topic.sub.E1.item[i] != NULL){
	    if (strcmp(topic.sub.E1.item[i], Topics[1].c_str()) == 0){
          dissectResult.E1 = i;
		  itemPath = String(i);
		  dissectResult.found = true;
		  break;
        }else dissectResult.found = false;
	  }
    }
  }	
  if (depth > 2 and Topics[2] != ""){
    for (int i = 0; i < topic.sub.E2.count; i++) {
	  if (topic.sub.E2.item[dissectResult.E1][i] != NULL){
	    if (strcmp(topic.sub.E2.item[dissectResult.E1][i], Topics[2].c_str()) == 0){
          dissectResult.E2 = i;
		  itemPath += "/";
		  itemPath += String(i);
		  dissectResult.found = true;
		  break;
        }else dissectResult.found = false;
	  }
    }
  }	
  if (depth > 3 and Topics[3] != ""){
    for (int i = 0; i < topic.sub.E3.count; i++) {
	  if (topic.sub.E3.item[dissectResult.E1][dissectResult.E2][i] != NULL){
	    if (strcmp(topic.sub.E3.item[dissectResult.E1][dissectResult.E2][i], Topics[3].c_str()) == 0){
          dissectResult.E3 = i;
		  itemPath += "/";
		  itemPath += String(i);
		  dissectResult.found = true;
		  break;
        }else dissectResult.found = false;
	  }
    }
  }	
  dissectResult.itemPath = itemPath;
  
  return dissectResult; 
}

//===> publish WiFi Configuration <-------------------------------------------------
void ESP8266_Basic::pubConfig(){
  pub(0,0,0, cfg.webUser);
  pub(0,0,1, cfg.webPassword);
  pub(0,1,0, cfg.apName);
  pub(0,1,1, cfg.apPassword);
  pub(0,2,0, cfg.wifiSSID);
  pub(0,2,1, cfg.wifiPSK);
  pub(0,2,2, cfg.wifiIP);
  pub(0,3,0, cfg.mqttServer);
  pub(0,3,1, cfg.mqttPort);
  pub(0,4,0, cfg.updateServer);
  pub(0,4,1, cfg.filePath);
  //pub(0,3,2, cfg.mqttDeviceName);  
}

//===============================================================================
//  Public WiFi-Manager-Control 
//===============================================================================

//===> Start WiFi Connection <-------------------------------------------------
void ESP8266_Basic::start_WiFi_connections(){
  checkFlash();
  config_running = true;
  
  read_cfg();
  read_MyFile();

  if (start_WiFi()){
    WiFi.mode(WIFI_STA);     //exit AP-Mode if set once
	config_running = false;
	startConfigServer();
  }else{
    startAccessPoint();
  }
  printCFG();
}

//===> handle connections WiFi, MQTT, WebServer <------------------------------
void ESP8266_Basic::handle_connections(){
  
  webServer.handleClient();
  
  if (config_running == false){
    if (WiFi.status() == WL_CONNECTED) {
 	  mqtt_client.loop();
	  
	  if (!mqtt_client.connected()){
	    Serial.println("### MQTT has disconnected...");
	    mqtt_client.disconnect();
		strcpy(cfg.mqttStatus, "disconnected");
		start_MQTT();
	  }
	  
    }else{
      Serial.println("### WIFI has disconnected...");
	  if (!start_WiFi()){
		start_WiFi_connections();
	  }else{
	    Serial.println("### WIFI reconnect successful...");
	  };

      //publish_value(publish_status_topic[1], "off");
      //restart_wifi();
	  //WIFImanager();
	
	}
  }
  delay(1);
}


//===============================================================================
//  private WiFi-Manager-Control 
//===============================================================================

//===> start ConfigServer <----------------------------------------------------
void ESP8266_Basic::startConfigServer(){  

  webServer.set_cfgPointer(&cfg);
 // webServer.set_ConfigRunningPointer(&config_running);
 
  struct MyScreen *pMyScreen = &MyOLEDDisplay[0];
  webServer.set_OLEDPointer(pMyScreen);

  webServer.start();
}

//===> start AP <--------------------------------------------------------------
void ESP8266_Basic::startAccessPoint(){
  Serial.println("AccessPoint start");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(cfg.apName, cfg.apPassword);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  startConfigServer();
}

//===> Set Function for save cfg Callback <------------------------------------
void ESP8266_Basic::cfgChange_Callback(){
  Serial.println("incomming Callback, config has changed!!");
  printCFG();
  write_cfgFile();
//  start_MQTT();
};

//===> WIFI SETUP <------------------------------------------------------------
bool ESP8266_Basic::start_WiFi(){
bool WiFiOK = false;

  delay(10);
  Serial.println();
  Serial.print("Connecting WiFi to: ");
  Serial.println(cfg.wifiSSID);
  
  WiFi.begin(cfg.wifiSSID, cfg.wifiPSK);

  int i = 0;
  while (WiFi.status() != WL_CONNECTED and i < 31) {
    delay(500);
    Serial.print(".");
	i++;
  }
  Serial.println("");
  
  if (WiFi.status() == WL_CONNECTED){
    WiFiOK = true; 
    WIFI_reconnect = true;
    Serial.println("");
    Serial.print("WiFi connected with IP:    ");Serial.println(WiFi.localIP());
	strcpy( cfg.wifiIP, IPtoString(WiFi.localIP()).c_str()  );
	strcpy( cfg.wifiSSID, WiFi.SSID().c_str() );
	strcpy( cfg.wifiPSK, WiFi.psk().c_str() );

	// UDP-Connection for NTP-Sync
	Serial.println("Starting UDP");
    udp.begin(localPort);
	Serial.print("Local port: ");
    Serial.println(udp.localPort());

 
	//IPAddress ip = WiFi.localIP();
    //TopicHeader = ip[3];
  }  
  return WiFiOK; 
}

//===> MQTT SETUP <---------------------------------------------------------
bool ESP8266_Basic::start_MQTT(){
bool MQTTOK = false;

  Serial.print("Connecting to MQTT-Broker: ");
  Serial.print(cfg.mqttServer);Serial.print(":");Serial.println(cfg.mqttPort);
  
  mqtt_client.setServer(charToIP(cfg.mqttServer), atoi(cfg.mqttPort)); 
 
  if (mqtt_client.connect(cfg.mqttDeviceName)) {
    Serial.println("MQTT connected");
	strcpy(cfg.mqttStatus, "connected");
	MQTTOK = true;	        
	  
    if (WIFI_reconnect == true){
      WIFI_reconnect = false;
	  pub(1,0,0, "on");
	  pub(1,0,0, "off");
    }
    pub(1,0,1, "on");
    pub(1,0,1, "off");
	
    //broker_subcribe();
	String sub = cfg.mqttDeviceName;
	sub += "/#";
	mqtt_client.subscribe(sub.c_str());
    mqtt_client.loop();
	sub = "/";
	sub += cfg.mqttDeviceName;
	sub += "/#";
	mqtt_client.subscribe(sub.c_str());
    mqtt_client.loop();
     
    // register second subscribe if exists 
	sub = cfg.mqttSecSub;
  	if (sub.length() > 0) 
	{
		sub += "/#";
		mqtt_client.subscribe(sub.c_str());
		mqtt_client.loop();
		Serial.print("MQTT subscribed: ");    Serial.println(sub.c_str());
		sub = "/";
		sub += cfg.mqttSecSub;
		sub += "/#";
		mqtt_client.subscribe(sub.c_str());
		mqtt_client.loop();
     	Serial.print("MQTT subscribed: ");    Serial.println(sub.c_str());
    }
  }
  return MQTTOK;
}

//===============================================================================
//  Configuration 
//===============================================================================

//===> WIFI Manager Config <---------------------------------------------------
void  ESP8266_Basic::read_cfg(){

  Serial.println("read config");
  if (read_cfgFile()){
    Serial.println("fsMount OK and File exist");
  
  }else{
    Serial.println("create new config");
    strcpy(cfg.webUser, "ESPuser");
    strcpy(cfg.webPassword, "ESPpass");

    String str = "ESP8266_";
    str += String(ESP.getChipId());
    strcpy(cfg.apName, str.c_str());
    strcpy(cfg.apPassword, "ESP8266config");
	
    //strcpy(cfg.wifiSSID, "");
    //strcpy(cfg.wifiPSK, "");
    strcpy(cfg.wifiIP, "");
	
	//strcpy( cfg.wifiIP, IPtoString(WiFi.localIP()).c_str()  );
	strcpy( cfg.wifiSSID, WiFi.SSID().c_str() );
	strcpy( cfg.wifiPSK, WiFi.psk().c_str() );
	
    strcpy(cfg.mqttServer, "");
    strcpy(cfg.mqttPort, "1883");
    strcpy(cfg.mqttDeviceName, cfg.apName);
	
	write_cfgFile();
  }
}

//===> WIFI Manager <----------------------------------------------------------
void ESP8266_Basic::write_cfg() {
 
  //IPAddress ip = WiFi.localIP();
  //TopicHeader = ip[3];
  //String MQTT_DeviceName = TopicHeader;
  //MQTT_DeviceName += "/AktSen";
  //strcpy(cfg.mqttDeviceName, MQTT_DeviceName.c_str());
}

//===> Reset Settings <--------------------------------------------------------
void ESP8266_Basic::resetSettings(){
  Serial.println("resetSettings");
  strcpy(cfg.webUser, "");
  strcpy(cfg.webPassword, "");
  strcpy(cfg.apName, "");
  strcpy(cfg.apPassword, "");
  strcpy(cfg.wifiSSID, "");
  strcpy(cfg.wifiPSK, "");
  strcpy(cfg.wifiIP, "");
  strcpy(cfg.mqttServer, "");
  strcpy(cfg.mqttPort, "1883");
  strcpy(cfg.mqttDeviceName, "");
  strcpy(cfg.mqttSecSub, "");
  strcpy(cfg.updateServer, "");
  strcpy(cfg.filePath, "");
  strcpy(cfg.webNameScreen0, "");
  strcpy(cfg.webUnitScreen0, "");
  strcpy(cfg.webDurationScreen0, "");
  strcpy(cfg.webNameScreen1, "");
  strcpy(cfg.webUnitScreen1, "");
  strcpy(cfg.webDurationScreen1, "");
  strcpy(cfg.webNameScreen2, "");
  strcpy(cfg.webUnitScreen2, "");
  strcpy(cfg.webDurationScreen2, "");
  strcpy(cfg.webNameScreen3, "");
  strcpy(cfg.webUnitScreen3, "");
  strcpy(cfg.webDurationScreen3, "");
  strcpy(cfg.webNameScreen4, "");
  strcpy(cfg.webUnitScreen4, "");
  strcpy(cfg.webDurationScreen4, "");
  strcpy(cfg.webNameScreen5, "");
  strcpy(cfg.webUnitScreen5, "");
  strcpy(cfg.webDurationScreen5, "");
  strcpy(cfg.webNameScreen6, "");
  strcpy(cfg.webUnitScreen6, "");
  strcpy(cfg.webDurationScreen6, "");
  strcpy(cfg.webNameScreen7, "");
  strcpy(cfg.webUnitScreen7, "");
  strcpy(cfg.webDurationScreen7, "");
  strcpy(cfg.webNameScreen8, "");
  strcpy(cfg.webUnitScreen8, "");
  strcpy(cfg.webDurationScreen8, "");
  strcpy(cfg.webNameScreen9, "");
  strcpy(cfg.webUnitScreen9, "");
  strcpy(cfg.webDurationScreen9, "");

  write_cfgFile();
}

//===============================================================================
//  FileSystem
//===============================================================================


//My File
//===> read from MyFile <-------------------------------------------------
bool ESP8266_Basic::read_MyFile(){
  bool readOK = false;
  File MyFile;
  //clean FS, for testing
  //SPIFFS.format();

  //read configuration from FS json
  Serial.println("");
  Serial.println("mounting FS...for MyFile");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/MyFile.json")) {
      //file exists, reading and loading
      Serial.println("reading Myfile");
      MyFile = SPIFFS.open("/MyFile.json", "r");
      if (MyFile) {
        Serial.println("opened MyFile");
        size_t size = MyFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        MyFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        //json.printTo(Serial);
        if (json.success()) {
          Serial.println("json success to MyFile");
          
		  //Get Data from File
          strcpy(myFile.Field_01, json["Field_01"]);
          strcpy(myFile.Field_02, json["Field_02"]);
		  
	      updateMeasure_time = String(myFile.Field_01).toInt();
          Serial.print("Update Time = ");
          Serial.println(updateMeasure_time);


		  readOK = true;

        } else {
          Serial.println("failed to load json MyFile");
        }
      }
    }else{
	  Serial.println("MyFile does not exist");
	}
  } else {
    Serial.println("failed to mount FS");
  }
  MyFile.close();
  //end read
  return readOK;

};

//===> write to MyFile <--------------------------------------------------
void ESP8266_Basic::write_MyFile(){

  SPIFFS.begin();
  //save the custom parameters to FS
  Serial.println("saving MyFile");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  
  //Serial.println("bevor write_cfgFile ");
  //json.printTo(Serial);

  json["Field_01"] = myFile.Field_01;
  json["Field_02"] = myFile.Field_02;

  File MyFile = SPIFFS.open("/MyFile.json", "w");
  if (!MyFile) {
    Serial.println("failed to open MyFile for writing");
    Serial.print("format file System.. ");
	SPIFFS.format();
	Serial.println("done");
	//write_cfgFile();
 }

  json.printTo(Serial);
  json.printTo(MyFile);
  MyFile.close();
  //end save

}

//===> read Config from File <-------------------------------------------------
bool ESP8266_Basic::read_cfgFile(){
  bool readOK = false;
  //clean FS, for testing
  //SPIFFS.format();

  //read configuration from FS json
  Serial.println("");
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
  	  Serial.println("reading config file");

	  // Remove config file 
	  //SPIFFS.remove("/config.json");
      //return readOK;   

	  File cfgFile = SPIFFS.open("/config.json", "r");
      if (cfgFile) {
        Serial.println("opened config file");
        size_t size = cfgFile.size();
		
		
		// Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        cfgFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        //json.printTo(Serial);
        if (json.success()) {
          Serial.println("json success");
          
		  //Get Data from File
          strcpy(cfg.configSize, json["configSize"]);

		  // file exist but new size => remove and return
		  if (( atoi(cfg.configSize) - sizeof(CFG)) != 0)
		  {
              Serial.print("Size of old struct CFG: ");Serial.println(cfg.configSize);
              Serial.print("Size of new struct CFG: ");Serial.println(sizeof(CFG));
              Serial.println("New size -> remove config file");
			  cfgFile.close();
  		  	  SPIFFS.remove("/config.json");
              return readOK;   // false
	      }
          strcpy(cfg.webUser, json["webUser"]);
          strcpy(cfg.webPassword, json["webPassword"]);
          strcpy(cfg.apName, json["apName"]);
          strcpy(cfg.apPassword, json["apPassword"]);
          strcpy(cfg.wifiSSID, json["wifiSSID"]);
          strcpy(cfg.wifiPSK, json["wifiPSK"]);
          strcpy(cfg.wifiIP, json["wifiIP"]);
          strcpy(cfg.mqttServer, json["mqttServer"]);
          strcpy(cfg.mqttPort, json["mqttPort"]);
          strcpy(cfg.mqttDeviceName, json["mqttDeviceName"]);
          strcpy(cfg.mqttSecSub, json["mqttSecSub"]);
          strcpy(cfg.updateServer, json["updateServer"]);
          strcpy(cfg.filePath, json["filePath"]);
          strcpy(cfg.webNameScreen0, json["webNameScreen0"]);
          strcpy(cfg.webUnitScreen0, json["webUnitScreen0"]);
          strcpy(cfg.webDurationScreen0, json["webDurationScreen0"]);
          strcpy(cfg.webNameScreen1, json["webNameScreen1"]);
          strcpy(cfg.webUnitScreen1, json["webUnitScreen1"]);
          strcpy(cfg.webDurationScreen1, json["webDurationScreen1"]);
          strcpy(cfg.webNameScreen2, json["webNameScreen2"]);
          strcpy(cfg.webUnitScreen2, json["webUnitScreen2"]);
          strcpy(cfg.webDurationScreen2, json["webDurationScreen2"]);
          strcpy(cfg.webNameScreen3, json["webNameScreen3"]);
          strcpy(cfg.webUnitScreen3, json["webUnitScreen3"]);
          strcpy(cfg.webDurationScreen3, json["webDurationScreen3"]);
          strcpy(cfg.webNameScreen4, json["webNameScreen4"]);
          strcpy(cfg.webUnitScreen4, json["webUnitScreen4"]);
          strcpy(cfg.webDurationScreen4, json["webDurationScreen4"]);
          strcpy(cfg.webNameScreen5, json["webNameScreen5"]);
          strcpy(cfg.webUnitScreen5, json["webUnitScreen5"]);
          strcpy(cfg.webDurationScreen5, json["webDurationScreen5"]);
          strcpy(cfg.webNameScreen6, json["webNameScreen6"]);
          strcpy(cfg.webUnitScreen6, json["webUnitScreen6"]);
          strcpy(cfg.webDurationScreen6, json["webDurationScreen6"]);
          strcpy(cfg.webNameScreen7, json["webNameScreen7"]);
          strcpy(cfg.webUnitScreen7, json["webUnitScreen7"]);
          strcpy(cfg.webDurationScreen7, json["webDurationScreen7"]);
          strcpy(cfg.webNameScreen8, json["webNameScreen8"]);
          strcpy(cfg.webUnitScreen8, json["webUnitScreen8"]);
          strcpy(cfg.webDurationScreen8, json["webDurationScreen8"]);
          strcpy(cfg.webNameScreen9, json["webNameScreen9"]);
          strcpy(cfg.webUnitScreen9, json["webUnitScreen9"]);
          strcpy(cfg.webDurationScreen9, json["webDurationScreen9"]);


		  readOK = true;

        } else {
          Serial.println("failed to load json config");
        }
      }
    }else{
	  Serial.println("file does not exist");
	}
  } else {
    Serial.println("failed to mount FS");
  }
  Serial.println("cfgFile close...");

  cfgFile.close();
  Serial.println("cfgFile closed.");
  //end read
  return readOK;

};

//===> write Config to File <--------------------------------------------------
void ESP8266_Basic::write_cfgFile(){

  SPIFFS.begin();
  //save the custom parameters to FS
  Serial.println("saving config");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  
//  Serial.println("bevor write_cfgFile ");
//  json.printTo(Serial);

  sprintf(cfg.configSize, "%4d", sizeof(CFG));

  json["configSize"] = cfg.configSize;
  json["webUser"] = cfg.webUser;
  json["webPassword"] = cfg.webPassword;
  json["apName"] = cfg.apName;
  json["apPassword"] = cfg.apPassword;
  json["wifiSSID"] = cfg.wifiSSID;
  json["wifiPSK"] = cfg.wifiPSK;
  json["wifiIP"] = cfg.wifiIP;
  json["mqttServer"] = cfg.mqttServer;
  json["mqttPort"] = cfg.mqttPort;
  json["mqttDeviceName"] = cfg.mqttDeviceName;
  json["mqttSecSub"] = cfg.mqttSecSub;
  json["updateServer"] = cfg.updateServer;
  json["filePath"] = cfg.filePath;
  json["webNameScreen0"] = cfg.webNameScreen0;
  json["webUnitScreen0"] = cfg.webUnitScreen0;
  json["webDurationScreen0"] = cfg.webDurationScreen0;
  json["webNameScreen1"] = cfg.webNameScreen1;
  json["webUnitScreen1"] = cfg.webUnitScreen1;
  json["webDurationScreen1"] = cfg.webDurationScreen1;
  json["webNameScreen2"] = cfg.webNameScreen2;
  json["webUnitScreen2"] = cfg.webUnitScreen2;
  json["webDurationScreen2"] = cfg.webDurationScreen2;
  json["webNameScreen3"] = cfg.webNameScreen3;
  json["webUnitScreen3"] = cfg.webUnitScreen3;
  json["webDurationScreen3"] = cfg.webDurationScreen3;
  json["webNameScreen4"] = cfg.webNameScreen4;
  json["webUnitScreen4"] = cfg.webUnitScreen4;
  json["webDurationScreen4"] = cfg.webDurationScreen4;
  json["webNameScreen5"] = cfg.webNameScreen5;
  json["webUnitScreen5"] = cfg.webUnitScreen5;
  json["webDurationScreen5"] = cfg.webDurationScreen5;
  json["webNameScreen6"] = cfg.webNameScreen6;
  json["webUnitScreen6"] = cfg.webUnitScreen6;
  json["webDurationScreen6"] = cfg.webDurationScreen6;
  json["webNameScreen7"] = cfg.webNameScreen7;
  json["webUnitScreen7"] = cfg.webUnitScreen7;
  json["webDurationScreen7"] = cfg.webDurationScreen7;
  json["webNameScreen8"] = cfg.webNameScreen8;
  json["webUnitScreen8"] = cfg.webUnitScreen8;
  json["webDurationScreen8"] = cfg.webDurationScreen8;
  json["webNameScreen9"] = cfg.webNameScreen9;
  json["webUnitScreen9"] = cfg.webUnitScreen9;
  json["webDurationScreen9"] = cfg.webDurationScreen9;




  File cfgFile = SPIFFS.open("/config.json", "w");
  if (!cfgFile) {
    Serial.println("failed to open config file for writing");
    Serial.print("format file System.. ");
	SPIFFS.format();
	Serial.println("done");
	//write_cfgFile();
  }

//  Serial.println("after write_cfgFile ");
//  json.printTo(Serial);
  json.printTo(cfgFile);
  cfgFile.close();
  //end save

}

//===============================================================================
//  helpers
//===============================================================================

//===> IPToString  <-----------------------------------------------------------
String ESP8266_Basic::IPtoString(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

IPAddress ESP8266_Basic::charToIP(char* IP) {
  IPAddress MyIP;

  String str = String(IP);
  for (int i = 0; i < 4; i++){
    String x = str.substring(0, str.indexOf("."));
    MyIP[i] = x.toInt();
    str.remove(0, str.indexOf(".")+1); 
  }
  return MyIP;
}

//===> Print Config  <---------------------------------------------------------
void ESP8266_Basic::printCFG(){

  Serial.println("");
  Serial.println("");
  Serial.println("Config:");
  Serial.println("########################################");
  Serial.print("WEBcfg Username:      "); Serial.println(cfg.webUser);
  Serial.print("WEBcfg Password:      "); Serial.println(cfg.webPassword);
  Serial.println("----------------------------------------");
  Serial.print("AP SSID:              "); Serial.println(cfg.apName);
  Serial.print("AP Password:          "); Serial.println(cfg.apPassword);
  Serial.println("----------------------------------------");
  Serial.print("WiFi SSID:            "); Serial.println(cfg.wifiSSID);
  Serial.print("WiFi Password:        "); Serial.println(cfg.wifiPSK);
  Serial.print("DHCP IP:              "); Serial.println(cfg.wifiIP);
  Serial.println("----------------------------------------");
  Serial.print("MQTT-Server IP:       "); Serial.println(cfg.mqttServer);
  Serial.print("MQTT-Server Port:     "); Serial.println(cfg.mqttPort);
  Serial.print("MQTT-DeviceName:      "); Serial.println(cfg.mqttDeviceName);
  Serial.print("MQTT-Second Subscribe:"); Serial.println(cfg.mqttSecSub);
  Serial.println("----------------------------------------");
  Serial.print("Update-Server IP:     "); Serial.println(cfg.updateServer);
  Serial.print("FilePath:             "); Serial.println(cfg.filePath);
  Serial.println("########################################");

};

//===> Check Flash Memory <-----------------------------------------------------
void ESP8266_Basic::checkFlash(){
  uint32_t realSize = ESP.getFlashChipRealSize();
  uint32_t ideSize = ESP.getFlashChipSize();
  FlashMode_t ideMode = ESP.getFlashChipMode();

  Serial.println("");
  Serial.println("============================================");
    Serial.printf("Flash real id:   %08X", ESP.getFlashChipId());
    Serial.println("");
    Serial.printf("Flash real size: %u", realSize);
    Serial.println("");

    Serial.printf("Flash ide  size: %u", ideSize);
    Serial.println("");
    Serial.printf("Flash ide speed: %u", ESP.getFlashChipSpeed());
    Serial.println("");
    Serial.printf("Flash ide mode:  %s", (ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));
    Serial.println("");
    
    if(ideSize != realSize) {
        Serial.println("Flash Chip configuration wrong!");
    } else {
        Serial.println("Flash Chip configuration ok.");
    }
  Serial.println("============================================");
  Serial.println("");
    
}






