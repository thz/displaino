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
#include "ESP8266_Basic_webServer.h"

ESP8266_Basic_webServer::ESP8266_Basic_webServer() : webServer(80){ 

  httpUpdater.setup(&webServer); 
 
  webServer.on("/", std::bind(&ESP8266_Basic_webServer::rootPageHandler, this));
  webServer.on("/sensor", std::bind(&ESP8266_Basic_webServer::sensorPageHandler, this));
  webServer.on("/screens", std::bind(&ESP8266_Basic_webServer::screenConfigHandler, this));
 
  webServer.on("/wlan_config", std::bind(&ESP8266_Basic_webServer::wlanPageHandler, this));
  webServer.on("/gpio", std::bind(&ESP8266_Basic_webServer::gpioPageHandler, this));
  //webServer.on("/cfg", std::bind(&ESP8266_Basic_webServer::cfgPageHandler, this));
/*  //list directory
  webServer.on("/list", HTTP_GET, std::bind(&ESP8266_Basic_webServer::handleFileList, this));
  //load editor
//  webServer.on("/edit", HTTP_GET, [](){ if(!std::bind(&ESP8266_Basic_webServer::handleFileRead("/edit.htm"), this)) webServer.send(404, "text/plain", "FileNotFound"); });
//  String filePath = "/edit.htm";
//  webServer.on("/edit", HTTP_GET, [&](){ if(!std::bind(&ESP8266_Basic_webServer::handleFileRead(filePath), this)) webServer.send(404, "text/plain", "FileNotFound"); });
  
  //create file
  webServer.on("/edit", HTTP_PUT, std::bind(&ESP8266_Basic_webServer::handleFileCreate, this));
  //delete file
  webServer.on("/edit", HTTP_DELETE, std::bind(&ESP8266_Basic_webServer::handleFileDelete, this));
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  webServer.on("/edit", HTTP_POST, [](){ std::bind(&ESP8266_Basic_webServer::webServer.send(200, "text/plain", ""), this }, std::bind(&ESP8266_Basic_webServer::handleFileUpload, this));
  //called when the url is not defined here
  //use it to load content from SPIFFS
//  webServer.onNotFound([](){ if(!std::bind(&ESP8266_Basic_webServer::handleFileRead(webServer.uri()), this)) webServer.send(404, "text/plain", "FileNotFound"); });
  //called when the url is not defined here
  //use it to load content from SPIFFS
//  webServer.onNotFound([](){ if(!std::bind(&ESP8266_Basic_webServer::handleFileRead(webServer.uri()), this)) webServer.send(404, "text/plain", "FileNotFound");  });
  //get heap status, analog input value and all GPIO statuses in one json call
//  webServer.on("/all", HTTP_GET, [](){
//    String json = "{";
//    json += "\"heap\":"+String(ESP.getFreeHeap());
//    json += ", \"analog\":"+String(analogRead(A0));
//    json += ", \"gpio\":"+String((uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16)));
//    json += "}";
//    webServer.send(200, "text/json", json);
//    json = String();
//  });
*/
  webServer.onNotFound(std::bind(&ESP8266_Basic_webServer::handleNotFound, this));
}

//===============================================================================
//  WEB-Server control
//===============================================================================
void ESP8266_Basic_webServer::start(){
  Serial.println("Start WEB-Server");
  
  pinMode(GPIO2, OUTPUT);
  webServer.begin(); 
  Serial.println("HTTP server started");

}
//===> handle WEB-Server <-----------------------------------------------------
void ESP8266_Basic_webServer::handleClient(){
   webServer.handleClient();
}
//===> CFGstruct pointer <-----------------------------------------------------
void ESP8266_Basic_webServer::set_cfgPointer(CFG *p){
  cfg = p;
}

//===> OLEDDisplaystruct pointer <---------------------------------------------
void ESP8266_Basic_webServer::set_OLEDPointer(MyScreen *p){
  oled = p;
}

//void ESP8266_Basic_webServer::set_ConfigRunningPointer(bool *p){
//  fConfigRunning = p;
//}


