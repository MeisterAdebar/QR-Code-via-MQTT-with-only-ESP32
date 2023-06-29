#include <Arduino.h>
#include <ESP32QRCodeReader.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Add here ssid and password
const char* ssid = "<ssid>";
const char* password = "<pwd>";

// Add here broker ip, port, username and password
const char* mqttServer = "<ip>";
const int mqttPort = 1883;
const char* mqttUser = "<usrname>";
const char* mqttPassword = "pwd";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;



ESP32QRCodeReader reader(CAMERA_MODEL_AI_THINKER);

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
        client.publish("house/qrcode", (const char *)qrCodeData.payload);
        digitalWrite(12, HIGH); // turn the green LED on
        delay(2000);             // wait for 2000 milliseconds
        digitalWrite(12, LOW);  // turn the green LED off
      }
      else
      {
        Serial.print("Invalid: ");
        Serial.println((const char *)qrCodeData.payload);
        digitalWrite(2, HIGH); // turn the red LED on
        delay(200);             // wait for 200 milliseconds
        digitalWrite(2, LOW);  // turn the red LED off
      }
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  pinMode(12, OUTPUT); // configure pin to digital output

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println("Connected to the WiFi network");

  client.setServer(mqttServer, mqttPort);
 
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");

    if (client.connect("doorpen", mqttUser, mqttPassword )) {
 
      Serial.println("connected");  
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
 
    }
}

  Serial.println();

  reader.setup();

  Serial.println("Setup QRCode Reader");

  reader.beginOnCore(1);

  Serial.println("Begin on Core 1");

  xTaskCreate(onQrCodeTask, "onQrCode", 4 * 1024, NULL, 4, NULL);
}

void loop()
{
  client.loop();
  
}
