/*********************************************************************
// DISPLAY SERVER NODE

Copyright 2018 Michael Egger

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the 
"Software"), to deal in the Software without restriction, including 
without limitation the rights to use, copy, modify, merge, publish, 
distribute, sublicense, and/or sell copies of the Software, and to 
permit persons to whom the Software is furnished to do so, subject to 
the following conditions:

The above copyright notice and this permission notice shall be 
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS 
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN 
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN 
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
SOFTWARE.

Notes:
 THIS IS A WORK IN PROGRESS

Summary:
 The goal of this program was to have a display with an open Wifi
 interface.
 I am using the OLED display from AZdelivery with the SSD1306 driver.
 My arduino is a counterfeit D1 with a modified ESP8266 chip.
 The GPIOs might be different with your board please check the 
 comments  below (NodemCu, D_1).

Wiring:
 No breadboard required. Simply connect the following pins:
  display      arduino
   SDA -------- D14/SDA (D_1)
   SCL -------- D15/SCL (D_1)
   VCC -------- 3.3V (D_1)
   GND -------- GND (D_1)

Usage:
 Connect a Sensor Client Node via wifi to this Display Server Node.
 Whenever you want the display to display some data, you can do so
 by a page request. 

Example:
 opening the following URL when connected to the AP:
  http://192.168.4.1/?key1=value1&key2=2.2
 will display the following text on the display:
  key1: value1
  key2: 2.2
 It is possible to include a fontsize parameter by adding it as an argument
  http://192.168.4.1/?key1=value1&key2=2.2&fontsize=2
 will display the text with double the size (2) or triple size (3).

// DISPLAY SERVER NODE
*********************************************************************/


// STRING CONVERSION
#include <string.h>

// WEB SERVER
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>

//DISPLAY
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>  //edit .h according to display version
#define OLED_RESET LED_BUILTIN   //for NodemCu: D4=2 // for D_1: LED_BUILTIN

Adafruit_SSD1306 display(OLED_RESET);
const char *ssid = "D1_DisplayServer";
const char *password = "d1d1d1d1d1d1d1d1";
String message = "";
ESP8266WebServer server(80);


void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  Serial.begin(115200);

  // TEST DISPLAY
  Serial.println("Testing Display...\n");
  display.clearDisplay();
  display.display();
  delay(50);
  displayTest("Testing...\n123456789.123456789:123456789|12...\n.\n.\n.\n.\n.");
  Serial.println("Display Test Complete!\n");

  // SET UP WIFI
  Serial.print("Configuring access point...");
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
  displayTest("Ready");
  delay(300);
}


void loop() {
  server.handleClient();
}


/// this will handle each request and display the message 
/// on the display and reply with wgat was contained in the args
void handleRoot() {
  int fontzise = 1;
  
  Serial.println("Received request:");
  message = "";
  for (int i = 0; i < server.args(); i++) {
    if(
      server.argName(i).equals("fontsize")
      && (int)server.arg(i).toInt() < 4 
      && (int)server.arg(i).toInt() > 0 ){
        fontzise = server.arg(i).toInt();
    } else{
      message += server.argName(i) + ": ";
      message += server.arg(i) + "\n";
    }
  } 
  Serial.println(message);
  displayAndClear(message, fontzise);
  Serial.println("Display complete.");
  server.send(200, "text/html", "\nACK:\n" + message + "\n:ENDACK");
  Serial.println("Ack sent.");
}


/// This function is used to display text without
/// the need for clearDisplay (wich causes flickering)
void displayAndClear(String string, int textsize){
  Serial.println("Displaying: " + string);
  // writes text
  display.setCursor(0,0);
  display.setTextSize(textsize);
  display.setTextColor(WHITE);
  display.print(string);
  // refresh display
  display.display();
  // clear buffer without redraw
  display.setCursor(0,0);
  display.setTextColor(BLACK);
  display.print(string);
}


/// Just writes one character at a time from message. 
/// Three runs, increases fontsize from 1 to 2
void displayTest(String text){
  int ml = text.length();
  for(int ts = 1; ts < 3; ts++){
    for (int i = 0; i < ml; i++){
      displayAndClear(text.substring(0,i+1),ts);
      delay(10);
    }
  }
}
