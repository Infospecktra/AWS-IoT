/*
ESP32 Version, just replace esp8266 library with WiFi.h library
 */

//#include "WiFi.h"         //needed for the ESP32
#include "ESP8266WiFi.h"  //needed for the ESP8266


//-----------------------------------------------
//This sketch is combined from Adafruit DHT sensor and tdicola for dht.h library
//by Stephen Borsay



//#include "DHT.h"  uncomment if using a DHT sensor with Adafruit DHT library
//#define DHTPIN 2    // what digital pin we're connected to  pin2 to D4 on esp board

// Uncomment whatever DHT sensor type you're using!
//#define DHTTYPE DHT11  // DHT 11
//#define DHTTYPE DHT21  // DHT 21
//#define DHTTYPE DHT22  // DHT 22

//DHT dht(DHTPIN,DHTTYPE);

long randomNumberT, randomNumberH; //randoms variables for temperature and humidity for testing
const char WEBSITE[] = "YOUR_HOST_HERE.mybluemix.net"; //Static IP at Node Red do not use http or https prefix

const char* MY_SSID = "YOUR_SSID HERE";
const char* MY_PWD =  "Your_WiFi_Password_here";


void setup()
{
  Serial.begin(115200);
 // dht.begin();
  Serial.print("Connecting to "+*MY_SSID);
  WiFi.begin(MY_SSID, MY_PWD);
  Serial.println("going into wl connect");

  while (WiFi.status() != WL_CONNECTED) //not connected,  ...waiting to connect
    {
      delay(1000);
      Serial.print(".");
    }
  Serial.println("wl connected");
  Serial.println("");
  Serial.println("Credentials accepted! Connected to wifi\n ");
  Serial.println("");
}


void loop()
{
  
  //Change loop delay as you see fit
  delay(5000); //5 seconds, adjust as you like relative to sampling rate vs. service call quota
/*
  float humidityData = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float celData = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float fehrData = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(humidityData) || isnan(celData) || isnan(fehrData))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Celsius (isFahreheit = false)
  float hicData = dht.computeHeatIndex(celData, humidityData, false);
  // Compute heat index in Fahrenheit (the default)
  float hifData = dht.computeHeatIndex(fehrData, humidityData);


  //Print to Serial monitor or Terminal of your chocice at 115200 Baud
  Serial.print("Humidity: ");
  Serial.print(humidityData);
  Serial.print(" %\t");
  Serial.print("Temperature in Cel: ");
  Serial.print(celData);
  Serial.print(" *C ");
  Serial.print("Temperature in Fehr: ");
  Serial.print(fehrData);
  Serial.print(" *F\t");
  Serial.print("Heat index in Cel: ");
  Serial.print(hicData);
  Serial.print(" *C ");
  Serial.print("Heat index in Fehr: ");
  Serial.print(hifData);
  Serial.print(" *F\n");
    */
  WiFiClient client;  //Instantiate WiFi object, can scope from here or Globally

    //Start or API service using our WiFi Client through PushingBox then relayed to Google
    if (client.connect(WEBSITE, 80))
      { 
         
         randomNumberT = random(10, 90);
         randomNumberH = random(5, 70);
         Serial.print(" sending data package now \n");
         //http URl format is --->   ?variable1=value&variable2=value2
         client.print("GET /YOUR_NODE_RED_URL_EXTENTION?Temperature=" + (String)randomNumberT
       + "&Humidity=" + (String) randomNumberH
  /*    + "&celData="      + (String) celData
       + "&fehrData="     + (String) fehrData
       + "&hicData="      + (String) hicData
       + "&hifData="      + (String) hifData */
         );

      // HTTP 1.1 provides a persistent connection, allowing multiple requests to be batched
      // or pipelined to an output buffer.  Careful when altering headers, they arnt forgiving!
      client.println(" HTTP/1.1"); 
      client.print("Host: ");
      client.println(WEBSITE);
      client.println("User-Agent: ESP32/1.0");
      client.println("Connection: close");
      client.println();
      Serial.print(" finished sending http headers \n");
      }
}
