// Incluir las librerías necesarias
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// Definir los pines de los sensores y el buzzer
#define FLAME_PIN 4 // Sensor de flama
#define CO_PIN 36 // Sensor de monóxido
#define BUZZER_PIN 23 // Buzzer
#define DHT_PIN 15 // Sensor de temperatura y humedad

// Definir el tipo y el pin del sensor DHT
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

// Definir las credenciales de la red Wi-Fi
const char* ssid = "Politecnica";
const char* password = "";

// Definir las credenciales del servidor MQTT
const char* mqtt_server = "3.212.118.145";
const char* mqtt_user = "guest";
const char* mqtt_password = "guest";

// Definir los nombres de los topics MQTT
const char* flame_topic = "esp32.flame";
const char* co_topic = "esp32.co";
const char* temp_topic = "esp32.temp";
const char* hum_topic = "esp32.hum";

// Crear un objeto cliente MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// Función para conectar el ESP32 al Wi-Fi
void setup_wifi() {
  delay(10);
  // Mostrar el nombre de la red a la que se va a conectar
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  // Conectar al Wi-Fi
  WiFi.begin(ssid, password);

  // Esperar hasta que se establezca la conexión
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Mostrar la dirección IP asignada al ESP32
  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

// Función para reconectar el ESP32 al servidor MQTT si se pierde la conexión
void reconnect() {
  // Esperar hasta que se conecte al servidor MQTT
  while (!client.connected()) {
    Serial.print("Intentando conectar al servidor MQTT...");
    // Intentar conectar con el usuario y la contraseña definidos
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("conectado");
      // Publicar un mensaje de bienvenida en cada topic
      client.publish(flame_topic, "Hola desde ESP32");
      client.publish(co_topic, "Hola desde ESP32");
      client.publish(temp_topic, "Hola desde ESP32");
      client.publish(hum_topic, "Hola desde ESP32");
    } else {
      Serial.print("falló, rc=");
      Serial.print(client.state());
      Serial.println(" intentar de nuevo en 5 segundos");
      // Esperar 5 segundos antes de reintentar
      delay(5000);
    }
  }
}

// Función para leer los valores de los sensores y publicarlos en los topics correspondientes
void publish_sensor_data() {
  // Leer el valor analógico del sensor de flama
  int flame_value = analogRead(FLAME_PIN);
  // Convertir el valor a una cadena de caracteres
  char flame_str[10];
  sprintf(flame_str, "%d", flame_value);
  // Publicar el valor en el topic de flama
  client.publish(flame_topic, flame_str);

  // Leer el valor analógico del sensor de monóxido
  int co_value = analogRead(CO_PIN);
  // Convertir el valor a una cadena de caracteres
  char co_str[10];
  sprintf(co_str, "%d", co_value);
  // Publicar el valor en el topic de monóxido
  client.publish(co_topic, co_str);

  // Leer la temperatura y la humedad del sensor DHT
  float temp_value = dht.readTemperature();
  float hum_value = dht.readHumidity();
  
  // Comprobar si la lectura es válida
  if (isnan(temp_value) || isnan(hum_value)) {
    // Mostrar un mensaje de error si no se pudo leer el sensor
    Serial.println("Error al leer el sensor DHT");
  } else {
    // Convertir los valores a cadenas de caracteres
    char temp_str[10];
    char hum_str[10];
    sprintf(temp_str, "%.2f", temp_value);
    sprintf(hum_str, "%.2f", hum_value);
    // Publicar los valores en los topics de temperatura y humedad
    client.publish(temp_topic, temp_str);
    client.publish(hum_topic, hum_str);
  }

  // Activar el buzzer si el valor de flama o monóxido es mayor que un umbral
  if (flame_value > 500 || co_value > 500) {
    // Encender el buzzer
    digitalWrite(BUZZER_PIN, HIGH);
  } else {
    // Apagar el buzzer
    digitalWrite(BUZZER_PIN, LOW);
  }
}

// Definir las funciones setup() y loop()

void setup() {
  // Inicializar el puerto serie
  Serial.begin(115200);
  // Inicializar el sensor DHT
  dht.begin();
  // Configurar el pin del buzzer como salida
  pinMode(BUZZER_PIN, OUTPUT);
  // Conectar al Wi-Fi
  setup_wifi();
  // Configurar el servidor MQTT
  client.setServer(mqtt_server, 1883);
}

void loop() {
  // Reconectar al servidor MQTT si se pierde la conexión
  if (!client.connected()) {
    reconnect();
  }
  // Mantener la conexión con el servidor MQTT
  client.loop();
  // Publicar los datos de los sensores cada 10 segundos
  static unsigned long last_time = 0;
  unsigned long current_time = millis();
  if (current_time - last_time > 10000) {
    last_time = current_time;
    publish_sensor_data();
  }
}