//===> Callback for CFGchange <------------------------------------------------
void ESP8266_Basic_webServer::set_saveConfig_Callback(CallbackFunction c){
  saveConfig_Callback = c;
}
//===> Update Firmware <-------------------------------------------------------
void ESP8266_Basic_webServer::updateFirmware(){
  Serial.println("Updating Firmware.......");
  t_httpUpdate_return ret = ESPhttpUpdate.update(cfg->updateServer, 80, cfg->filePath);
  switch(ret) {
    case HTTP_UPDATE_FAILED:
        Serial.println("[update] Update failed.");
        break;
    case HTTP_UPDATE_NO_UPDATES:
        Serial.println("[update] Update no Update.");
        break;
    case HTTP_UPDATE_OK:
        Serial.println("[update] Update ok."); // may not called we reboot the ESP
        break;
  }
}

//===============================================================================
//  HTML handling
//===============================================================================
/*
//====> File reading <-----------------------------------------------------------
bool ESP8266_Basic_webServer::handleFileRead(String path){
  DBG_OUTPUT_PORT.println("handleFileRead: " + path);
  if(path.endsWith("/")) path += "index.htm";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    if(SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = webServer.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

//====> File uploading <-----------------------------------------------------------
void ESP8266_Basic_webServer::handleFileUpload(){
  if(webServer.uri() != "/edit") return;
  HTTPUpload& upload = webServer.upload();
  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    DBG_OUTPUT_PORT.print("handleFileUpload Name: "); DBG_OUTPUT_PORT.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    //DBG_OUTPUT_PORT.print("handleFileUpload Data: "); DBG_OUTPUT_PORT.println(upload.currentSize);
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile)
      fsUploadFile.close();
    DBG_OUTPUT_PORT.print("handleFileUpload Size: "); DBG_OUTPUT_PORT.println(upload.totalSize);
  }
}

//====> File delete <-----------------------------------------------------------
void ESP8266_Basic_webServer::handleFileDelete(){
  if(webServer.args() == 0) return webServer.send(500, "text/plain", "BAD ARGS");
  String path = webServer.arg(0);
  DBG_OUTPUT_PORT.println("handleFileDelete: " + path);
  if(path == "/")
    return webServer.send(500, "text/plain", "BAD PATH");
  if(!SPIFFS.exists(path))
    return webServer.send(404, "text/plain", "FileNotFound");
  SPIFFS.remove(path);
  webServer.send(200, "text/plain", "");
  path = String();
}

//====> File create <-----------------------------------------------------------
void ESP8266_Basic_webServer::handleFileCreate(){
  if(webServer.args() == 0)
    return webServer.send(500, "text/plain", "BAD ARGS");
  String path = webServer.arg(0);
  DBG_OUTPUT_PORT.println("handleFileCreate: " + path);
  if(path == "/")
    return webServer.send(500, "text/plain", "BAD PATH");
  if(SPIFFS.exists(path))
    return webServer.send(500, "text/plain", "FILE EXISTS");
  File file = SPIFFS.open(path, "w");
  if(file)
    file.close();
  else
    return webServer.send(500, "text/plain", "CREATE FAILED");
  webServer.send(200, "text/plain", "");
  path = String();
}


//====> File listing <-----------------------------------------------------------
void ESP8266_Basic_webServer::handleFileList() {
  if(!webServer.hasArg("dir")) {webServer.send(500, "text/plain", "BAD ARGS"); return;}
  
  String path = webServer.arg("dir");
  DBG_OUTPUT_PORT.println("handleFileList: " + path);
  Dir dir = SPIFFS.openDir(path);
  path = String();

  String output = "[";
  while(dir.next()){
    File entry = dir.openFile("r");
    if (output != "[") output += ',';
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir)?"dir":"file";
    output += "\",\"name\":\"";
    output += String(entry.name()).substring(1);
    output += "\"}";
    entry.close();
  }
  
  output += "]";
  webServer.send(200, "text/json", output);
}

*/

//===============================================================================
//  HTML handling
//===============================================================================

