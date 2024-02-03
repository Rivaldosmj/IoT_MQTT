#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SimpleDHT.h>


// DHT 11
int pinDHT11 = 12; //pin D6
SimpleDHT11 dht11; 

// WiFi
const char *ssid = "30"; // Enter your WiFi name
const char *password = "bayernmania";  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";
const char *topic = "sensor/status";
const char *humidity_ch = "sensor/humidity";
const char *temperature_ch = "sensor/temperature";
const char *mqtt_username = "admin"; //"emqx";
const char *mqtt_password = "public";
const int mqtt_port = 1883;
WiFiClient espClient;
PubSubClient client(espClient);



void setup() {
  // Set software serial baud to 115200;
  Serial.begin(115200);
  // connecting to a WiFi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  //connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  while (!client.connected()) {
      String client_id = "esp8266-client-";
      client_id += String(WiFi.macAddress());
      Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
      if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
          Serial.println("Public emqx mqtt broker connected");
      } else {
          Serial.print("failed with state ");
          Serial.print(client.state());
          delay(2000);
      }
  }
  // publish and subscribe
  client.publish(topic, "hello emqx");
  client.subscribe(topic);
}

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
      Serial.print((char) payload[i]);
  }
  Serial.println();
  Serial.println("-----------------------");
}

void loop() {
  client.loop();
    delay(1000);
byte temperature = 0; 
byte humidity = 0;
int err = SimpleDHTErrSuccess; 
if ((err = dht11.read (pinDHT11, &temperature, &humidity, NULL)) != SimpleDHTErrSuccess) { Serial.println("Read DHT11 failed, err="); 
 Serial.println(err); 
 delay(1000); 
        return;
        }

Serial.print("Temperature: ");
Serial.print( (int)temperature);     client.publish(temperature_ch, String(temperature).c_str(), true);
Serial.println(" C");

Serial.print("Humidity: ");
Serial.print( (int)humidity);
client.publish(humidity_ch, String(humidity).c_str(), true);
    Serial.println(" %"); Serial.println("===================="); 
    delay(5000);
}   
