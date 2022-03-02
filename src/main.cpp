#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"

// custom headers
#include <logo.h>

AsyncWebServer webServer(80);

// import lcd library
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C oled(U8G2_R0);

int g_lineHeight = 0;

const int ledPin = 13;
String ledState;

#define ssid "xxxxxxx"
#define password "xxxxxxxxxxxx"
#define wifi_timeout_ms 20000
#define hostname "dedsec"

// oled init
void oledInit()
{
  oled.begin();
  oled.clear();
  oled.setFont(u8g2_font_profont11_tr);
  g_lineHeight = oled.getFontAscent() - oled.getFontDescent();
}

// start wifi connection
void connectToWiFi()
{

  // oled text for connecting
  oled.setCursor(0, g_lineHeight);
  oled.print("./connecting");
  delay(500);
  oled.sendBuffer();

  // initialize wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.setAutoReconnect(true);

  // keep track of connection time attempt
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < wifi_timeout_ms)
  {
    oled.setCursor(0, g_lineHeight);
  }

  // oled text for connection initialization status
  if (WiFi.status() != WL_CONNECTED)
  {
    oled.clear();
    oled.setCursor(0, g_lineHeight);
    oled.print(" ./failed ");
    oled.sendBuffer();
  }
  else
  {
    oled.clear();
    oled.setCursor(0, g_lineHeight);
    oled.println("./connected");
    oled.sendBuffer();
    delay(1000);
    oled.clear();
  }
}

// draws dedsec logo
void drawLogo()
{
  oled.firstPage();
  do
  {
    oled.drawXBMP(0, 0, logo_width, logo_height, logo);
  } while (oled.nextPage());
}

// oled text with ip address and device name
void infoLoop()
{

  // text and ip address
  oled.setCursor(0, g_lineHeight);
  oled.print("./dedsec");
  oled.setCursor(0, g_lineHeight * 2);
  oled.print("ip~");
  oled.setCursor(20, g_lineHeight * 2);
  oled.print(WiFi.localIP());
  oled.sendBuffer();
  delay(2000);
  oled.clearBuffer();
  // draw xmb image ** dedsec **
  drawLogo();
  delay(2000);
  oled.sendBuffer();
  oled.clearBuffer();
}

// setup dns for http://dedsec.local
void setupDNS()
{

  if (!MDNS.begin("dedsec"))
  {
    Serial.println("Error starting mDNS");
    return;
  }

  webServer.begin();
}

void initSpiffs()
{
  // Initialize SPIFFS
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
}

void genSite()
{
  // Route for root / web page
  webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
               { request->send(SPIFFS, "/index.html", String(), false); });

  // Route to load style.css file
  webServer.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
               { request->send(SPIFFS, "/style.css", "text/css"); });

  // Route to set GPIO to HIGH
  webServer.on("/yes", HTTP_GET, [](AsyncWebServerRequest *request)
               {
      digitalWrite(ledPin, HIGH);
      request->send(SPIFFS, "/index.html", String(), false/*, processor*/); });

  // Route to set GPIO to LOW
  webServer.on("/no", HTTP_GET, [](AsyncWebServerRequest *request)
               {
      digitalWrite(ledPin, LOW);    
      request->send(SPIFFS, "/index.html", String(), false/*, processor*/); });

  // Start server
  webServer.begin();
}

void setup()
{
  // put your setup code here, to run once:

  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  oledInit();
  drawLogo();
  delay(5000);
  oled.clear();

  connectToWiFi();
  WiFi.setHostname(hostname);
  delay(1000);
  setupDNS();
  initSpiffs();
}

void loop()
{
  // put your main code here, to run repeatedly:

  infoLoop();
  genSite();
}