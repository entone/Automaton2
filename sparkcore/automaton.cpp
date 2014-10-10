// This #include statement was automatically added by the Spark IDE.
#include "application.h"
#include "libraries/MQTT/MQTT.h"
#include "libraries/Base64/Base64.h"
#include "libraries/AnalogSensor/AnalogSensor.h"


SYSTEM_MODE(MANUAL);

byte ip[] = { 192, 168, 1, 251 };

unsigned char KEY[16] = "111111111111111";
unsigned char IV[16] = "111111111111111";

void callback(char* topic, byte* payload, unsigned int length);
void subscribe();
MQTT client(ip, 1883, callback);
AnalogSensor light(A2, 1);

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
    Serial.begin(9600);
    RGB.control(true);
    RGB.brightness(5);
    WiFi.connect();
    delay(1000);
    while(!WiFi.ready()) Serial.println("Connecting Wifi...");
    
    // connect to the server
    client.connect("automaton");
    delay(1000);
    // publish/subscribe
    if (client.isConnected()) {
        client.publish("outTopic","helloéééé world");
        subscribe();
    }
}
// PKCS #7 padding
// Do this before encrypting to get the message
// up to a multiple of 16 bytes.
size_t pad(unsigned char *buf, size_t messageLength) {
    size_t paddedLength = (messageLength & ~15) + 16;
    char pad = paddedLength - messageLength;
    memset(buf + messageLength, pad, pad);
    return paddedLength;
}

String hexPrint(const unsigned char *buf, size_t len) {
    const char hex[] = "0123456789ABCDEF";
    String s = String("");
    for (size_t i = 0; i < len; i++) {
        char c = buf[i];
        char hexDigit = hex[(c >> 4) & 0xF];
        s.concat(hexDigit);
        hexDigit = hex[c & 0xF];
        s.concat(hexDigit);
        s.concat(' ');
    }
    return s;
}

int count = 0;

void loop() {
    if (client.isConnected()){
        client.loop();
    }else{
        client.connect("automaton");
        delay(1000);
        subscribe();
    }

    
    int val = light.read();
    
    count++;
    if(count == 300 && client.isConnected()){
        client.publish("/node/light", val);
        /*aes_context aes;
        unsigned char key[16];
        unsigned char iv[16];
        unsigned char originalIV[16];
        unsigned char buf[48];
 
        Serial.println("Key:");
        hexPrint(KEY, 16);
 
        Serial.println("\nIV:");
        Serial.println(hexPrint(IV, 16));
 
        // Our super secret message goes into buf
        const char *original = "Hey! What a great demo! I can do encryption!";
        int length = strlen(original) + 1; // include null terminating byte
        memcpy(buf, original, length);
 
        // Esoterica: add PKCS #7 padding
        size_t paddedLength = pad(buf, length);
 
        // Print the plaintext directly
        Serial.println("\nPlaintext:");
        Serial.println((const char *)buf);
 
        // Print the plaintext as hex
        Serial.println(hexPrint(buf, paddedLength));
 
        // Encrypt
        aes_setkey_enc(&aes, KEY, 128);
        aes_crypt_cbc(&aes, AES_ENCRYPT, paddedLength, IV, buf, buf);
 
        // Print the ciphertext directly
        Serial.println("\nCiphertext:");
        Serial.println(hexPrint(buf, paddedLength));
 
        // Print the ciphertext as hex
        String s = hexPrint(buf, paddedLength);
        char o_buf[48]; 
        s.toCharArray(o_buf, s.length());
        client.publish("/node/light", *o_buf);*/
        count = 0;
    }
}
