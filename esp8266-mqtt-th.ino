/***************************************************************************************************/
/* 
   This Sketch runs on an ESP8266 controller with integrated WiFi stack, interfaces with an AHT10
   high resolution temperature and humidity senseor via I2C and publishes the collected temperature
   and humidity values via a MQTT broker in a format that can be consumed by OpenHAB.
   
   written by : dmazan
   sourse code: https://github.com/dmazan/esp8266-mqtt-th

   Frameworks & Libraries:
   ESP8266 - https://github.com/esp8266/Arduino
   AHT10   - https://github.com/enjoyneering/AHT10
   MQTT    - https://github.com/knolleary/pubsubclient

   GNU GPL license, all text above must be included in any redistribution,
   see link for details  - https://www.gnu.org/licenses/licenses.html
*/
/***************************************************************************************************/
#include <AHT10.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <secret.h>

/* Local Configuration - ALL_CAPS identifiers are defined in secret.h */

const char* ssid = SECRET_SSID;
const char* password = SECRET_PASSWORD;
const char* mqtt_server = SECRET_MQTT_SERVER;
String mainTopic = "ha";
String temperatureTopic = "_temperature";
String humidityTopic = "_humidity";

uint8_t readStatus = 0;
float temperature = 0;
float humidity = 0;
String clientId = "";

AHT10 myAHT10(AHT10_ADDRESS_0X38);
WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  WiFi.mode(WIFI_STA);
  clientId = String(WiFi.macAddress()).substring(15);
  clientId.toLowerCase();
  temperatureTopic = mainTopic + "/_" + clientId + "/" + temperatureTopic;
  humidityTopic = mainTopic + "/_" + clientId + "/" + humidityTopic;
  Serial.println("Client Id: " + clientId);
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
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient_temperature_sensor")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  WiFi.persistent(false);  //disable saving wifi config into SDK flash area
  //WiFi.forceSleepBegin();  //disable AP & station by calling "WiFi.mode(WIFI_OFF)" & put modem to sleep
  Serial.begin(115200);
  Serial.println("Initialize AHT10");
  
  while (myAHT10.begin(D6,D7) != true)
  {
    Serial.println(F("AHT10 not connected or fail to load calibration coefficient")); //(F()) save string to flash & keeps dynamic memory free
    delay(5000);
  }
  Serial.println(F("AHT10 OK"));

  //Wire.setClock(400000); //experimental I2C speed! 400KHz, default 100KHz

  setup_wifi(); 
  client.setServer(mqtt_server, 1883);
}


void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  temperature = myAHT10.readTemperature(AHT10_FORCE_READ_DATA);
  Serial.print(F("Temperature: ")); Serial.print(temperature);
  client.publish(temperatureTopic.c_str(), String(temperature).c_str(), true);
  Serial.println("...published");
  
  humidity = myAHT10.readHumidity(AHT10_USE_READ_DATA);
  Serial.print(F("Humidity...: ")); Serial.print(humidity);
  client.publish(humidityTopic.c_str(), String(humidity).c_str(), true);
  Serial.println("...published");
  Serial.println();

  delay(30000); //recomended polling frequency 8sec..30sec
}
