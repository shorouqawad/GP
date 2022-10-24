/* IOT vertical farming*/


#include <WiFi.h>
#include <Wire.h>
#define WIFI_SSID "ProjectLabWirless"
#define WIFI_PASSWORD "ProjectLab204"


#include "FirebaseESP32.h"
#define FIREBASE_HOST "https://iot-vertical-farming-40e8a-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "AIzaSyADywmfNsRaT-9RnE1MXS24nS5ujfgseBg"
//Define Firebase Data object
FirebaseData firebaseData;

String control_path = "/control";
String plant_properties_path = "/setting_plant_properties";
String sensor_reading_path = "/sensor_reading";





#include "DHT.h"
#define DHTTYPE DHT22 // DHT 22 (AM2302), AM2321
//DHT Sensor;
uint8_t DHTPin = 4; //connected to GPIO 4 (Analog ADC1)
DHT dht(DHTPin, DHTTYPE);
float Temperature;
float Humidity;
float Temp_Fahrenheit;



#define tds_pin 36 //connected to GPIO 36 (Analog ADC0) 
int sensorValue = 0;
float tdsValue = 0;
float Voltage = 0;

const int big_pump1_spraying_pin = 23; //GPIO23

//water temperature 
#include <OneWire.h>
#include <DallasTemperature.h>

const int ds18b20_pin = 5; // ds18b20 is connected to GPIO 5

OneWire oneWire(ds18b20_pin);         // setup a oneWire instance
DallasTemperature tempSensor(&oneWire); // pass oneWire to DallasTemperature library

float tempCelsius;    // temperature in Celsius
float tempFahrenheit; // temperature in Fahrenheit


//water temperature  ends


void set_data_double_firebase(String complete_path, double value)
{
  Serial.println("------------------Set Sensor Data------------------");

  if (Firebase.setDouble(firebaseData,  complete_path, value))
  {
    Serial.println("------------------PASSED------------------" + complete_path);
  }
  else
  {
    Serial.println("------------------FAILED------------------" + complete_path);
    Serial.println("REASON: " + firebaseData.errorReason());
  }
}
void set_data_string_firebase(String complete_path, String value)
{
  Serial.println("------------------Set Sensor Data------------------");

  if (Firebase.setString(firebaseData,  complete_path, value))
  {
    Serial.println("------------------PASSED------------------" + complete_path);
  }
  else
  {
    Serial.println("------------------FAILED------------------" + complete_path);
    Serial.println("REASON: " + firebaseData.errorReason());
  }
}

String fetch_data;
String get_data_string_firebase(String complete_path)
{

  if (Firebase.getString(firebaseData, complete_path))
  {
    Serial.println("-----------------GET-------------------");
    fetch_data = firebaseData.stringData();
    Serial.println(fetch_data);
    return fetch_data;
  }
  else
  {
    Serial.println("-----------------FAILED-------------------");
    Serial.println("REASON: " + firebaseData.errorReason());
    return "error";
  }
}

int check_path_exit_firebase(String complete_path)
{
  Serial.println("------------------check_path_exit------------------");
  if (Firebase.pathExist(firebaseData, complete_path))
  {
    Serial.println("Path " + complete_path + " exists");
    return 1;
  }
  else
  {
    Serial.println("Path " + complete_path + " is not exist");
    return 0;
  }
}

int delete_node_firebase(String complete_path)
{
  if (check_path_exit_firebase(complete_path))
  {
    Firebase.deleteNode(firebaseData, complete_path);
    return 1;
  }
  else
  {
    Serial.println("error delete node");
    return 0;
  }

}

void setup()
{

  Serial.begin(115200);

  pinMode(DHTPin, INPUT);
  dht.begin();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  //Set database read timeout to 1 minute (max 15 minutes)
  Firebase.setReadTimeout(firebaseData, 1000 * 60);
  //tiny, small, medium, large and unlimited.
  //Size and its write timeout e.g. tiny (1s), small (10s), medium (30s) and large (60s).
  Firebase.setwriteSizeLimit(firebaseData, "tiny");




  pinMode(big_pump1_spraying_pin, OUTPUT);
  digitalWrite(big_pump1_spraying_pin, HIGH);
  // water temp setup
  tempSensor.begin();
}

