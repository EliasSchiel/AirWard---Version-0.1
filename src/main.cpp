#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_AHTX0.h>
#include <MHZ19.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>
#include <SPIFFS.h>

//Sensores utilizados: MQ7, AHT20, MH-Z19

// --- Credenciales y Servidor ---
const char* ssid = "Proximo a Editar";         
const char* password = "Proximo a Editar"; 
WebServer server(80);
String htmlCache = ""; 

// --- LED TEST 
#define PIN_LED_TEST              13
#define CFG_LED_TEST              pinMode(PIN_LED_TEST, OUTPUT)
#define PRENDER_LED_TEST          digitalWrite(PIN_LED_TEST, HIGH)
#define APAGAR_LED_TEST           digitalWrite(PIN_LED_TEST, LOW)
#define ESTADO_LED_TEST()         digitalRead(PIN_LED_TEST)

// --- Pantalla TFT
#define TFT_CS 5
#define TFT_DC 22
#define TFT_RST 4
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

#define SENSOR_MQ7 34
#define MHZ_RX 16
#define MHZ_TX 17

#define LEER_MQ7() analogRead(SENSOR_MQ7)
#define LEER_CO2() mhz19.getCO2()

// --- Objetos ---
Adafruit_AHTX0 aht;  
MHZ19 mhz19;
HardwareSerial mhzSerial(2);

// --- Tiempo ---
const unsigned long INTERVALO_LECTURA = 2000; 
const unsigned long INTERVALO_BLINK   = 500;  

unsigned long ultimoTiempoLectura = 0;

// Variables Globales
int valorGases = 0;
float temperatura = 0;
float humedad = 0;
int valorCO2 = 0;

void Blink_Test();
void ActualizarSensoresYPantalla();

void setup() {
  Serial.begin(115200);

  // Inicializar LED de Prueba
  CFG_LED_TEST;
  APAGAR_LED_TEST;

  // 1. Pantalla TFT
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK); 
  tft.setTextSize(2);

  // 2. Sensores
  Wire.begin(13, 14);
  aht.begin(&Wire);
  mhzSerial.begin(9600, SERIAL_8N1, MHZ_RX, MHZ_TX);
  mhz19.begin(mhzSerial);

  // 3. WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
  }

  // 4. Cargar HTML en RAM
  if (SPIFFS.begin(true)) {
    File file = SPIFFS.open("/index.html", "r");
    if (file) {
      htmlCache = file.readString();
      file.close();
    }
  }

  // 5. Rutas Web
  server.on("/", HTTP_GET, []() {
    if (htmlCache.length() > 0) {
      server.send(200, "text/html", htmlCache);
    } else {
      server.send(404, "text/plain", "Error cargando HTML");
    }
  });

  server.on("/data", HTTP_GET, []() {
    String json = "{";
    json += "\"mq7\":" + String(valorGases) + ",";
    json += "\"co2\":" + String(valorCO2) + ",";
    json += "\"temperatura\":" + String(temperatura, 1) + ",";
    json += "\"humedad\":" + String(humedad, 1);
    json += "}";
    server.send(200, "application/json", json);
  });

  server.begin();
  tft.fillScreen(ILI9341_BLACK);
}

void loop() {
  // 1. Atender el servidor web inmediatamente
  server.handleClient();

  // 2. Parpadeo del LED_TEST  //Prueba agregada para verificar que el loop se ejecuta correctamente
  Blink_Test();

  // 3. Tarea programada de sensores y pantalla (cada 2s)
  if (millis() - ultimoTiempoLectura >= INTERVALO_LECTURA) {
    ultimoTiempoLectura = millis();
    ActualizarSensoresYPantalla();
  }
}


void Blink_Test() {
  static unsigned long ultimoBlink = 0;

  
  if (millis() - ultimoBlink < INTERVALO_BLINK) return;

  ultimoBlink = millis();


  if (ESTADO_LED_TEST() == LOW) {
    PRENDER_LED_TEST;
  } else {
    APAGAR_LED_TEST;
  }
}

void ActualizarSensoresYPantalla() {
  // Lecturas
  valorGases = LEER_MQ7();
  valorCO2 = LEER_CO2();

  sensors_event_t humEvent, tempEvent;
  aht.getEvent(&humEvent, &tempEvent);
  temperatura = tempEvent.temperature;
  humedad = humEvent.relative_humidity;

  // Actualización pantalla tft con los datos de los sensores
  tft.setCursor(10, 20);  tft.print("Gases: "); tft.print(valorGases); tft.print("    ");
  tft.setCursor(10, 60);  tft.print("Temp:  "); tft.print(temperatura, 1); tft.print(" C  ");
  tft.setCursor(10, 100); tft.print("Hum:   "); tft.print(humedad, 1); tft.print(" %  ");
  tft.setCursor(10, 140); tft.print("CO2:   "); tft.print(valorCO2); tft.print(" ppm  ");
}