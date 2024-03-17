#include <PubSubClient.h>
#include <WiFiManager.h>

const char* mqtt_server = "mqtt.netpie.io";
const char* mqtt_username = "1r8o7C1Gpsd5KwTHg5gBCk7dUeSNZQ6U";
const char* mqtt_password = "FP8xZgMNNAVMS3HKfpJaU6pGgBxnZed9";
const char* client_id = "c92321a5-6809-49da-bf56-ccca2b6e3392";

String otp = "";
const char* status = "UnLock";

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String message;
  for (int i = 0; i < length; i++) {
    message = message + (char)payload[i];
  }
  Serial.println(message);
  getStart(topic, message);
  getOTP(topic, message);
}

void getStart(String t, String mes) {
  if (t == "@msg/open" && mes == "start") {
    client.publish("@msg/open", status);
  }
}

void getOTP(String t, String num) {
  if (t == "@msg/secret") {
    if (status == "UnLock") {
      otp = num;
      status = "Lock";
      client.publish("@msg/open", "Lock");
    } else if (status == "Lock") {
      if (otp == num) {
        otp = "";
        status = "UnLock";
        client.publish("@msg/open", "UnLock");
      } else if (otp == "AdminUnLock") {
        otp = "";
        status = "UnLock";
        client.publish("@msg/open", "AdminUnLock");
        client.publish("@msg/open", "UnLock");
      } else {
        client.publish("@msg/open", "Error");
        client.publish("@msg/open", "Lock");
      }
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(client_id, mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe("@msg/open");
      client.subscribe("@msg/secret");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  WiFiManager wm;

  bool res;
  // res = wm.autoConnect(); // auto generated AP name from chipid
  res = wm.autoConnect("AutoConnectAP");

  if (!res) {
    Serial.println("Failed to connect");
    ESP.restart();
  } else {
    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
  }

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
