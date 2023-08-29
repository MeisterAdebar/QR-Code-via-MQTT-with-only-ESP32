#include <Arduino.h>
#include <ESP32QRCodeReader.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>

// WS2812B
#define NUM_PIXELS 1
#define PIN 2

// MQTT Topics
#define TOPIC_IN  "house/in"
#define TOPIC_LWT "house/lwt"
#define TOPIC_QRCODE "house/qrcode"

// WiFi
const char* ssid        = "<ssid>";
const char* password    = "<pwd>";

// MQTT
const char* mqtt_server = "<ip>";
const int mqttPort = 1883;
const char* mqttUser = "usrname";
const char* mqttPassword = "pwd";

// MQTT LWT
byte willQoS = 0;
const char* willTopic = "house/lwt";
const char* willMessage = "offline";
boolean willRetain = false;


ESP32QRCodeReader reader(CAMERA_MODEL_AI_THINKER);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  payload[length] = '\0';
  if (strcmp(topic,TOPIC_IN)==0){

//    String payload_str = (char *) payload;
//    setMode(payload_str.substring(6,7).toInt(),payload_str.substring(8,9).toInt());
  }
}

WiFiClient espClient;
PubSubClient client(mqtt_server,mqttPort,callback,espClient);
// declare pixels
Adafruit_NeoPixel pixels(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup_wifi() {
  Serial.println();
  Serial.printf("Connecting to %s ",ssid);

  WiFi.mode(WIFI_STA);
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
  Serial.println("");
}

void reconnect() {
  while (!client.connected()) {
    String clientId = "ESP-QRCODE";
    Serial.print("Attempting MQTT connection to ");
    Serial.print(mqtt_server);
    Serial.print(" as ");
    Serial.println(clientId.c_str());
    if (client.connect(clientId.c_str(), mqttUser, mqttPassword, willTopic, willQoS, willRetain, willMessage)) {
      clientId += " connected";
      Serial.println(clientId);
      Serial.println();
      client.publish(TOPIC_LWT,"online");
      client.subscribe(TOPIC_IN);
    } else {
      Serial.print(clientId);
      Serial.print(" failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void onQrCodeTask(void *pvParameters)
{
  struct QRCodeData qrCodeData;

  while (true)
  {
    if (reader.receiveQrCode(&qrCodeData, 100))
    {
      Serial.println("Found QRCode");
      if (qrCodeData.valid)
      {
        Serial.print("Payload: ");
        Serial.println((const char *)qrCodeData.payload);
        client.publish(TOPIC_QRCODE, (const char *)qrCodeData.payload);
        pixels.setPixelColor(0, pixels.Color(0, 150, 0));  //green
        pixels.show();
        delay(2000);             // wait for 2000 milliseconds
        pixels.clear();
        pixels.show();
      }
      else
      {
        Serial.print("Invalid: ");
        Serial.println((const char *)qrCodeData.payload);
        pixels.setPixelColor(0, pixels.Color(150, 0, 0));  //red
        pixels.show();
        delay(200);             // wait for 2000 milliseconds
        pixels.clear();
        pixels.show();
      }
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  pixels.begin();
    Serial.println();

  reader.setup();

  Serial.println("Setup QRCode Reader");

  reader.beginOnCore(1);

  Serial.println("Begin on Core 1");

  xTaskCreate(onQrCodeTask, "onQrCode", 4 * 1024, NULL, 4, NULL);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  delay(100);
}

void loop()
{
  client.loop();
  
}
