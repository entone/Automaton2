#include "application.h"
#include "AES.h"
// PKCS #7 padding
// Do this before encrypting to get the message
// up to a multiple of 16 bytes.
size_t pad(unsigned char *buf, size_t messageLength) {
    size_t paddedLength = (messageLength & ~15) + 16;
    char pad = paddedLength - messageLength;
    memset(buf + messageLength, pad, pad);
    return paddedLength;
}

void sixteenRandomBytes(unsigned char buf[16]) {
    for (int i = 0; i < 16; i++) {
        buf[i] = rand() & 0xff;
    }
}

void slice(unsigned char dest[128], unsigned char original[128], int start, int end){
    int counter = 0;
    for(int i=start; i<end; i++){
        dest[counter] = original[i];
        counter++;
    }
}

void append(unsigned char dest[128], unsigned char original[128], int d_len, int o_len){
    int counter = d_len;
    for(int i=0; i<o_len; i++){
        dest[counter] = original[i];
        counter++;
    }
}

void print_char(unsigned char msg[128], size_t len){
    for(int i=0; i<len; i++){
        Serial.write(msg[i]);
    }
    Serial.println("");
}

void print_char(char msg[128], size_t len){
    for(int i=0; i<len; i++){
        Serial.write(msg[i]);
    }
    Serial.println("");
}

void aes_128_encrypt(int value, unsigned char key[16], unsigned char output[64]){        
    unsigned char buf[48];
    unsigned char IV[16];
    //generate random IV
    sixteenRandomBytes(IV);
    //copy original IV into final output
    memcpy(output, IV, 16);

    char original[48];
    String s = String(value);
    int length = s.length()+1;
    s.toCharArray(original, length);
    memcpy(buf, original, length);
    size_t paddedLength = pad(buf, length);

    aes_context aes;
    aes_setkey_enc(&aes, key, 128);
    // Encrypt
    Serial.println("Original");
    print_char(buf, length);
    aes_crypt_cbc(&aes, AES_ENCRYPT, paddedLength, IV, buf, buf);
    Serial.println("Encrypted");
    append(output, buf, 16, sizeof(buf));
    print_char(output, sizeof(output));
}