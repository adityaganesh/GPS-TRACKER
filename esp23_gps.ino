#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <TinyGPS++.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

//On ESP32: GPIO-21(SDA), GPIO-22(SCL)
#define OLED_RESET -1 //Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C //See datasheet for Address
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// Replace the next variables with your SSID/Password combination
const char* ssid = "GANESH";
const char* password = "ggag2363";

// Add your MQTT Broker IP address, example:
//const char* mqtt_server = "192.168.1.144";
const char* mqtt_server = "broker.emqx.io";
#define RXD2 16
#define TXD2 17
HardwareSerial neogps(1);
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
TinyGPSPlus gps;
String latitude, longitude;
bool valid = 0;
void setup() {
  Serial.begin(115200);
  //Begin serial communication Arduino IDE (Serial Monitor)

  //Begin serial communication Neo6mGPS
  neogps.begin(9600, SERIAL_8N1, RXD2, TXD2);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.display();
  delay(2000);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.print("WIFI IS CONNECTED");
  display.display();
  delay(2000);
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());



}
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.print(messageTemp);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("YOUR LOCATION WILL BE SENT");
  display.display();
  delay(5000);






  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off".
  // Changes the output state according to the message

}
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("dubblinmqtt")) {
      Serial.println("connected");
      // Subscribe
      display.clearDisplay();
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 0);
      display.setTextSize(2);
      display.print("MQTT CONNECTED");
      display.display();
      client.subscribe("dubblin/getLocation");
      delay(5000);
    } else {
      display.clearDisplay();
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 0);
      display.setTextSize(3);
      display.print("CONNECTION LOST");
      display.display();
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  boolean newData = false;
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (neogps.available())
    {
      // Serial.print(neogps.read());
      if (gps.encode(neogps.read()))
      {
        newData = true;
      }
    }
  }
  Serial.println(newData);
  //If newData is true
  if (newData == true)
  {

    newData = false;
    Serial.println(gps.satellites.value());
    if (gps.satellites.value() == 0)
    {
      display.clearDisplay();
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 0);
      display.setTextSize(3);
      display.print("PROPER DATA");
      display.display();
    }
    print_speed();
  }
 
  else
  {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.setTextSize(3);
    display.print("No Data");
    display.display();
    StaticJsonDocument<450> cred2;
    cred2["LAT"] = "NO DATA";
    cred2["LON"] = "NO DATA";
    cred2["SAT"] = "NO DATA";
    cred2["ALT"] = "NO DATA";
    cred2["SPEED"] = "NO DATA";
    char JSONmessageBuffer[450];
    serializeJson(cred2, JSONmessageBuffer);
    if (client.publish("dubblin/location", JSONmessageBuffer) == true)
    {

      Serial.println("Success sending message for motion");
    }
    else
    {
      Serial.println("Error sending message for motion");
    }
  }

}

void print_speed()
{
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  if (gps.location.isValid() == 1)
  {
    //valid = 1;
    //String gps_speed = String(gps.speed.kmph());
    display.setTextSize(1);

    display.setCursor(25, 5);
    display.print("Lat: ");
    display.setCursor(50, 5);
    display.print(gps.location.lat(), 6);
    latitude = String(gps.location.lat(), 6);
    display.setCursor(25, 20);
    display.print("Lng: ");
    display.setCursor(50, 20);
    display.print(gps.location.lng(), 6);
    longitude = String(gps.location.lng(), 6);
    display.setCursor(25, 35);
    display.print("Speed: ");
    display.setCursor(65, 35);
    display.print(gps.speed.kmph());

    display.setTextSize(1);
    display.setCursor(0, 50);
    display.print("SAT:");
    display.setCursor(25, 50);
    display.print(gps.satellites.value());

    display.setTextSize(1);
    display.setCursor(70, 50);
    display.print("ALT:");
    display.setCursor(95, 50);
    display.print(gps.altitude.meters(), 0);

    display.display();
    StaticJsonDocument<450> cred;
    cred["LAT"] = String(gps.location.lat(), 6);
    cred["LON"] = String(gps.location.lng(), 6);
    cred["SAT"] = String(gps.satellites.value());
    cred["ALT"] = String(gps.altitude.meters(), 0);
    cred["SPEED"] = String(gps.speed.kmph());
    char JSONmessageBuffer[450];
    serializeJson(cred, JSONmessageBuffer);
    if (client.publish("dubblin/location", JSONmessageBuffer) == true)
    {

      Serial.println("Success sending message for motion");
    }
    else
    {
      Serial.println("Error sending message for motion");
    }

  }
  else
  {
   // valid = 0;
    //    display.clearDisplay();
    //    display.setTextColor(SSD1306_WHITE);
    //    display.setCursor(0, 0);
    //    display.setTextSize(3);
    //    display.print("No Data");
    //    display.display();
    StaticJsonDocument<450> cred1;
    cred1["LAT"] = "NO DATA";
    cred1["LON"] = "NO DATA";
    cred1["SAT"] = "NO DATA";
    cred1["ALT"] = "NO DATA";
    cred1["SPEED"] = "NO DATA";
    char JSONmessageBuffer[450];
    serializeJson(cred1, JSONmessageBuffer);
    if (client.publish("dubblin/location", JSONmessageBuffer) == true)
    {

      Serial.println("Success sending message for motion");
    }
    else
    {
      Serial.println("Error sending message for motion");
    }
  }

}
