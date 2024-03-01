
#include <PubSubClient.h>
#include <WiFi.h>

const char *ssid = "Adonis";
const char *password = "12345678";
const char *esp_mqtt_server = "mqtt.zig-web.com";
const int esp_mqtt_port = 1883;
const char *esp_mqtt_topic1 = "mutex/mqtt";
const char *esp_mqtt_topic2 = "mutex/mqtt2";
bool wifiAvailable = false;

void esp_task1(void *pvParameters);
void esp_task2(void *pvParameters);

WiFiClient espClient;
PubSubClient client(espClient);

SemaphoreHandle_t xMutex = NULL;
int counter = 0;

void setupWiFi()
{
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 10)
  {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  wifiAvailable = true;
  Serial.println("");
  Serial.println("WiFi connected");
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client"))
    {
      Serial.println("connected");
      // client.subscribe(esp_mqtt_topic1);
      client.subscribe(esp_mqtt_topic2);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup()
{

  Serial.begin(115200);
  setupWiFi();
  void checkWiFi();
  client.setServer(esp_mqtt_server, esp_mqtt_port);
  client.setCallback(callback);
  xMutex = xSemaphoreCreateMutex();
  xTaskCreatePinnedToCore(
      esp_task1,
      "task1",
      10000,
      NULL,
      10,
      NULL,
      0);
  xTaskCreatePinnedToCore(
      esp_task2,
      "task2",
      10000,
      NULL,
      10,
      NULL,
      0);
}

void loop()
{
}

void esp_task1(void *pvParameters)
{
  while (1)
  {

    if (client.connected())
    {
      client.publish(esp_mqtt_topic1, "Hello from ESP32");
    }

    delay(500);
  }
}

void esp_task2(void *pvParameters)
{
  while (1)
  {
    if (!client.connected())
    {
      reconnect();
    }
    client.loop();
    delay(500);
  }
}
void checkWiFi()
{
  while (1)
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      if (wifiAvailable)
      {
        Serial.println("Wi-Fi disconnected");
        wifiAvailable = false;
      }
    }
    else
    {
      if (!wifiAvailable)
      {
        Serial.println("Wi-Fi reconnected");
        wifiAvailable = true;
      }
    }
    delay(1000);
  }
}