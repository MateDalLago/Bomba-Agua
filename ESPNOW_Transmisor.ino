#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Ticker.h>
#include <espnow.h>
#include <ESP8266mDNS.h>

#define ledWifi 2
#define flotantePin 14

WiFiServer server(80);

const char* ssid = "Personal-110";
const char* password = "AEKk6yXPtG";

uint8_t remoteMac[] = { 0x24, 0xA1, 0x60, 0x2C, 0xCC, 0x76 };

String header; 

int decisegundo = 0;
int webServerState = 0;
int lastState = -1;
int currentState = 0;

String Pagina = R"(
 <!DOCTYPE html>
<html>
<head>
  <meta charset='utf-8' />
  <title>Control bomba</title>
  <style>
    body {
      text-align: center; 
    }
    h1 {
      margin-top: 70px; /* Aumenta el espacio en la parte superior del título */
      margin-bottom: 155px; /* Espacio en la parte inferior del título */
      font-size: 100px; /* Tamaño del texto */
      color: #000000; /* Color del texto */
    }

    /* Estilos para el segundo encabezado (BOMBA AGUA) */
    h2 {
      font-size: 75px; /* Tamaño del texto */
      margin-bottom: 50px; /* Reduzco el margen inferior */
      color: #000000; /* Color del texto */
    }

    /* Estilos para los botones */
    button {
      font-size: 70px; /* Tamaño del texto en los botones */
      margin: 10px; /* Reduzco el espacio alrededor de los botones */
      background-color:#529EFF; /* Color de fondo de los botones */
      color: #000000; /* Color del texto en los botones */
      border: none; /* Elimina el borde de los botones */
      padding: 15px 30px;
      cursor: pointer; /* Cambia el cursor al pasar sobre los botones */
    }
  </style>
</head>
<body>
  <h1>Servidor Web</h1>

  <h2>BOMBA AGUA</h2>

  <a href='/on'><button style='height:225px;width:450px'>ON</button></a>

  <!-- Espacio adicional (H3) -->
  <H3> </H3>

  <!-- Botón OFF -->
  <a href='/off'><button style='height:225px;width:450px'>OFF</button></a>
</body>
</html>
)"; 

Ticker tic_ledWifi;

byte cont = 0;
byte max_intentos = 50;

void parpadeoLed() {
  byte estado = digitalRead(ledWifi);
  digitalWrite(ledWifi, !estado);
}

void setup() {
  pinMode(ledWifi, OUTPUT);
  pinMode(flotantePin, INPUT);

  Serial.begin(115200);
    Serial.println("\n");

  tic_ledWifi.attach(0.2, parpadeoLed);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED and cont < max_intentos) {
    cont++;
    delay(500);
    Serial.print(".");
  }

  Serial.println("");

  if (cont < max_intentos) {
    Serial.println("************");
    Serial.print("Conectado a la red WiFi: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("MacAdress: ");
    Serial.println(WiFi.macAddress());
    Serial.println("************");
    digitalWrite(ledWifi, LOW);
  } else {
    Serial.println("************");
    Serial.println("Error de conexión");
    Serial.println("************");
  }

  tic_ledWifi.detach();
  digitalWrite(ledWifi, HIGH);

  esp_now_init();
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_add_peer(remoteMac, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

  server.begin();

}

void loop() {

   while (decisegundo == 9000){
    int flotanteState = digitalRead(flotantePin); 
    uint8_t dataToSend = 0;
    currentState = 0;
    esp_now_send(remoteMac, &dataToSend, sizeof(dataToSend));
    Serial.println("esperando a que se llene el tanque con agua corriente");
    Serial.print ("currentState:");
    Serial.println ( currentState);
    delay(100);
   
  if (flotanteState == 0){
      decisegundo = 0;
    }
   }
  
  int flotanteState = digitalRead(flotantePin); 

  if (flotanteState == 1){
    currentState = 1;
    decisegundo ++;
    delay(100);
  }
  else if (webServerState == 1){
      currentState = 1;
      delay(100);
    }
    else {
      currentState = 0;
      decisegundo = 0;
    }

  if (currentState != lastState) {
    uint8_t dataToSend = currentState;
    esp_now_send(remoteMac, &dataToSend, sizeof(dataToSend));
    delay(10);

    lastState = currentState;
    Serial.print ("currentState:");
    Serial.println ( currentState);
  }

  WiFiClient client = server.available();   

  if (client) {                                
    String currentLine = "";                
    while (client.connected()) {            
      if (client.available()) {             
        char c = client.read();                               
        header += c;
        if (c == '\n') {                    
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            
            if (header.indexOf("GET /on") >= 0) {
              webServerState = 1;
              Serial.print("webServerState:");
              Serial.println( webServerState);
            } else if (header.indexOf("GET /off") >= 0) {
              webServerState = 0;
              Serial.print("webServerState:");
              Serial.println( webServerState);
            }
           
            client.println(Pagina);
            
            client.println();
            break;
          } else { 
            currentLine = "";
          }
        } else if (c != '\r') {  
          currentLine += c;      
        }
      }
    }
   
    header = "";
    
    client.stop();
    } 
  }

