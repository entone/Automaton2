// This #include statement was automatically added by the Spark IDE.
#include "application.h"
#include "libraries/MQTT/MQTT.h"
#include "libraries/AES/AES.h"
#include "libraries/AnalogSensor/AnalogSensor.h"


SYSTEM_MODE(MANUAL);

byte ip[] = { 192, 168, 1, 9 };

void callback(char* topic, byte* payload, unsigned int length);
void subscribe();
MQTT client(ip, 1883, callback);
AnalogSensor light(A2, 1);
AnalogSensor pot(A4, 2);

unsigned char KEY[16] = "111111111111111";

// recieve message
void callback(char* topic, byte* payload, unsigned int length) {
    char p[length + 1];
    memcpy(p, payload, length);
    p[length] = NULL;
    String message(p);

    if (message.equals("RED"))
        RGB.color(255, 0, 0);
    else if (message.equals("GREEN"))
        RGB.color(0, 255, 0);
    else if (message.equals("BLUE"))
        RGB.color(0, 0, 255);
    else
        RGB.color(255, 255, 255);
    delay(1000);
}

void subscribe(){
    if(client.isConnected())
        client.subscribe("inTopic");
}

void setup() {
    light.init();
    pot.init();
    Serial.begin(9600);
    RGB.control(true);
    RGB.brightness(5);
    WiFi.connect();
    delay(1000);
    Serial.println("running");
    while(!WiFi.ready()){
        Serial.println("Connecting to Wifi...");
        delay(500);
    }
    // connect to the server
    client.connect("automaton");
    delay(1000);
    // publish/subscribe
    if (client.isConnected()) {
        client.publish("outTopic","helloéééé world");
        subscribe();
    }

}

unsigned char l_out[64];
unsigned char p_out[64];
String myIDStr = Spark.deviceID();

void loop() {
    Serial.println("Checking mqtt connection");
    Serial.println(myIDStr);
    if(client.isConnected()){
        Serial.println("Connected");
        Serial.println("Handle mqtt incoming");
        client.loop();
        Serial.println("getting sensor values");
        int l = light.read();
        int p = pot.read();
        Serial.println("encrypt light");
        aes_128_encrypt(l, KEY, l_out);
        Serial.println("publish light");
        client.publish("/node/light", l_out, sizeof(l_out));
        Serial.println("encrypt pot");
        aes_128_encrypt(p, KEY, p_out);
        Serial.println("publish pot");
        client.publish("/node/pot", p_out, sizeof(p_out));
    }
    Serial.println("Looped");
}
