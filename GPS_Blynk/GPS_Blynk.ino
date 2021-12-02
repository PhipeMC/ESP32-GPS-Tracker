#include <Blynk.h>
#include <TinyGPSPlus.h>
#include "HardwareSerial.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include "ThingSpeak.h"

HardwareSerial gps_serial(2);
#define BLYNK_PRINT Serial
#define BLYNK_HEARTBEAT 30
#define TINY_GSM_MODEM_SIM800
#include <BlynkSimpleEsp32.h>
/*
   This sample sketch demonstrates the normal use of a TinyGPS++ (TinyGPSPlus) object.
   It requires the use of SoftwareSerial, and assumes that you have a
   4800-baud serial GPS device hooked up on pins 4(rx) and 3(tx).
*/
static const int RXPin = 16, TXPin = 17;
static const uint32_t GPSBaud = 9600;

//Auth Token for Blynk App
const char auth[]= "Ei3cxCrmjBib5edDq0CpXjkWo4CwHtiz";

//Thingspeak API
unsigned long myChannelNumber = 1583556;
const char * myWriteAPIKey = "RJCAMCMN52Z3MHCV";

// The TinyGPS++ object
TinyGPSPlus gps;

WidgetMap myMap(V0);
BlynkTimer timer;
WiFiClient  client;

float latitude;
float longitude;
float speed;
float satellites;
String direction;
unsigned int move_index = 1;

char ssid[] = "Tenda_B01FF0";
char pass[] = "3945010640";

BLYNK_WRITE(V0) {
    GpsParam gps(param);
  
    // Print 6 decimal places for Lat, Lon
    Serial.print("Lat: ");
    Serial.println(gps.getLat(), 7);
  
    Serial.print("Lon: ");
    Serial.println(gps.getLon(), 7);
  
    // Print 2 decimal places for Alt, Speed
    Serial.print("Altitute: ");
    Serial.println(gps.getAltitude(), 2);
  
    Serial.print("Speed: ");
    Serial.println(gps.getSpeed(), 2);
  
    Serial.println();
}


void setup()
{
    Serial.begin(115200);
    delay(10);
    gps_serial.begin(9600, SERIAL_8N1, RXPin, TXPin);
    
    Blynk.begin(auth, ssid, pass);
    timer.setInterval(5000L, checkGPS);
    Blynk.notify("Servicio GPS Tracker Iniciado");
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
     Serial.println("");
    Serial.println("WiFi connected");  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Netmask: ");
    Serial.println(WiFi.subnetMask());
    Serial.print("Gateway: ");
    Serial.println(WiFi.gatewayIP());
    ThingSpeak.begin(client);
}

void loop()
{
    // This sketch displays information every time a new sentence is correctly encoded.
    while (gps_serial.available() > 0){
      if (gps.encode(gps_serial.read())){
          displayInfo();
      }
    }
    
    Blynk.run();
    timer.run();
}

void displayInfo()
{
    if (gps.location.isValid() )
    {
        latitude = (gps.location.lat());     //Storing the Lat. and Lon.
        longitude = (gps.location.lng());
        Serial.println(String("latitud: ")+latitude);
        Serial.println(String("longitud: ")+longitude);
        speed = gps.speed.kmph();               //get speed
        Serial.println(String("velocidad: ")+speed);
        direction = TinyGPSPlus::cardinal(gps.course.value()); // get the direction
        Serial.println(String("direccion: ")+direction);
        satellites = gps.satellites.value();    //get number of satellites
        Serial.println(String("satelites: ")+satellites);
        Serial.println("--------------------");
    }
}

void checkGPS(){
    if(gps.location.isValid()){
        latitude = (gps.location.lat());     //Storing the Lat. and Lon.
        longitude = (gps.location.lng());
        Blynk.virtualWrite(V1, String(latitude, 6));
        Serial.println(String("latitud: ")+latitude);
        Blynk.virtualWrite(V2, String(longitude, 6));
        Serial.println(String("longitud: ")+longitude);
        myMap.location(move_index, latitude, longitude, "GPS_Location");
        speed = gps.speed.kmph();               //get speed
        Serial.println(String("velocidad: ")+speed+ "Km/h");
        Blynk.virtualWrite(V3, speed);
        direction = TinyGPSPlus::cardinal(gps.course.value()); // get the direction
        Blynk.virtualWrite(V4, direction);
        Serial.println(String("direccion: ")+direction);
        satellites = gps.satellites.value();    //get number of satellites
        Blynk.virtualWrite(V5, satellites);
        Serial.println(String("satelites: ")+satellites);
        ThingSpeak.setField(1, latitude);
        ThingSpeak.setField(2, longitude);
        ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
        Serial.println("--------------------");
        if(speed > 5){
            Blynk.notify("¡¡El objetivo esta en movimiento!!");
        }
    }else{
        Blynk.virtualWrite(V1,"No GPS");
        Blynk.virtualWrite(V2,"No GPS");
    }
}
