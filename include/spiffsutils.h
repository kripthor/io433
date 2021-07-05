#include "FS.h"
#include "SPIFFS.h"

#define FORMAT_SPIFFS_IF_FAILED true

void listSPIFFS(const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = SPIFFS.open(dirname);
    if(!root){
        Serial.printf("Open failed for %s\n",dirname);
        return;
    }
    if(!root.isDirectory()){
        Serial.printf("%s is not a directory\n",dirname);
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print(" DIR : ");
            Serial.println(file.name());
            if(levels){
                listSPIFFS(file.name(), levels -1);
            }
        } else {
            Serial.print(" FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void loadSPIFFS(const char * path, uint16_t *signal433_store, uint16_t size){
    Serial.printf("Reading file: %s\n", path);
    File file = SPIFFS.open(path);
    if(!file || file.isDirectory()){
        Serial.printf("Load failed for %s\n",path);
        return;
    }
    int i = 0;
    while(file.available() && i < size){
       uint16_t value = ((unsigned char)file.read() << 8) | (unsigned char)file.read();
       signal433_store[i++] = value;
    }
}

void storeSPIFFS(const char * path, uint16_t *signal433_store, uint16_t size){
    Serial.printf("Writing file: %s\r\n", path);
    File file = SPIFFS.open(path, FILE_WRITE);
    if(!file){
       Serial.printf("Store failed for %s\n",path);
        return;
    }
   int i = 0;
   while(i < size){
       file.write((unsigned char)(signal433_store[i]>>8 & 0xff));
       file.write((unsigned char)(signal433_store[i] & 0xff));
       i++;
    }
}