//===> Sensor Page <-------------------------------------------------------
void ESP8266_Basic_webServer::sensorPageHandler(){
 String rm = ""
  
  "<!doctype html> <html>"
  "<head> <meta charset='utf-8'>"
  "<title>ESP8266 Configuration</title>"
  "</head>"

  "<body><body bgcolor='#F0F0F0'><font face='VERDANA,ARIAL,HELVETICA'> <form> <font face='VERDANA,ARIAL,HELVETICA'>"
  "<b><h1>ESP8266 Configuration</h1></b>";

  rm += ""
  "<font size='-2'>&copy; 2016   |   " + String(cfg->version) + "</font>"
  "</body bgcolor> </body></font>"
  "</html>"  
  ;

  webServer.send(200, "text/html", rm);


}

//#############################################################################
void ESP8266_Basic_webServer::rootPageHandler()
{
   // Disable Screen-Rotate
//   Serial.print("Config mode activ! (rootPageHandler()) -> ");Serial.println(fConfigRunning);

//   fConfigRunning = true;


  // Check if there are any GET parameters
  if (webServer.hasArg("webUser")) strcpy(cfg->webUser, webServer.arg("webUser").c_str());
  if (webServer.hasArg("webPassword")) strcpy(cfg->webPassword, webServer.arg("webPassword").c_str());
  if (webServer.hasArg("apName")) strcpy(cfg->apName, webServer.arg("apName").c_str());
  if (webServer.hasArg("apPassword")) strcpy(cfg->apPassword, webServer.arg("apPassword").c_str());
  if (webServer.hasArg("wifiSSID")) strcpy(cfg->wifiSSID, webServer.arg("wifiSSID").c_str());
  if (webServer.hasArg("wifiPSK")) strcpy(cfg->wifiPSK, webServer.arg("wifiPSK").c_str());
  if (webServer.hasArg("wifiIP")) strcpy(cfg->wifiIP, webServer.arg("wifiIP").c_str());
  if (webServer.hasArg("mqttServer")) strcpy(cfg->mqttServer, webServer.arg("mqttServer").c_str());
  if (webServer.hasArg("mqttPort")) strcpy(cfg->mqttPort, webServer.arg("mqttPort").c_str());
  if (webServer.hasArg("mqttDeviceName")) strcpy(cfg->mqttDeviceName, webServer.arg("mqttDeviceName").c_str());
  if (webServer.hasArg("mqttSecSub")) strcpy(cfg->mqttSecSub, webServer.arg("mqttSecSub").c_str());
  if (webServer.hasArg("updateServer")) strcpy(cfg->updateServer, webServer.arg("updateServer").c_str());
  if (webServer.hasArg("filePath")) strcpy(cfg->filePath, webServer.arg("filePath").c_str());
  

  String rm = ""
  
  "<!doctype html> <html>"
  "<head> <meta charset='utf-8'>"
  "<title>ESP8266 Configuration</title>"
  "</head>"

  "<body><body bgcolor='#F0F0F0'><font face='VERDANA,ARIAL,HELVETICA'> <form> <font face='VERDANA,ARIAL,HELVETICA'>"
  "<b><h1>ESP8266 Configuration</h1></b>";

  if (WiFi.status() == WL_CONNECTED){
    rm += "ESP8266 connected to: "; rm += WiFi.SSID(); rm += "<br>";
    rm += "DHCP IP: "; rm += String(IPtoString(WiFi.localIP())); rm += "<p></p>";
  }else{
    rm += "ESP8266 ist not connected!"; rm += "<p></p>";
  }	
  
  String str = String(cfg->mqttStatus);
  if (str == "connected"){
    rm += "ESP8266 connected to MQTT-Broker: "; rm += cfg->mqttServer; rm += "<p></p>";
  }

  rm += ""
  "<table width='30%' border='1' cellpadding='0' cellspacing='2'>"
  " <tr>"
  "  <td><b><font size='+1'>WEB Server</font></b></td>"
  "  <td></td>"
  " </tr>"
  " <tr>"
  "  <td>Username</td>"
  "  <td><input type='text' id='webUser' name='webUser' value='" + String(cfg->webUser) + "' size='30' maxlength='40' placeholder='Username'></td>"
  " </tr>"
  " <tr>"
  " <tr>"
  "  <td>Password</td>"
  "  <td><input type='text' id='webPassword' name='webPassword' value='" + String(cfg->webPassword) + "' size='30' maxlength='40' placeholder='Password'></td>"
  " </tr>"
  " <tr>"

  " <tr>"
  "  <td><b><font size='+1'>Accesspoint</font></b></td>"
  "  <td></td>"
  " </tr>"
  " <tr>"
  "  <td>SSID</td>"
  "  <td><input type='text' id='apName' name='apName' value='" + String(cfg->apName) + "' size='30' maxlength='40' placeholder='SSID'></td>"
  " </tr>"
  " <tr>"
  " <tr>"
  "  <td>Password</td>"
  "  <td><input type='text' id='apPassword' name='apPassword' value='" + String(cfg->apPassword) + "' size='30' maxlength='40' placeholder='Password'></td>"
  " </tr>"
  " <tr>"

  " <tr>"
  "  <td><b><font size='+1'>WiFi</font></b></td>"
  "  <td></td>"
  " </tr>"
  "  <td>SSID</td>"
  "  <td><input type='text' id='wifiSSID' name='wifiSSID' value='" + String(cfg->wifiSSID) + "' size='30' maxlength='40' placeholder='SSID'></td>"
  " </tr>"
  " <tr>"
  " <tr>"
  "  <td>Password</td>"
  "  <td><input type='password' id='wifiPSK' name='wifiPSK' size='30' maxlength='40' placeholder='Password'></td>"
  " </tr>"
  " <tr>"

  " <tr>"
  "  <td><b><font size='+1'>MQTT</font></b></td>"
  "  <td></td>"
  " </tr>"
  " <tr>"
  "  <td>Broker IP</td>"
  "  <td><input type='text' id='mqttServer' name='mqttServer' value='" + String(cfg->mqttServer) + "' size='30' maxlength='40' placeholder='IP Address'></td>"
  " </tr>"
  " <tr>"
  " <tr>"
  "  <td>Port</td>"
  "  <td><input type='text' id='mqttPort' name='mqttPort' value='" + String(cfg->mqttPort) + "' size='30' maxlength='40' placeholder='Port'></td>"
  " </tr>"
  " <tr>"
  "  <td>Devicename (first sub)</td>"
  "  <td><input type='text' id='mqttDeviceName' name='mqttDeviceName' value='" + String(cfg->mqttDeviceName) + "' size='20' maxlength='40' placeholder='Devicename'></td>"
  " </tr>"
  " <tr>"
  "  <td>second subscribe</td>"
  "  <td><input type='text' id='mqttSecSub' name='mqttSecSub' value='" + String(cfg->mqttSecSub) + "' size='30' maxlength='40' placeholder='second subscribe (alle/wetter/)'></td>"
  " </tr>"

  " <tr>"
  "  <td><b><font size='+1'>UpdateServer</font></b></td>"
  "  <td></td>"
  " </tr>"
  " <tr>"
  "  <td>Server IP</td>"
  "  <td><input type='text' id='updateServer' name='updateServer' value='" + String(cfg->updateServer) + "' size='30' maxlength='40' placeholder='IP Address'></td>"
  " </tr>"
  " <tr>"
  " <tr>"
  "  <td>FilePath</td>"
  "  <td><input type='text' id='filePath' name='filePath' value='" + String(cfg->filePath) + "' size='30' maxlength='40' placeholder='Path'></td>"
  " </tr>"

  " <tr>"
  "  <td><p></p> </td>"
  "  <td>  </td>"
  " </tr>"
  " <tr>"
  "  <td></td>"
  "  <td><input type='button' onclick=\"location.href='./screens'\"  value='Screens konfigurieren' style='height:30px; width:200px' >  </font></form> </td>"
  " </tr>"


  " <tr>"
  "  <td></td>"
  "  <td></td>"
  " </tr>"
  " <tr>"
  "  <td></td>"
  "  <td><input type='submit' value='Configuration sichern' style='height:30px; width:200px' > </font></form> </td>"
  " </tr>"

 " <tr>"
 "  <td></td>"
 "  <td></td>"
 " </tr>"
 " <tr>"
 "  <td></td>"
 "  <td><input type='submit' value='Update Firmware' id='update' name='update' value='' style='height:30px; width:200px' ></td>"
 " </tr>"

  " <tr>"
  "  <td></td>"
  "  <td></td>"
  " </tr>"
  " <tr>"
  "  <td></td>"
  "  <td><input type='submit' value='RESET' id='reset' name='reset' value='' style='height:30px; width:200px' >  </font></form> </td>"
  " </tr>"

  " <tr>"
  "  <td></td>"
  "  <td></td>"
  " </tr>"
  " <tr>"
  "  <td></td>"
  "  <td><input type='button' onclick=\"location.href='./update'\"  value='Flash Firmware' style='height:30px; width:200px' >  </font></form> </td>"
  " </tr>"

  "</table>"

  "<font size='-2'>&copy; 2016   |   " + String(cfg->version) + "</font>"
  "</body bgcolor> </body></font>"
  "</html>"  
  ;

  webServer.send(200, "text/html", rm);
 
  if (saveConfig_Callback != nullptr)
    saveConfig_Callback();
  else
     Serial.println("null");
	 
  if (webServer.hasArg("reset")) ESP.restart();
  if (webServer.hasArg("update")) updateFirmware();
}

