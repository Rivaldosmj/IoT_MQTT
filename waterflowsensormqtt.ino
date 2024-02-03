#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Water Flow Sensor
const int pinWaterFlow = D5; // Pin yang terhubung dengan sensor aliran air
volatile int flow_frequency; // Variabel untuk menyimpan frekuensi aliran air
unsigned int flow_rate;      // Aliran air dalam liter per menit (LPM)
unsigned long totalMilliLitres;
unsigned int totalLitres;
unsigned long oldTime;

// WiFi
const char *ssid = "30"; // Ganti dengan nama WiFi Anda
const char *password = "bayernmania"; // Ganti dengan kata sandi WiFi Anda

// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";
const char *topic = "sensor/status";
const char *flow_rate_ch = "sensor/flow_rate";
const char *total_litres_ch = "sensor/total_litres";
const char *mqtt_username = "admin"; // Ganti dengan nama pengguna MQTT Anda
const char *mqtt_password = "public"; // Ganti dengan kata sandi MQTT Anda
const int mqtt_port = 1883;
WiFiClient espClient;
PubSubClient client(espClient);

// Deklarasi prototipe fungsi flowCounter()
void ICACHE_RAM_ATTR flowCounter();

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi network");

  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);

  while (!client.connected()) {
    String client_id = "esp8266-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Public EMQX MQTT broker connected");
    } else {
      Serial.print("Failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }

  client.publish(topic, "Hello EMQX");
  client.subscribe(topic);

  attachInterrupt(digitalPinToInterrupt(pinWaterFlow), flowCounter, RISING); // Mengatur pin interupsi untuk menghitung frekuensi aliran air
  oldTime = millis(); // Inisialisasi waktu awal
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
  if ((millis() - oldTime) > 1000) { // Setiap detik
    detachInterrupt(digitalPinToInterrupt(pinWaterFlow)); // Matikan interupsi untuk membaca frekuensi aliran air
    flow_rate = (flow_frequency / 7.5); // Menghitung aliran air dalam liter per menit (LPM)
    totalLitres += (flow_rate / 60); // Menghitung total liter yang telah dilewati sejak awal
    client.publish(flow_rate_ch, String(flow_rate).c_str(), true);
    client.publish(total_litres_ch, String(totalLitres).c_str(), true);
    flow_frequency = 0; // Reset frekuensi aliran air
    attachInterrupt(digitalPinToInterrupt(pinWaterFlow), flowCounter, RISING); // Aktifkan kembali interupsi untuk menghitung frekuensi aliran air
    oldTime = millis(); // Reset waktu
    Serial.print ("debit = ");
    Serial.print (flow_rate); 
    Serial.print (" Liter/menit");
    Serial.println ("");
  }
}

// Definisi fungsi flowCounter()
void ICACHE_RAM_ATTR flowCounter() {
  flow_frequency++;
}
