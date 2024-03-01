
#include <PubSubClient.h>
#include <WiFi.h>

const char *ssid = "ACTFIBERNET";
const char *password = "act12345";
const char *mqtt_server = "mqtt.zig-web.com";
const int mqtt_port = 1883;
const char *mqtt_topic = "mutex/mqtt";
const char *mqtt_topic2 = "mutex/mqtt2";

void task1(void *pvParameters);
void task2(void *pvParameters);

WiFiClient espClient;
PubSubClient client(espClient);

SemaphoreHandle_t xMutex = NULL;
int counter = 0;

void setupWiFi()
{
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
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
      //client.subscribe(mqtt_topic);
      client.subscribe(mqtt_topic2);
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
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  xMutex = xSemaphoreCreateMutex();
  xTaskCreatePinnedToCore(
      task1,
      "task1",
      10000,
      NULL,
      10,
      NULL,
      0);
  xTaskCreatePinnedToCore(
      task2,
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

void task1(void *pvParameters)
{
  while (1)
  {
    
    if (client.connected())
    {
      client.publish(mqtt_topic, "Hello from ESP32");
    }

    delay(500);
  }
}

void task2(void *pvParameters)
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