//#############################################################################

void ESP8266_Basic_webServer::wlanPageHandler()
{
  // Check if there are any GET parameters
  if (webServer.hasArg("ssid"))
  {    
    if (webServer.hasArg("password"))
    {
      WiFi.begin(webServer.arg("ssid").c_str(), webServer.arg("password").c_str());
    }
    else
    {
      WiFi.begin(webServer.arg("ssid").c_str());
    }
    
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
    }
      
    Serial.println("");
    Serial.println("WiFi connected");  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    delay(1000);
  }
  
  String response_message = "";
  response_message += "<html>";
  response_message += "<head><title>ESP8266 Webserver</title></head>";
  response_message += "<body style=\"background-color:PaleGoldenRod\"><h1><center>WLAN Settings</center></h1>";
  
  if (WiFi.status() == WL_CONNECTED)
  {
    response_message += "Status: Connected<br>";
  }
  else
  {
    response_message += "Status: Disconnected<br>";
  }
  
  response_message += "<p>To connect to a WiFi network, please select a network...</p>";

  // Get number of visible access points
  int ap_count = WiFi.scanNetworks();
  
  if (ap_count == 0)
  {
    response_message += "No access points found.<br>";
  }
  else
  {
    response_message += "<form method=\"get\">";

    // Show access points
    for (uint8_t ap_idx = 0; ap_idx < ap_count; ap_idx++)
    {
      response_message += "<input type=\"radio\" name=\"ssid\" value=\"" + String(WiFi.SSID(ap_idx)) + "\">";
      response_message += String(WiFi.SSID(ap_idx)) + " (RSSI: " + WiFi.RSSI(ap_idx) +")";
      (WiFi.encryptionType(ap_idx) == ENC_TYPE_NONE) ? response_message += " " : response_message += "*";
      response_message += "<br><br>";
    }
    
    response_message += "WiFi password (if required):<br>";
    response_message += "<input type=\"text\" name=\"password\"><br>";
    response_message += "<input type=\"submit\" value=\"Connect\">";
    response_message += "</form>";
  }

  response_message += "</body></html>";
  
  webServer.send(200, "text/html", response_message);
}


