#include <DHT.h>    // DHT ---> D3 Relay ---> D2 VCC ---> 3V3 GND ---> GND 
#include "SimpleDHT.h"
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#define Relay D2
// WiFi parameters
#define WLAN_SSID       "xxxxxxxxxxxxxxx"   //WiFi ssid
#define WLAN_PASS       "xxxxxxxxxx" //WiFi Password
// Adafruit IO
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "xxxxxxxx" //your username
#define AIO_KEY         "xxxxxxxxxxxxxxxxxxxxxxxxxx" //your key 
WiFiClient client;
// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish Temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Temperature");
Adafruit_MQTT_Publish Humidity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Humidity");
Adafruit_MQTT_Subscribe Fan = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/Fan Button");
Adafruit_MQTT_Subscribe Auto = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/Auto");
int pinDHT11 = 0;
SimpleDHT11  dht11(pinDHT11);
byte hum = 0;  //Stores humidity value
byte temp = 0; //Stores temperature value
void setup() {
  Serial.begin(115200);
  pinMode(Relay,OUTPUT);
  Serial.println(F("Adafruit IO Example"));
  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  delay(10);
  Serial.print(F("Connecting to "));
  Serial.println(WLAN_SSID);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();
  Serial.println(F("WiFi connected"));
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());
  mqtt.subscribe(&Auto);
  mqtt.subscribe(&Fan);
  // connect to adafruit io
  connect();

}

// connect to adafruit io via MQTT
void connect() {
  Serial.print(F("Connecting to Adafruit IO... "));
  int8_t ret;
  while ((ret = mqtt.connect()) != 0) {
    switch (ret) {
      case 1: Serial.println(F("Wrong protocol")); break;
      case 2: Serial.println(F("ID rejected")); break;
      case 3: Serial.println(F("Server unavail")); break;
      case 4: Serial.println(F("Bad user/pass")); break;
      case 5: Serial.println(F("Not authed")); break;
      case 6: Serial.println(F("Failed to subscribe")); break;
      default: Serial.println(F("Connection failed")); break;
    }

    if(ret >= 0)
      mqtt.disconnect();

    Serial.println(F("Retrying connection..."));
    delay(10000);
  }
  delay(3000);
  Serial.println(F("Adafruit IO Connected!"));
}

void loop() {
  // ping adafruit io a few times to make sure we remain connected
  if(! mqtt.ping(3)) {
    // reconnect to adafruit io
    if(! mqtt.connected())
      connect();
  }
  Adafruit_MQTT_Subscribe *subscription;
  subscription = mqtt.readSubscription(20000);
    Serial.println();Serial.print(F("Auto: "));
    Serial.println((char *)Auto.lastread);
    int autoresult = strcmp((char *)Auto.lastread,"ON");
    if(autoresult==0)
    {
      if(int(temp)>=34){
        digitalWrite(Relay,LOW);Serial.print("Relay ON");
      }
      if(int(temp)<=31){
        digitalWrite(Relay,HIGH);Serial.print("Relay OFF");
      }
    }
    else{
        int Fan_State=0;
        Serial.print(F("Fan Button: "));
        Serial.println((char *)Fan.lastread);
        int result = strcmp((char *)Fan.lastread,"ON");
        if(result==0){
          Fan_State=1;
        }
        if(Fan_State==1){
        digitalWrite(Relay,LOW);Serial.print("Relay ON");}
        if(Fan_State==0){
        digitalWrite(Relay,HIGH);Serial.print("Relay OFF");}
    }
    
  dht11.read(&temp, &hum, NULL);
  Serial.println();Serial.print((int)temp);Serial.print(" *C, ");
  Serial.print((int)hum); Serial.println(" H");
  delay(5000);
   
   if (! Temperature.publish(temp)) {                     //Publish to Adafruit
      Serial.println(F("Failed"));
    } 
       if (! Humidity.publish(hum)) {                     //Publish to Adafruit
      Serial.println(F("Failed"));
    }
    else {
      Serial.println(F("Sent!"));
    }
}
