#include <ESP8266WiFi.h>
#include <espnow.h>

const int ReléPin = 0;

void setup() {
  Serial.begin(115200);
  pinMode(ReléPin, OUTPUT);
  digitalWrite(ReléPin, LOW);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_now_init();
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(onDataReceived);
}

void loop() {

}

void onDataReceived(uint8_t *senderMac, uint8_t *data, uint8_t len) {
  if (*data == 1) {
    digitalWrite(ReléPin, HIGH);
    Serial.println("LED ON");
  } else if (*data == 0) {
    digitalWrite(ReléPin, LOW);
    Serial.println("LED OFF");
  }

}