void ESP8266_Basic_webServer::gpioPageHandler()
{
  // Check if there are any GET parameters
  if (webServer.hasArg("gpio2"))
  { 
    if (webServer.arg("gpio2") == "1")
    {
      digitalWrite(GPIO2, HIGH);
    }
    else
    {
      digitalWrite(GPIO2, LOW);
    }
  }

  String response_message = "<html><head><title>ESP8266 Webserver</title></head>";
  response_message += "<body style=\"background-color:PaleGoldenRod\"><h1><center>Control GPIO pins</center></h1>";
  response_message += "<form method=\"get\">";

  response_message += "GPIO2:<br>";

  if (digitalRead(GPIO2) == LOW)
  {
    response_message += "<input type=\"radio\" name=\"gpio2\" value=\"1\" onclick=\"submit();\">On<br>";
    response_message += "<input type=\"radio\" name=\"gpio2\" value=\"0\" onclick=\"submit();\" checked>Off<br>";
  }
  else
  {
    response_message += "<input type=\"radio\" name=\"gpio2\" value=\"1\" onclick=\"submit();\" checked>On<br>";
    response_message += "<input type=\"radio\" name=\"gpio2\" value=\"0\" onclick=\"submit();\">Off<br>";
  }

  response_message += "</form></body></html>";

  
  webServer.send(200, "text/html", response_message);
}


