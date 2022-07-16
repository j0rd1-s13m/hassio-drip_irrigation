#include <DHT.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "XXXXXXXXXXXX";
//Iglu_IoT
const char* password = "XXXXXXXXXXXX";
const char* mqtt_server = "XXX.XXX.XXX.XXX";
const char* mqttUser = "XXXXXXXXXXXX";
const char* mqttPassword = "XXXXXXXXXXXX";
const int mqttPort = 1883;

IPAddress ip(XXX,XXX,XXX,XXX);     
IPAddress gateway(XXX,XXX,XXX,XXX);   
IPAddress subnet(XXX,XXX,XXX,XXX);   


WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

int buttonState = 0;
int lastButtonState = 0;

int switch_water = 13;
int relay_water = 5;
int TempPin = 0;

#define DHTTYPE DHT21
DHT dht(TempPin, DHTTYPE);

float Temperature;
float Humidity;

unsigned long lastMillis;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '0') {
    Serial.println("Desactivar");
    digitalWrite(relay_water, HIGH);
  }
  if ((char)payload[0] == '1') {
    Serial.println("Activar");
    digitalWrite(relay_water, LOW);
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqttUser, mqttPassword)) {
      Serial.println("connected");
      client.subscribe("house/garden/water/relay");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  setup_wifi();
  pinMode(switch_water, INPUT);
  pinMode(relay_water, OUTPUT);

  delay(2000);
  digitalWrite(relay_water, HIGH);
  
  dht.begin();
  
  Serial.println(digitalRead(switch_water));
  

  lastButtonState = digitalRead(switch_water); 
   
  //attachInterrupt(switch_water, switch_changed, CHANGE);   

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void sendData(){  
  
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);
  
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" *C ");
  Serial.println(digitalRead(switch_water));  
  
  char temp[4];
  dtostrf(t, 4, 2, temp);

  Serial.println(t); 
  Serial.println(temp); 
  client.publish("house/0/garden/temp", temp);   
  
  char hum[4];
  dtostrf(h, 4, 2, hum);   
  Serial.println(hum);  
  client.publish("house/0/garden/hum", hum); 
}


void loop() {
  if (!client.connected()) {
    reconnect();
  }  
  
  client.loop();  
  if (millis() - lastMillis >= 5*60*1000UL) 
    {
      lastMillis = millis();
      sendData();
    }

  buttonState = digitalRead(switch_water);  
    
  if (buttonState != lastButtonState) {      
    if (buttonState == HIGH) {
      client.publish("house/0/garden/switch", "0"); 
    }
    if (buttonState == LOW) {
      client.publish("house/0/garden/switch", "1"); 
    }    
    delay(50);
  }  
  
  lastButtonState = buttonState;
}
