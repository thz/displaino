#ifndef __NHD_H__
#     define __NHD_H__

#include <Adafruit_Sensor.h>

#include <ESP8266WiFi.h>
#if DHT_ENABLED
#include <DHT.h>
#endif
#include <ESP8266WebServer.h>
#include <Time.h>
#include <TimeLib.h>
#include <math.h>
//#include <PubSubClient.h>
#include "ESP8266_Basic.h"
#include <FS.h>
#include "Font.h"
 

#if DHT_ENABLED
#define DHTTYPE DHT22
#define DHTPIN D3  //GPIO12  - Daten
#define DHTVDD D4  //GPIO13  - VDD
#endif

#define   SDI_PIN    13    // SDI (serial mode) signal connected to D4
#define   SCL_PIN    14    // SCL (serial mdoe) signal connected to D3
#define    RS_PIN    5    // RS (D/C) signal connected to          D2
#define   RES_PIN    0    // /RES signal connected to              D1
#define    CS_PIN   15    // /CS signal connected to               D5

#define    RED  0x0000FF
#define  GREEN  0x00FF00
#define   BLUE  0xFF0000
#define  WHITE  0xFFFFFF
#define  BLACK  0x000000
#define YELLOW  0x00FFEE

// Drivinig_Current
#define R10H 0x56         //Red
#define G11H 0x4D         //Green
#define B12H 0x46         //Blue

// #define LDC TRUE          // Helligkeitssteuerung über Fotowiederstand an A0
#ifdef LDC
  #define SENSOR //A0       // ADC Pin
#endif

#define BUFFPIXEL 20

#define DBG_OUTPUT_PORT Serial

#define VERSION "V 0.34" 

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address);


boolean isSchaltjahr( int year);



int ntp_daysofyear(int y);

unsigned char ntp_daysofmonth(int year,unsigned char m);


boolean summertime_EU(int year, byte month, byte day, byte hour, byte tzHours);
// European Daylight Savings Time calculation by "jurs" for German Arduino Forum
// input parameters: "normal time" for year, month, day, hour and tzHours (0=UTC, 1=MEZ)
// return value: returns true during Daylight Saving Time, false otherwise



void getNTPTime();


// Fuehrende '0' bei mmehrstelligen Zahlen ausgeben
String printDigits(int digits);

 
/*===============================*/
/*========== NTP read ===========*/
/*============ END ==============*/
/*===============================*/


/*===============================*/
/*========== DHT read ===========*/
/*=========== BEGIN =============*/
/*===============================*/

void getTemperature(); 

/*===============================*/
/*========== DHT read ===========*/
/*=========== BEGIN =============*/
/*===============================*/


/*********************************/
/****** LOW LEVEL FUNCTIONS ******/
/************* START *************/
/*********************************/

//wait without delay()
void milli_delay( unsigned long ms);

byte minValue( byte cR10h, byte cG11h, byte cB12h); 

byte maxValue( byte cR10h, byte cG11h, byte cB12h); 


//format bytes
String formatBytes(size_t bytes);

bool checkBound(float newValue, float prevValue, float maxDiff); 

void OLED_Command_160128RGB(unsigned char c);        // send command to OLED


void OLED_Data_160128RGB(unsigned char d);        // send data to OLED


void OLED_SerialPixelData_160128RGB(unsigned char d);    // serial write for pixel data


void OLED_SetColumnAddress_160128RGB(unsigned char x_start, unsigned char x_end);    // set column address start + end


void OLED_SetRowAddress_160128RGB(unsigned char y_start, unsigned char y_end);    // set row address start + end


void OLED_WriteMemoryStart_160128RGB(void);    // write to RAM command


void OLED_Pixel_160128RGB(unsigned long color);    // write one pixel of a given color


void OLED_SetPosition_160128RGB(unsigned char x_pos, unsigned char y_pos);    // set x,y address


void OLED_FillArea_160128RGB(unsigned char x_start, unsigned char x_end, unsigned char y_start, unsigned char y_end, unsigned long color);    // fill area with a given color



void OLED_FillScreen_160128RGB(unsigned long color);    // fill screen with a given color



void OLED_Driving_Current_160128RGB(byte r10h, byte g11h, byte b12h);      //OLED initialization


void OLED_FadeOut_160128RGB(unsigned int fadingTime);



void OLED_FadeIn_160128RGB(unsigned int fadingTime);



/*===============================*/
/*===== LOW LEVEL FUNCTIONS =====*/
/*============= END =============*/
/*===============================*/


/*********************************/
/***** HIGH LEVEL FUNCTIONS ******/
/************ START **************/
/*********************************/

unsigned char findLastRightBit(unsigned int letter);


int countPixel(const char array_of_string[]);


int countBigPixel(const char array_of_string[]);



void OLED_smallText_160128RGB(unsigned char x_pos, unsigned char y_pos, unsigned int letter, unsigned long textColor, unsigned long backgroundColor);  // function to show letter
 
 
void OLED_bigText_160128RGB(unsigned char x_pos, unsigned char y_pos, unsigned int letter, unsigned long textColor, unsigned long backgroundColor);  // function to show letter


/*
 * Returns 1 + the last x position which was painted onto.
 */
unsigned int OLED_StringSmallFont_160128RGB(
		unsigned char x_pos, unsigned char y_pos,
		const char array_of_string[],
		unsigned long textColor,
		unsigned long backgroundColor) ;


void OLED_StringBigFont_160128RGB(unsigned int x_pos, unsigned int y_pos, const char array_of_string[], unsigned long textColor, unsigned long backgroundColor);



/*===============================*/
/*==== HIGH LEVEL FUNCTIONS =====*/
/*============= END =============*/
/*===============================*/

/*********************************/
/***** WRITE BMP TO DISPLAY ******/
/************ START **************/
/*********************************/

uint16_t read16(File &f) ;

uint32_t read32(File &f) ;

void bmpDraw(const char *filename, uint8_t x, uint8_t y);

/*
Code Description
0   tornado
1   tropical storm
2   hurricane
3   severe thunderstorms
4   thunderstorms
5   mixed rain and snow
6   mixed rain and sleet
7   mixed snow and sleet
8   freezing drizzle
9   drizzle
10  freezing rain
11  showers
12  showers
13  snow flurries
14  light snow showers
15  blowing snow
16  snow
17  hail
18  sleet
19  dust
20  foggy
21  haze
22  smoky
23  blustery
24  windy
25  cold
26  cloudy
27  mostly cloudy (night)
28  mostly cloudy (day)
29  partly cloudy (night)
30  partly cloudy (day)
31  clear (night)
32  sunny
33  fair (night)
34  fair (day)
35  mixed rain and hail
36  hot
37  isolated thunderstorms
38  scattered thunderstorms
39  scattered thunderstorms
40  scattered showers
41  heavy snow
42  scattered snow showers
43  heavy snow
44  partly cloudy
45  thundershowers
46  snow showers
47  isolated thundershowers
*/

void selectWeatherIcon(int day);

/*===============================*/
/***** SELECT WEATHER ICON *******/
/************* END ***************/
/*===============================*/

/*********************************/
/*********** FSBrowser ***********/
/************ START **************/
/*********************************/


String getContentType(String filename);

bool handleFileRead(String path);

void handleFileUpload();

void handleFileDelete();


void handleFileCreate();


void handleFileList();


void initFSBrowserServer();

//OLED initialization
void OLED_Init_160128RGB(void);

void setup();


int mqttCallback(char* topic, byte* payload, unsigned int length);

void dirtyDisplay();

void masterRender(char heartbeat);

// the loop function runs over and over again forever
void loop();

#endif /* __NHD_H__ */