void ESP8266_Basic_webServer::screenConfigHandler()
{
   // Disable Screen-Rotate
   //Serial.println("Config mode activ! (screenConfigHandler()) -> ");Serial.println(fConfigRunning);
   //fConfigRunning = true;

  // Check if there are any GET parameters
  if (webServer.hasArg("webNameScreen0")) strcpy(cfg->webNameScreen0, webServer.arg("webNameScreen0").c_str());
  if (webServer.hasArg("webUnitScreen0")) strcpy(cfg->webUnitScreen0, webServer.arg("webUnitScreen0").c_str());
  if (webServer.hasArg("webDurationScreen0")) strcpy(cfg->webDurationScreen0, webServer.arg("webDurationScreen0").c_str());
  if (webServer.hasArg("webNameScreen1")) strcpy(cfg->webNameScreen1, webServer.arg("webNameScreen1").c_str());
  if (webServer.hasArg("webUnitScreen1")) strcpy(cfg->webUnitScreen1, webServer.arg("webUnitScreen1").c_str());
  if (webServer.hasArg("webDurationScreen1")) strcpy(cfg->webDurationScreen1, webServer.arg("webDurationScreen1").c_str());
  if (webServer.hasArg("webNameScreen2")) strcpy(cfg->webNameScreen2, webServer.arg("webNameScreen2").c_str());
  if (webServer.hasArg("webUnitScreen2")) strcpy(cfg->webUnitScreen2, webServer.arg("webUnitScreen2").c_str());
  if (webServer.hasArg("webDurationScreen2")) strcpy(cfg->webDurationScreen2, webServer.arg("webDurationScreen2").c_str());
  if (webServer.hasArg("webNameScreen3")) strcpy(cfg->webNameScreen3, webServer.arg("webNameScreen3").c_str());
  if (webServer.hasArg("webUnitScreen3")) strcpy(cfg->webUnitScreen3, webServer.arg("webUnitScreen3").c_str());
  if (webServer.hasArg("webDurationScreen3")) strcpy(cfg->webDurationScreen3, webServer.arg("webDurationScreen3").c_str());
  if (webServer.hasArg("webNameScreen4")) strcpy(cfg->webNameScreen4, webServer.arg("webNameScreen4").c_str());
  if (webServer.hasArg("webUnitScreen4")) strcpy(cfg->webUnitScreen4, webServer.arg("webUnitScreen4").c_str());
  if (webServer.hasArg("webDurationScreen4")) strcpy(cfg->webDurationScreen4, webServer.arg("webDurationScreen4").c_str());
  if (webServer.hasArg("webNameScreen5")) strcpy(cfg->webNameScreen5, webServer.arg("webNameScreen5").c_str());
  if (webServer.hasArg("webUnitScreen5")) strcpy(cfg->webUnitScreen5, webServer.arg("webUnitScreen5").c_str());
  if (webServer.hasArg("webDurationScreen5")) strcpy(cfg->webDurationScreen5, webServer.arg("webDurationScreen5").c_str());
  if (webServer.hasArg("webNameScreen6")) strcpy(cfg->webNameScreen6, webServer.arg("webNameScreen6").c_str());
  if (webServer.hasArg("webUnitScreen6")) strcpy(cfg->webUnitScreen6, webServer.arg("webUnitScreen6").c_str());
  if (webServer.hasArg("webDurationScreen6")) strcpy(cfg->webDurationScreen6, webServer.arg("webDurationScreen6").c_str());
  if (webServer.hasArg("webNameScreen7")) strcpy(cfg->webNameScreen7, webServer.arg("webNameScreen7").c_str());
  if (webServer.hasArg("webUnitScreen7")) strcpy(cfg->webUnitScreen7, webServer.arg("webUnitScreen7").c_str());
  if (webServer.hasArg("webDurationScreen7")) strcpy(cfg->webDurationScreen7, webServer.arg("webDurationScreen7").c_str());
  if (webServer.hasArg("webNameScreen8")) strcpy(cfg->webNameScreen8, webServer.arg("webNameScreen8").c_str());
  if (webServer.hasArg("webUnitScreen8")) strcpy(cfg->webUnitScreen8, webServer.arg("webUnitScreen8").c_str());
  if (webServer.hasArg("webDurationScreen8")) strcpy(cfg->webDurationScreen8, webServer.arg("webDurationScreen8").c_str());
  if (webServer.hasArg("webNameScreen9")) strcpy(cfg->webNameScreen9, webServer.arg("webNameScreen9").c_str());
  if (webServer.hasArg("webUnitScreen9")) strcpy(cfg->webUnitScreen9, webServer.arg("webUnitScreen9").c_str());
  if (webServer.hasArg("webDurationScreen9")) strcpy(cfg->webDurationScreen9, webServer.arg("webDurationScreen9").c_str());

  String rm = ""

"<!doctype html> <html>"
  "<head> <meta charset='utf-8'>"
  "<title>ESP8266 Configuration</title>"
  "</head>"

  "<body><body bgcolor='#F0F0F0'><font face='VERDANA,ARIAL,HELVETICA'> <form> <font face='VERDANA,ARIAL,HELVETICA'>"
  "<b><h1>ESP8266 Configuration</h1></b>";

  if (WiFi.status() == WL_CONNECTED){
    rm += "ESP8266 connected to: "; rm += WiFi.SSID(); rm += "<br>";
    rm += "DHCP IP: "; rm += String(IPtoString(WiFi.localIP())); rm += "<p></p>";
  }else{
    rm += "ESP8266 ist not connected!"; rm += "<p></p>";
  }	
  
  String str = String(cfg->mqttStatus);
  if (str == "connected"){
    rm += "ESP8266 connected to MQTT-Broker: "; rm += cfg->mqttServer; rm += "<p></p>";
  }
//  height: 732px;
  rm += ""
  
  " <table style='width: 440px;' border='1' cellpadding='0' cellspacing='2'>"
  " <tbody> "
  "		<tr> "
  "			<td><b><font size='+1'>Screens</font></b></td>"
  "			<td style='width: 192px;'>Name</td>"
  "			<td>aktueller Wert</td>"
  "			<td>Einheit</td>"
  "			<td>Dauer</td>"
  "		</tr>"

  "		<tr>"
  "			<td>Temp. in</td>"
  "			<td style='width: 192px;'>Temperatur</td>"
  "			<td>" + String(oled[4].Screen) + "</td>"
  "			<td>*C</td>"
  "			<td><input id='webDurationScreen4' name='webDurationScreen4' value='" + String(cfg->webDurationScreen4) + "' size='2' maxlength='2' placeholder='2' type='text'></td>"
  "		</tr>"

  "		<tr>"
  "			<td>Humi. in</td>"
  "			<td style='width: 192px;'>Luftfeuchte</td>"
  "			<td>" + String(oled[5].Screen) + "</td>"
  "			<td>%</td>"
  "			<td><input id='webDurationScreen5' name='webDurationScreen5' value='" + String(cfg->webDurationScreen5) + "' size='2' maxlength='2' placeholder='2' type='text'></td>"
  "		</tr>"

  "		<tr>"
  "			<td>Screen 0</td>"
  "			<td style='width: 192px;'><input id='webNameScreen0' name='webNameScreen0' value='" + String(cfg->webNameScreen0) + "' size='16' maxlength='16' placeholder='Name Screen 0' type='text'></td>"
  "			<td>" + String(oled[0].Screen) + "</td>"
  "			<td><input id='webUnitScreen0' name='webUnitScreen0' value='" + String(cfg->webUnitScreen0) + "' size='2' maxlength='2' placeholder='*C' type='text'></td>"
  "			<td><input id='webDurationScreen0' name='webDurationScreen0' value='" + String(cfg->webDurationScreen0) + "' size='2' maxlength='2' placeholder='2' type='text'></td>"
  "		</tr>"

  "		<tr>"
  "			<td>Screen 1</td>"
  "			<td style='width: 192px;'><input id='webNameScreen1' name='webNameScreen1' value='" + String(cfg->webNameScreen1) + "' size='16' maxlength='16' placeholder='Name Screen 1' type='text'></td>"
  "			<td>" + String(oled[1].Screen) + "</td>"
  "			<td><input id='webUnitScreen1' name='webUnitScreen1' value='" + String(cfg->webUnitScreen1) + "' size='2' maxlength='2' placeholder='*C' type='text'></td>"
  "			<td><input id='webDurationScreen1' name='webDurationScreen1' value='" + String(cfg->webDurationScreen1) + "' size='2' maxlength='2' placeholder='2' type='text'></td>"
  "		</tr>"

  "		<tr>"
  "			<td>Screen 2</td>"
  "			<td style='width: 192px;'><input id='webNameScreen2' name='webNameScreen2' value='" + String(cfg->webNameScreen2) + "' size='16' maxlength='16' placeholder='Name Screen 2' type='text'></td>"
  "			<td>" + String(oled[2].Screen) + "</td>"
  "			<td><input id='webUnitScreen2' name='webUnitScreen2' value='" + String(cfg->webUnitScreen2) + "' size='2' maxlength='2' placeholder='*C' type='text'></td>"
  "			<td><input id='webDurationScreen2' name='webDurationScreen2' value='" + String(cfg->webDurationScreen2) + "' size='2' maxlength='2' placeholder='2' type='text'></td>"
  "		</tr>"

  "		<tr>"
  "			<td>Screen 3</td>"
  "			<td style='width: 192px;'><input id='webNameScreen3' name='webNameScreen3' value='" + String(cfg->webNameScreen3) + "' size='16' maxlength='16' placeholder='Name Screen 3' type='text'></td>"
  "			<td>" + String(oled[3].Screen) + "</td>"
  "			<td><input id='webUnitScreen3' name='webUnitScreen3' value='" + String(cfg->webUnitScreen3) + "' size='2' maxlength='2' placeholder='*C' type='text'></td>"
  "			<td><input id='webDurationScreen3' name='webDurationScreen3' value='" + String(cfg->webDurationScreen3) + "' size='2' maxlength='2' placeholder='2' type='text'></td>"
  "		</tr>"

  "		<tr>"
  "			<td></td>"
  "			<td style='width: 192px;'><input value='Configuration sichern' style='height: 30px; width: 200px;' type='submit'> </td>"
  "			<td></td>"  
  "			<td></td>"
  "			<td></td>"
  "		</tr>"

  "		<tr>"
  "			<td></td>"
  "			<td style='width: 192px;'><input onclick=\"location.href='./'\" value='Homepage' style='height: 30px; width: 200px;' type='button'> </td>"
  "			<td></td>"  
  "			<td></td>"
  "			<td></td>"
  "		</tr>"
  "</tbody>"
  "</table>"
  "</form>"
  "<font size='-2'>&copy; 2016   |   " + String(cfg->version) + "</font>"
  "</body>"
  "</html>"
  ;
 
  if (saveConfig_Callback != nullptr)
    saveConfig_Callback();
  else
     Serial.println("null");

  webServer.send(200, "text/html", rm);
}


void ESP8266_Basic_webServer::handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += webServer.uri();
  message += "\nMethod: ";
  message += (webServer.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += webServer.args();
  message += "\n";
  
  for (uint8_t i = 0; i < webServer.args(); i++)
  {
    message += " " + webServer.argName(i) + ": " + webServer.arg(i) + "\n";
  }
  
  webServer.send(404, "text/plain", message);
}

//===============================================================================
//  helpers
//===============================================================================

//===> IPToString  <-----------------------------------------------------------
String ESP8266_Basic_webServer::IPtoString(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}/*

//format bytes
String ESP8266_Basic_webServer::formatBytes(size_t bytes){
  if (bytes < 1024){
    return String(bytes)+"B";
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0)+"KB";
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0)+"MB";
  } else {
    return String(bytes/1024.0/1024.0/1024.0)+"GB";
  }
}

String ESP8266_Basic_webServer::getContentType(String filename){
  if(webServer.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}
*/