String control_type;
String status_big_pump1_spraying;
String status_big_pump2_solution;
String status_light;
String status_small_pump1_acid;
String status_small_pump2_base;
String status_small_pump3_nutrient;


void loop()
{
  //----------------------------------------------dht
  Humidity = dht.readHumidity();
  Temperature = dht.readTemperature();
  Serial.print(F("Humidity: "));
  Serial.println(Humidity);
  Serial.print(F("%  Temperature: "));
  Serial.println(Temperature);
  if (isnan(Humidity) || isnan(Temperature))
  {
    Serial.println(F("Failed to read from DHT sensor!"));
  }
  else
  {
    set_data_double_firebase(sensor_reading_path + "/plant_id1/air_temperature",Temperature );
    set_data_double_firebase(sensor_reading_path + "/plant_id1/air_humidity", Humidity);
  }
  //----------------------------------------------


  //----------------------------------------------tds
  sensorValue = analogRead(tds_pin);
  Voltage = sensorValue * 5 / 1024.0; //Convert analog reading to Voltage
  tdsValue = (133.42 / Voltage * Voltage * Voltage - 255.86 * Voltage * Voltage + 857.39 * Voltage) * 0.5; //Convert voltage value to TDS value
  Serial.print("TDS Value = ");
  Serial.print(tdsValue);
  Serial.println(" ppm");
  //----------------------------------------------

  //------------test set
  // set_data_double_firebase(sensor_reading_path +"/plant_id1/water_temperature",22);
  set_data_double_firebase(sensor_reading_path + "/plant_id1/light_intensity", 22);
  set_data_double_firebase(sensor_reading_path + "/plant_id1/ph", 22);
  set_data_double_firebase(sensor_reading_path + "/plant_id1/tds", tdsValue);
  set_data_string_firebase(sensor_reading_path + "/plant_id1/time", "yy-mm-dd H:i:s");
  //------------

  //------------test get
  control_type = get_data_string_firebase(control_path + "/control_type");
  status_big_pump1_spraying = get_data_string_firebase(control_path + "/status_big_pump1_spraying");
  status_big_pump2_solution = get_data_string_firebase(control_path + "/status_big_pump2_solution");
  status_light = get_data_string_firebase(control_path + "/status_light");
  status_small_pump1_acid = get_data_string_firebase(control_path + "/status_small_pump1_acid");
  status_small_pump2_base = get_data_string_firebase(control_path + "/status_small_pump2_base");
  status_small_pump3_nutrient = get_data_string_firebase(control_path + "/status_small_pump3_nutrient");

  Serial.println("control_type =" + control_type);
  Serial.println("status_big_pump1_spraying =" + status_big_pump1_spraying);
  Serial.println("status_big_pump2_solution =" + status_big_pump2_solution);
  Serial.println("status_light =" + status_light);
  Serial.println("status_small_pump1_acid =" + status_small_pump1_acid);
  Serial.println("status_small_pump2_base =" + status_small_pump2_base);
  Serial.println("status_small_pump3_nutrient =" + status_small_pump3_nutrient);

  //------------

  if (String(status_big_pump1_spraying) == String("on"))
  {
    digitalWrite(big_pump1_spraying_pin, LOW);
    Serial.println("status_big_pump1_spraying ON");
  }
  else if (String(status_big_pump1_spraying) == String("off"))
  {
    digitalWrite(big_pump1_spraying_pin, HIGH);
    Serial.println("status_big_pump1_spraying OFF");
  }

  delay(100);
  //---------------------------------
  //water temp code start
  tempSensor.requestTemperatures();             // send the command to get temperatures
  tempCelsius = tempSensor.getTempCByIndex(0);  // read temperature in Celsius
  tempFahrenheit = tempCelsius * 9 / 5 + 32; // convert Celsius to Fahrenheit

  Serial.print("Temperature: ");
  Serial.print(tempCelsius);    // print the temperature in Celsius
  Serial.print("°C");
  Serial.print("  ~  ");        // separator between Celsius and Fahrenheit
  Serial.print(tempFahrenheit); // print the temperature in Fahrenheit
  Serial.println("°F");
  set_data_double_firebase(sensor_reading_path + "/plant_id1/water_temperature", tempCelsius);

  delay(500);
 //water temp code ends
}