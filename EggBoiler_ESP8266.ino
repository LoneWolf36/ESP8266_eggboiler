/***************************************************
  Adafruit MQTT Library ESP8266 Example

  Must use ESP8266 Arduino from:
    https://github.com/esp8266/Arduino

  Works great with Adafruit's Huzzah ESP board & Feather
  ----> https://www.adafruit.com/product/2471
  ----> https://www.adafruit.com/products/2821

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Tony DiCola for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// Define the pin to connect to
#define relayPin 16

unsigned int timer = 0;
bool flag = false;
bool flag1 = false;
/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "YOUNet"
#define WLAN_PASS       "Serenity"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "LoneWolf09"
#define AIO_KEY         "074f69f8173447a3b26d1e2ffb16cc9f"

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Setup a feed called 'onoff' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish onoffbutton = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/eggboiler");

// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe onoffbutton1 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/eggboiler");

/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

void setup() {
  Serial.begin(115200);
  delay(10);

  Serial.println(F("Adafruit MQTT demo"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());
  pinMode(relayPin, OUTPUT);

  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&onoffbutton1);
}

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // Logic for automatic switching off of the setup
  if (flag == true) {
    timer++;
    Serial.println(timer);
    if ( timer == 180 ) {
      // Now we can publish stuff!
      if (! onoffbutton.publish("OFF")) {
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("OK!"));
      }
      digitalWrite(relayPin, LOW);
      timer = 0;
      flag = false;
      flag1 = true;
    }
  }

  if (flag1 == true) {
    timer++;
    if ( timer == 12 ) {
      // Now we can publish stuff!
      if (! onoffbutton.publish("OFF")) {
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("OK!"));
      }
      digitalWrite(relayPin, LOW);
      timer = 0;
      flag1 = false;
    }
  }

  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &onoffbutton1) {
      Serial.print(F("Got: "));
      Serial.println((char *)onoffbutton1.lastread);
    }
    if ((strcmp((char *)onoffbutton1.lastread, "ON") == 0)) {
      flag = true;
      digitalWrite(relayPin, HIGH);
    }
    if ((strcmp((char *)onoffbutton1.lastread, "OFF") == 0)) {
      flag = false;
      timer = 0;
      digitalWrite(relayPin, LOW);
    }
  }

  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  /*
    if(! mqtt.ping()) {
    mqtt.disconnect();
    }
  */
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0) {
      // basically die and wait for WDT to reset me
      while (1);
    }
  }
  Serial.println("MQTT Connected!");
}
