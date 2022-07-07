#include <Arduino.h>
#include <ESP8266WiFi.h> 
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include <WiFiClientSecureBearSSL.h>
//#include <WiFiClientSecure.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
//#include <SPI.h>
#include <ESP8266HTTPClient.h> 
#include <NTPClient.h>
// Config laden
#include <config.h>
// BTC Logo laden
#include <btclogo.h>


/////////////////////////////////////////////////////////////////////////// TFT
#define TFT_CS         4
#define TFT_RST        16                                            
#define TFT_DC         5
// Color definitions
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF
#define BITCOIN 0xFD20

//#define COLOR3 ST7735_YELLOW
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

/////////////////////////////////////////////////////////////////////////// Intervall der Steuerung
unsigned long previousMillis_btckurs = 0;
unsigned long interval_btckurs = 25000; 

unsigned long previousMillis_zeit = 0;
unsigned long interval_zeit = 3500; 


/////////////////////////////////////////////////////////////////////////// ntp
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
      
String BTC_old_kurs ="0";      

std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

 float coin;


/////////////////////////////////////////////////////////////////////////// Funktionsprototypen
//void callback                (char*, byte*, unsigned int);
void loop                      ();
void tft_text                  (int x, int y, int size, char *text, uint16_t color);
void btc_kurs                  ();
void ntp_zeit                  ();

void setup() {

 //OTA Setup für Firmware
  ArduinoOTA.setHostname("24KanalRelaisWohnzimmer");
  ArduinoOTA.setPassword("7n6WkRpZtxtkykyMUx329");
  ArduinoOTA.begin();

  // Serielle Kommunikation starten
  Serial.begin(115200);


  /////////////////////////////////////////////////////////////////////////// TFT initialisieren
  tft.initR(INITR_BLACKTAB);      // Init ST7735S chip, black tab
  
  // TFT einfärben
  tft.fillScreen(BLACK); 


  // Verbindung zum WiFI aufbauen

  Serial.print("Verbindung zu SSID -> ");
  Serial.println(ssid);

  // WiFi 
  IPAddress ip(192, 168, 1, 100);
	IPAddress dns(192, 168, 1, 2);  
	IPAddress subnet(255, 255, 0, 0);
	IPAddress gateway(192, 168, 1, 2);

	WiFi.config(ip, dns, gateway, subnet);
  
  WiFi.begin(ssid, password);

  // Wifi AP deaktivieren
  WiFi.mode(WIFI_STA);

  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
   
   tft.setCursor(5,40);
   tft.setTextColor(RED,BLACK);
   tft.setTextSize(2);
   tft.print("Suche Wlan");
  
  }
  Serial.println("");
  Serial.println("Erfolgreich mit dem WiFi verbunden!");
  Serial.println(WiFi.localIP());
  Serial.print("SSID : ");
  Serial.println(ssid);

  tft.fillScreen(BLACK); 

  /////////////////////////////////////////////////////////////////////////// Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(7200);


  // Set starting text size to 1 for connecting message
  //tft_text(15,15,2,"BTC START",BITCOIN);
  delay(200);
   tft.drawBitmap(0, 0, bitcoinLogo, 128, 64, BITCOIN);
   tft.setCursor(13,80);
   tft.setTextColor(RED,BLACK);
   tft.setTextSize(1);
   tft.print("Lade BTC Kurs");
   delay(1000);
   tft.fillRect(1, 80, 128, 19, BLACK);

   tft.setCursor(20,151);
   tft.setTextColor(YELLOW,BLACK);
   tft.setTextSize(1);
   tft.print("by zurzy.shop");

}

/////////////////////////////////////////////////////////////////////////// TFT - Text schreiben
void tft_text(int x, int y, int size, char *text, uint16_t color) {
  tft.setTextWrap(true);
  tft.setCursor(x, y);
  tft.setTextColor(color);
  tft.setTextSize(size);
  tft.print(text);
}

