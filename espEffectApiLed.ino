
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <WebServer.h>
#include <ArduinoJson.h>

#define LED_COUNT 60
#define LED_PIN 2


const char* ssid     = "name";
const char* password = "password";


bool isEffectRunning = false;
String effectName;
uint8_t R, G, B, Wait;


WebServer server(80);


Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);


void setColor(uint8_t r, uint8_t g, uint8_t b, Adafruit_NeoPixel s) {
    uint8_t num = s.numPixels();
    for(int i = 0; i < num; i++) {
        s.setPixelColor(i, r, g, b);
    }
    s.show();
}


void setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness,  Adafruit_NeoPixel s) {
    uint8_t num = s.numPixels();
    for(int i = 0; i < num; i++) {
        s.setPixelColor(i, r, g, b);
        s.setBrightness(brightness);
    }
    s.show();
}


void handleLed() {
    Serial.println("handleLed");
    try {
        if (server.hasArg("plain")) {
            StaticJsonDocument<200> doc;
            String body = server.arg("plain");
            DeserializationError error = deserializeJson(doc, body);
            if (error) {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.c_str());
                return;
            }
            uint8_t r = doc["r"];
            uint8_t g = doc["g"];
            uint8_t b = doc["b"];
            Serial.println("Deserialized JSON");
            Serial.println(body);
            if (isEffectRunning) {
                isEffectRunning = false;
            }
            for(int i = 0; i < strip.numPixels(); i++) { 
                strip.setPixelColor(i, r, g, b);         
            }
            strip.show();
            server.send(200, "text/plain", "OK");
        } else {
            server.send(400, "text/plain", "BAD ARGS");
        }
    } catch (...) {
        server.send(400, "text/plain", "BAD JSON");
    }
}


void handleEffect() {
    Serial.println("handleEffect");
    try {
        if (server.hasArg("plain")) {
            StaticJsonDocument<200> doc;
            String body = server.arg("plain");
            DeserializationError error = deserializeJson(doc, body);
            if (error) {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.c_str());
                return;
            }
            String effect = doc["effect"];
            R = doc["r"];
            G = doc["g"];
            B = doc["b"];
            Wait = doc["wait"]
            Serial.println("Deserialized JSON");
            Serial.println(body);
            if (!isEffectRunning) {
                isEffectRunning = true;
            }
            effectName = effect;
            server.send(200, "text/plain", "OK");
        } else {
            server.send(400, "text/plain", "BAD ARGS");
        }
    } catch (...) {
        server.send(400, "text/plain", "BAD JSON");
    }
}


void setup_routing() {
    try {
       
        server.on("/led", HTTP_POST, handleLed);
        server.on("/effect", HTTP_POST, handleEffect);
      
        server.onNotFound([]() {
            server.send(404, "text/plain", "Not found");
        });
    } catch (const std::exception& e) {
        Serial.println(e.what());
    }

}




void rainbow(int wait) {
    for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
        strip.rainbow(firstPixelHue);
        strip.show();
        server.handleClient();
        delay(wait);
    }
}


void fadeInOut(uint8_t r, uint8_t g, uint8_t b, int wait) {
    for(int i = 0; i < 255; i++) {
        setColor(r, g, b, i, strip);
        server.handleClient();
        delay(wait);
    }
}

void strobe(uint8_t r, uint8_t g, uint8_t b, int wait) {
    for(int i = 0; i < 255; i++) {
        setColor(r, g, b, 255, strip);
        server.handleClient();
        delay(wait);
        setColor(0, 0, 0, 0, strip);
        server.handleClient();
        delay(wait);
    }
}

void twinkle(uint8_t r, uint8_t g, uint8_t b, int wait, int count) {
    for(int i = 0; i < count; i++) {
        strip.setPixelColor(random(LED_COUNT), r, g, b);
        server.handleClient();
        delay(wait);
    }
}

void setup() {

    try {
        Serial.begin(115200);
        strip.begin();           
        strip.show();           
        strip.setBrightness(50);
       
        setColor(255, 0, 0, strip);

        
        Serial.print("Connecting to ");
        Serial.println(ssid);
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
        }
      
        Serial.println("");
        Serial.println("WiFi connected.");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        setup_routing();
        server.begin();
    } catch (const std::exception& e) {
        Serial.println(e.what());
    }

}

void loop() {
    
    try {
        server.handleClient();
        if(isEffectRunning) {
            if (effectName == "rainbow") {
                rainbow(10);
            } else if (effectName == "fadeInOut") {
                fadeInOut(R, G, B, Wait);
            } else if (effectName == "twinkle") {
                twinkle(R, G, B, Wait, LED_COUNT);
            }
        }
    } catch (const std::exception& e) {
        Serial.println(e.what());
    }
}
