/* 
Aquarium IOT Project - Author: Hau Pham
Contact following link if you have any question : https://www.facebook.com/AkiPham
*/
#include <ESP8266WiFi.h>
#include<PubSubClient.h>
#include <OneWire.h>
const int pump       = D5;
const int fan        = D6;
const int filter     = D7;
const int feeder     = D8;

char const* switchfan     = "tank/fan/";
char const* switchpump    = "tank/pump/";
char const* switchfilter  = "tank/filter/";
char const* switchfeeder  = "tank/feeder/";

OneWire  ds(D4);
void setup()  // Setup all Pin function
{
  pinMode(fan, OUTPUT);
  digitalWrite(fan, LOW);
  pinMode(pump, OUTPUT);
  digitalWrite(pump, LOW);
  pinMode(filter, OUTPUT);
  digitalWrite(filter, HIGH);
  pinMode(feeder, OUTPUT);
  digitalWrite(feeder, LOW);
  Serial.begin(115200);
  Serial.print("connecting");
  WiFi.begin("DON","quydon4444");         //SSID,PASSWORD 
  while(WiFi.status()!=WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  
  reconnect();

}
// Setup recall function
void callback(char* topic,byte* payload,unsigned int length)
{    
  String topicStr = topic;
Serial.print("message arrived[");
Serial.print(topic);
Serial.println("]");
Serial.println(topicStr);
  for(int i=0;i<length;i++)
  {
  Serial.print(payload[i]); 
  
if(topicStr == "tank/pump")
{
  if((char)payload[0] == '1')
    digitalWrite(pump, HIGH);
    else if((char)payload[0] == '0')
    digitalWrite(pump, LOW);
  }
  else if(topicStr == "tank/fan")
  {
  if((char)payload[0] == '1')
    digitalWrite(fan, HIGH);
    else if((char)payload[0] == '0')
    digitalWrite(fan, LOW);
  }
  else if(topicStr == "tank/filter")
  {
  if((char)payload[0]=='1')
    digitalWrite(filter, HIGH);
    else if((char)payload[0]=='0')
    digitalWrite(filter, LOW);
  }
  else if(topicStr == "tank/feeder")
  {
  if((char)payload[0] == '1')
    digitalWrite(feeder, HIGH);
    else if((char)payload[0] == '0')
    digitalWrite(feeder, LOW);
  }
  delay(100);
Serial.println();
}
}

const char* mqtt_server="test.mosquitto.org";
WiFiClient espclient;
PubSubClient client(mqtt_server,1883,callback,espclient);

void temperature(){

  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;
  
  if ( !ds.search(addr)) {
  /// Serial.println("No more addresses.");
   /// Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }

  
  for( i = 0; i < 8; i++) {  
    addr[i];
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
//  Serial.println();
 
  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:

      type_s = 1;
      break;
    case 0x28:

      type_s = 0;
      break;
    case 0x22:
    //  Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
    //  Serial.println("Device is not a DS18x20 family device.");
      return;
  } 

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);      
  
  delay(1000);    

//  delay(1000);    
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         

  for ( i = 0; i < 9; i++) {       
    data[i] = ds.read();
  }
  OneWire::crc8(data, 8); 
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; 
    if (data[7] == 0x10) {      
      raw = (raw & 0xFFF0) + 12 - data[6];    }
  } else {
    byte cfg = (data[4] & 0x60);

    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    
  }

 
  celsius = (float)raw / 16.0;

   Serial.println(celsius);    
     
   char temperaturenow [15];
   dtostrf(celsius,7, 3, temperaturenow);  //// convert float to char
      
   client.publish("temp/sensor", temperaturenow);    /// send char
}


void reconnect(){
  while(WiFi.status()!=WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  while(!client.connected()){
  if(client.connect("ESP8266Client123456789")){
    Serial.println("connected");
    client.subscribe("tank/pump");
    client.subscribe("tank/fan");
    client.subscribe("tank/filter");
    client.subscribe("tank/feeder");
    client.subscribe("temp/sensor");
  }
    else{
      Serial.print("failed,rc=");
      Serial.println(client.state());
      delay(500);
    }
  } 
}
void loop() {
   temperature(); 
    if(!client.connected()){
      reconnect();
    }
    
    client.loop();
}
