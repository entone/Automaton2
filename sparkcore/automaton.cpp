// This #include statement was automatically added by the Spark IDE.
#include "application.h"
#include "libraries/MQTT/MQTT.h"
#include "libraries/AES/AES.h"
#include "libraries/AnalogSensor/AnalogSensor.h"
#include "libraries/DHT/DHT.h"

#define DHTPIN D0
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

SYSTEM_MODE(MANUAL);

byte ip[] = { 192, 168, 1, 174 };

void callback(char* topic, byte* payload, unsigned int length);
void subscribe();
MQTT client(ip, 1883, callback);
AnalogSensor light(A2, 1);
AnalogSensor pot(A4, 2);
String myIDStr = Spark.deviceID();
char id[30];


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

void connect(){
    myIDStr.toCharArray(id, 30);
    client.connect(id);
    delay(1000);
}

void setup() {
    light.init();
    pot.init();
    dht.begin();
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
    connect();
    // publish/subscribe
    if (client.isConnected()) {
        client.publish("outTopic","helloéééé world");
        subscribe();
    }

}

char l_out[10];
char p_out[10];
char h_out[10];
char t_out[10];
int count = 0;
void loop() {
    client.loop();
    light.read();
    pot.read();
    count++;
    if(client.isConnected() && count == 100){
        String l = String(light.read(), DEC);
        String p = String(pot.read(), DEC);
        sprintf(h_out, "%.2f", dht.readHumidity());
        sprintf(t_out, "%.2f", dht.readTemperature(false));
        Serial.println("Humidity: ");
        Serial.println(h_out);
        Serial.println("Temp: ");
        Serial.println(t_out);
        client.publish("/node/temperature", t_out);
        client.publish("/node/humidity", h_out);
        //Serial.println("encrypt light");
        //aes_128_encrypt(l, KEY, l_out);
        Serial.println("publish light");
        Serial.println(l);
        l.toCharArray(l_out, 10);
        client.publish("/node/light", l_out);
        //Serial.println("encrypt pot");
        //aes_128_encrypt(p, KEY, p_out);
        Serial.println("publish pot");
        Serial.println(p);
        p.toCharArray(p_out, 10);
        client.publish("/node/pot", p_out);
        count = 0;
    }else if(!client.isConnected()){
        connect();
    }
}
