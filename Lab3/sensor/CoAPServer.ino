/* 
   Clement LEFEBVRE / Implementing IoT protocols
   Code created for lab 3: Project
   Provides a simple CoAP server compatible with ESP8266. 
   It has 2 topics: 
    occupancy: provides a rondomize value simulating an occupancy sensor
    light: provides a way to turn ON/OFF the builtin led (also possible to get the led status)

  Code based on the examples from the library: https://github.com/hirotakaster/CoAP-simple-library 
*/

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <coap-simple.h>

//Access point info 
const char* ssid     = "DESKTOP-FDH";
const char* password = "bonjour00";

// CoAP client response callback
void callback_response(CoapPacket &packet, IPAddress ip, int port);

// CoAP server endpoint url callback
void callback_light(CoapPacket &packet, IPAddress ip, int port);

void callback_occupancy(CoapPacket &packet, IPAddress ip, int port);

// UDP and CoAP class
// other initialize is "Coap coap(Udp, 512);"
// 2nd default parameter is COAP_BUF_MAX_SIZE(defaulit:128)
// For UDP fragmentation, it is good to set the maximum under
// 1280byte when using the internet connection.
WiFiUDP udp;
Coap coap(udp);

// LED STATE
bool LEDSTATE;

// CoAP server endpoint URL (light)
void callback_light(CoapPacket &packet, IPAddress ip, int port) {
  Serial.println("[Light] ON/OFF");
  
  // send response
  char p[packet.payloadlen + 1];
  memcpy(p, packet.payload, packet.payloadlen);
  p[packet.payloadlen] = NULL;
  
  String message(p);

  if (message.equals("0"))
    LEDSTATE = false;
  else if(message.equals("1"))
    LEDSTATE = true;
      
  if (LEDSTATE) {
    digitalWrite(BUILTIN_LED, HIGH) ; 
    coap.sendResponse(ip, port, packet.messageid, "1");
  } else { 
    digitalWrite(BUILTIN_LED, LOW) ; 
    coap.sendResponse(ip, port, packet.messageid, "0");
  }
}

// CoAP server endpoint URL (light)
void callback_occupancy(CoapPacket &packet, IPAddress ip, int port) {
  Serial.println("Occupancy");
  if(random(2) == 1)
    coap.sendResponse(ip, port, packet.messageid, "1");
  else 
    coap.sendResponse(ip, port, packet.messageid, "0");
}

// CoAP client response callback
void callback_response(CoapPacket &packet, IPAddress ip, int port) {
  Serial.println("[Coap Response got]");
  
  char p[packet.payloadlen + 1];
  memcpy(p, packet.payload, packet.payloadlen);
  p[packet.payloadlen] = NULL;
  
  Serial.println(p);
}

void setup() {
  Serial.begin(9600);
  //used for randomizing occupancy values
  randomSeed(analogRead(0));

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // LED State
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, HIGH);
  LEDSTATE = true;
  
  // add server url endpoints.
  // can add multiple endpoint urls.
  // exp) coap.server(callback_switch, "switch");
  //      coap.server(callback_env, "env/temp");
  //      coap.server(callback_env, "env/humidity");
  Serial.println("Setup Callback Light");
  coap.server(callback_light, "light");
  Serial.println("Setup Callback Occupancy");
  coap.server(callback_occupancy, "occupancy");

  // client response callback.
  // this endpoint is single callback.
  Serial.println("Setup Response Callback");
  coap.response(callback_response);

  // start coap server/client
  coap.start();
}

void loop() {
  delay(1000);
  coap.loop();
}