/////////////////////////////////////////////////////////////////////////// BTC Kurs 
void btc_kurs(){

  
  client->setInsecure();
  HTTPClient https;

  //if (https.begin(*client, "https://min-api.cryptocompare.com/data/pricemulti?fsyms=XRP&tsyms=ZAR&e=Luno&extraParams=your_app_name")) {  // HTTPS
  if (https.begin(*client, "https://min-api.cryptocompare.com/data/pricemulti?fsyms=BTC&tsyms=USD")) {  // HTTPS
    int httpCode = https.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
      Serial.println(httpCode);
      // file found at server?
      //if (httpCode == HTTP_CODE_OK) {
      if (httpCode == 200) {
        String payload = https.getString();

        const size_t capacity = 2 * JSON_OBJECT_SIZE(1) + 10;
        DynamicJsonDocument doc(capacity);

        String json = payload;

        //const char* json = "{\"XRP\":{\"ZAR\":4.20}}";

        deserializeJson(doc, json);

        String BTC_USD = doc["BTC"]["USD"]; 
        
        Serial.println("BTC Wert ausgeben");
        
      if (BTC_old_kurs < BTC_USD) {

      tft.drawBitmap(0, 0, bitcoinLogo, 128, 64, GREEN);
      tft.setCursor(24,78);
      tft.setTextColor(GREEN,BLACK);
      tft.setTextSize(2);
      tft.print("$"+BTC_USD.substring(0, 5));

      }
      else
      {
      tft.drawBitmap(0, 0, bitcoinLogo, 128, 64, RED);
      tft.setCursor(24,78);
      tft.setTextColor(RED,BLACK);
      tft.setTextSize(2);
      tft.print("$"+BTC_USD.substring(0, 5));
      }

      BTC_old_kurs = BTC_USD;

      }
    } else {

    }

  } else {

  }

}

/////////////////////////////////////////////////////////////////////////// NTP Zeit
void ntp_zeit(){

timeClient.update();

  //int currentHour = timeClient.getHours();
  //int currentMinute = timeClient.getMinutes();
  //String aktuelleZeit = String(currentHour) + ":" + String(currentMinute);


  String formattedTime = timeClient.getFormattedTime();
  //Serial.print("Formatted Time: ");
  //Serial.println(formattedTime);

      Serial.println("Zeit schreiben");
      //tft.fillRect(1, 102, 128, 20, BLACK);
      //tft.setTextWrap(true);
      tft.setCursor(30,105);
      tft.setTextColor(WHITE,BLACK);
      tft.setTextSize(2);
      tft.print(formattedTime.substring(0, 5));


// Datum ausgeben
time_t epochTime = timeClient.getEpochTime();
struct tm *ptm = gmtime ((time_t *)&epochTime); 

int currentYear = ptm->tm_year+1900;
int currentMonth = ptm->tm_mon+1;
int monthDay = ptm->tm_mday;
String datum_aktuell = String(monthDay) + "." + String(currentMonth) + "." + String(currentYear);

      //tft.fillRect(1, 128, 128, 10, BLACK);
      //tft.setTextWrap(true);
      tft.setCursor(37,130);
      tft.setTextColor(CYAN,BLACK);
      tft.setTextSize(1);

      tft.print(datum_aktuell);  


}


/////////////////////////////////////////////////////////////////////////// VOID LOOP
void loop() {

  

  // OTA Handle starten
  ArduinoOTA.handle();  

  ///////////////////////////////////////////////////////////////////////// BTC Kurs abfragen
  if (millis() - previousMillis_btckurs > interval_btckurs) {
      previousMillis_btckurs = millis();   // aktuelle Zeit abspeichern
      // BTC Kurs abfragen
      btc_kurs();
    }

    ///////////////////////////////////////////////////////////////////////// Zeit
  if (millis() - previousMillis_zeit > interval_zeit) {
      previousMillis_zeit = millis();   // aktuelle Zeit abspeichern
      // BTC Kurs abfragen
      ntp_zeit();
    }  


}

