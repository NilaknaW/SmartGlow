

//This is a working code do not touch anything
//this code works for all the basic fuunctionality needed

#include <PubSubClient.h>
#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>


// MQTT broker settings
const char* mqtt_server = "ora6.emanate.io";
const int mqtt_port = 1883;

String ssid;
String password;
String mqttTopic;
String bulbID;

const char* ssidAP = "Smartglow";
const char* passwordAP = "password";

// MQTT topic to subscribe to
//const char* mqtt_topic = "test";
char msg[4];

WiFiClient espClient;
PubSubClient client(espClient);

ESP8266WebServer server(80);

void saveCredentials() {
  EEPROM.begin(512);
  EEPROM.put(0, ssid);
  EEPROM.put(32, password);
  EEPROM.put(64, mqttTopic);
  EEPROM.put(96, bulbID);
  EEPROM.commit();
  EEPROM.end();
}

void loadCredentials() {
  EEPROM.begin(512);
  char ssidArr[32];
  char passwordArr[32];
  char mqttTopicArr[32];
  char bulbIDArr[32];

  EEPROM.get(0, ssidArr);
  EEPROM.get(32, passwordArr);
  EEPROM.get(64, mqttTopicArr);
  EEPROM.get(96, bulbIDArr);

  ssid = String(ssidArr);
  password = String(passwordArr);
  mqttTopic = String(mqttTopicArr);
  bulbID = String(bulbIDArr);

  EEPROM.end();
}

// Function to handle MQTT messages
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    msg[i] = (char)payload[i];
  }
  char* commaPos = strchr(msg, ',');

  if (commaPos != nullptr) {
    *commaPos = '\0';

    char* onoff_state_msg = msg;
    char* bulb_id_msg = commaPos + 1;

    int onoff_state = atoi(onoff_state_msg);
    int bulb_id = atoi(bulb_id_msg);

    if (bulb_id == bulbID.toInt()) {
      if (onoff_state == 1) {
        digitalWrite(2, 1);
      } else {
        digitalWrite(2, 0);
      }
    }
  }

  Serial.println();

  // Add your logic here based on the received message
}

void setup_wifi() {
  // Create WiFi access point
  WiFi.softAP(ssidAP, passwordAP);

  IPAddress apIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(apIP);


  server.on("/save", HTTP_POST, []() {
    if (server.hasArg("plain")) {
      String requestBody = server.arg("plain");
      Serial.println("Received POST request body:");
      Serial.println(requestBody);

      // Manual JSON parsing
      int ssidStart = requestBody.indexOf("\"ssid\":\"") + 8;
      int ssidEnd = requestBody.indexOf("\"", ssidStart);
      int passwordStart = requestBody.indexOf("\"password\":\"") + 12;
      int passwordEnd = requestBody.indexOf("\"", passwordStart);
      int mqttTopicStart = requestBody.indexOf("\"mqtt_topic\":\"") + 14;
      int mqttTopicEnd = requestBody.indexOf("\"", mqttTopicStart);
      int bulbIDStart = requestBody.indexOf("\"bulbID\":\"") + 10;
      int bulbIDEnd = requestBody.indexOf("\"", bulbIDStart);

      if (ssidStart > 7 && passwordStart > 11 && mqttTopicStart > 13 && bulbIDStart > 9) {
        ssid = requestBody.substring(ssidStart, ssidEnd);
        password = requestBody.substring(passwordStart, passwordEnd);
        mqttTopic = requestBody.substring(mqttTopicStart, mqttTopicEnd);
        bulbID = requestBody.substring(bulbIDStart, bulbIDEnd);

        // Print the received parameters
        Serial.println("Parameters received:");
        Serial.println("SSID: " + ssid);
        Serial.println("Password: " + password);
        Serial.println("MQTT Topic: " + mqttTopic);
        Serial.println("bulbID: " + bulbID);

        // Save the WiFi credentials and MQTT topic
        // Save the WiFi credentials and MQTT topic
        // preferences.begin("wifi", false);
        // preferences.putString("ssid", ssid);
        // preferences.putString("password", password);
        // preferences.putString("mqtt_topic", mqttTopic);
        // preferences.putString("bulbID", bulbID);
        // preferences.end();

         // Save the WiFi credentials and MQTT topic
        saveCredentials();

        WiFi.begin(ssid.c_str(), password.c_str());

        Serial.println("done");
        server.send(200, "text/plain", "done");
      }
    } else {
      // request->send(400, "text/plain", "Missing parameters");
      Serial.println("Missing parameters");
      server.send(400, "text/plain", "Missing parameters");
    }
  });

  server.begin();
  Serial.println("server begun");
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    server.handleClient();
    // Serial.print("beginning");
    // Serial.println(!client.connected());
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println(client.connected());
      Serial.println("connected");
      // Once connected, subscribe to the topic
      client.subscribe(mqttTopic.c_str());
      Serial.println("subscribed");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 3 seconds");
      // Wait 5 seconds before retrying
      delay(3000);
    }
  }
}

// void setup() {
//   pinMode(2,OUTPUT);
//   pinMode(4,OUTPUT);
//   Serial.begin(115200);
//   setup_wifi();
//   client.setServer(mqtt_server, mqtt_port);
//   client.setCallback(callback);
// }

void setup() {
  Serial.print("came to setup");
  pinMode(2, OUTPUT);
  //pinMode(4, OUTPUT);
  Serial.begin(115200);

    loadCredentials();

  // preferences.begin("wifi", true);
  // ssid = preferences.getString("ssid", "");
  // password = preferences.getString("password", "");
  // mqttTopic = preferences.getString("mqtt_topic", "");
  // bulbID = preferences.getString("bulbID", "");
  // preferences.end();

  if (ssid.length() > 0 && password.length() > 0) {
    WiFi.begin(ssid.c_str(), password.c_str());
    // while (WiFi.status() != WL_CONNECTED) {
    //   delay(500);
    //   Serial.print(".");
    // }
    Serial.println("WiFi connected");
  }

  setup_wifi();
  Serial.print("came to the setup wifi");

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  server.handleClient();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
