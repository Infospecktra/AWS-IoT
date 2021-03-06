/* ESP8266 AWS IoT
 *  
 * Simplest possible example (that I could come up with) of using an ESP8266 with AWS IoT.
 * No messing with openssl or spiffs just regular pubsub and certificates in string constants
 *
 * This is working as at 3rd Aug 2019 with the current ESP8266 Arduino core release:
 * SDK:2.2.1(cfd48f3)/Core:2.5.2-56-g403001e3=20502056/lwIP:STABLE-2_1_2_RELEASE/glue:1.1-7-g82abda3/BearSSL:6b9587f
 *
 * Author: Anthony Elder
 * License: Apache License v2
 *
 * Sketch Modified by Stephen Borsay for www.udemy.com/course/exploring-aws-iot/
 * https://github.com/sborsay
 *
 * For Device Shadow Subscriptions alter PubSub Librabry file: PubSubClient.h
 * MQTT_MAX_PACKET_SIZE : Maximum packet size
 * MQTT_MAX_PACKET_SIZE 128---> 256 or higher if using AWS Device Shadow for subscribtions on the device
 * EX: $aws/things/esp_511B/shadow/update/accepted and $aws/things/myDevice/shadow/get/accepted
 *
 * Add in EOF certificate delimiter
 * Add in Char buffer utilizing sprintf to dispatch JSON data to AWS IoT Core
 * First 9 chars of certs obfusicated, use your own, but you can share root CA / x.509 until revoked
 */
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
extern "C" {
#include "libb64/cdecode.h"
}

const char* ssid = "<YOUR-2.4GHz-WiFi>";
const char* password = "<YOUR-PASSWORD>";

// Find this awsEndpoint in the AWS Console: Manage - Things, choose your thing
// choose Interact, its the HTTPS Rest endpoint.  Endpoints ARE regional
const char* awsEndpoint = "<YOUR-ENDPOINT-ats.iot.<YOUR-REGION>.amazonaws.com";

// For the two certificate strings below paste in the text of your AWS
// device certificate and private key, comment out the BEGIN and END
// lines, add a quote character at the start of each line and a quote
// and backslash at the end of each line:

// xxxxxxxxxx-certificate.pem.crt
const String certificatePemCrt = \
//-----BEGIN CERTIFICATE-----
R"EOF(

XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTIwMDUxMjAwMDU0
MloXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAOlFTaW4OuK9C+/cBKns
jQNsyseHbP4C1+5Xji9a+Fvc3k+oWwDKXPswKz+hcuS6F2Cd0aAA8UghruwLTmSs
aGzJdPyRbeq+n6ICRQ4iGOkdq2EH0ot11lcHaib8lxg1jamNhAJhswJCyFlQjrln
HH2T6/GAyuE3NIui37N6ATusc0XeT+kgUGowQuWl9kuRJ2p3I7zLTHXtKS6fKCZ1
hmqIcCqhU3viNhletB1kKipUk85T1/yjDzm500HNOf/G4z8npV98VwjndBBbgt7B
/z4P74brP97bHcDSCiPbkwAu/UBth9MJPVTBNe0ug5QtppbXVyfwSEDMiInOtnwb
YwcCAwEAAaNgMF4wHwYDVR0jBBgwFoAUaSs3dQYUconS0a4gHuUEnRYHFUkwHQYD
VR0OBBYEFOvuPMsmqyF6Pf0bKL0zRkWahBSqMAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQAW2zFLlkSk4oHFy9mGytPT6jnQ
F8kAI7//aR4LyD5/Q9qjInQENQq5Gg1NAboXwabDqGUKYhNbX8EcLR338z77Sj7d
mHMvPF13AgH7a7cenYN7nC9UJ1OfuDdo5fO30f2LZckUHyjo+iHO9nHTj1ujadiD
J+uyGPujWoYTouYyQCsccMeeeyy04bLvvdqJgaOUzUwXIXO3ZAHQ4DQo55SuY5pC
zUAh6VBM1BIsinbq9wLQhIR1qCNJEaEROdHhUQHmnIqK+d9smWR0gY8RLdJZPiNm
rMo47WyiRtUQQfjhJHdMw6wdTwm9/KCanIESbft0PpPqv0BYlebgapqlX7jC

)EOF";
//-----END CERTIFICATE-----

// xxxxxxxxxx-private.pem.key
const String privatePemKey = \
//-----BEGIN RSA PRIVATE KEY-----
R"EOF(

XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXr4W9zeT6hb
AMpc+zArP6Fy5LoXYJ3RoADxSCGu7AtOZKxobMl0/JFt6r6fogJFDiIY6R2rYQfS
i3XWVwdqJvyXGDWNqY2EAmGzAkLIWVCOuWccfZPr8YDK4Tc0i6Lfs3oBO6xzRd5P
6SBQajBC5aX2S5EnancjvMtMde0pLp8oJnWGaohwKqFTe+I2GV60HWQqKlSTzlPX
/KMPObnTQc05/8bjPyelX3xXCOd0EFuC3sH/Pg/vhus/3tsdwNIKI9uTAC79QG2H
0wk9VME17S6DlC2mltdXJ/BIQMyIic62fBtjBwIDAQABAoIBAEatse4hWO40W3aM
knkt7Df6caJtHRAFqolTzAf7Jz8mN9Go8BHYPr35BVGdSGOXItgqWUaCopi5Y/ev
IUyNlIJJtAGGyugSY+nBiEzU9flD/RZEOX358TyjSX2QkPNL3qVd9H6IPaXUuEGf
j+8LF+3QsO+Wl2IPyQL6EY+eytraPE0Ja3U+rKiYYdFo13xoefBLAXqc60+yfrxW
QYmSOySVtExBqtu9CmPXlLHVAg71dAUoWOiuF8a4jmv/O/cLQGtyhZEzpwCleAgY
1J6lQP3uYtMsIHAjtT98kcQvz62lceaplJnEolhFK3R9Vqb1ekHYyx6HH3K7G5ro
lu3HE0ECgYEA/fO0KOkUdTtfO79mcLjgj1Mq489lwCAHmwJxtweBwYX+qqTHFssH
2CwSBFamFbka3B1kkXBo/4Nq+YH/8NNvebzrDZCe9wh2xEJ8Z8gkT7LZLUb17Ecn
NGBvZ/q528b376ZGLPLG7efKuPO/bm3k9byJI37yxJxzN4tpR9YtWecCgYEA6ybm
8GSrBjaniRryMiGwu0A+fyfOVjgXrLDGD6U24Ggx8EyzsB9Gm6Sw8TLmGT0VOFWW
vn8Jv52M/TIR2NB0wvv9vlIQzJ1OXzU3jMCnDNRIKpeJFrzyR2wlMu4alox/N2xK
QC9I+VvSj6wilB7rvEVjI548LvSEp9KFiSzvyeECgYAYZocj+yk9DmRbIEveftUM
0Gfpla7oI+u3KCf29yCvkIlkqgp7FSz+x1U/13eD1NJQ3f4gv1VHX8AykxwdbqNi
ZhWLqx6rC8QlnWHIswFSKxQU9qGAf2Gz9aQesIeVJ2wBOc+6V+VnTIM8/CajnKmI
NwJSR8tZutKTPrDx9xFKdwKBgBct0n3HroQCRylgmpcOcxlCD+cAyXn+NXc0RZGt
O2eLXcDODrhBnPgil75uwfei6cCPzzgf1ZopsYgTs2anxZqitDRaaoWAzATefgdF
xjGhiyuHnF+9aDRaGKCYj85fMvsHlZoWDx9lSbAXy6DGG36HaGpOYqhOOfVA7yDC
aTCBAoGBAMJjaEDxxn3PdhLeepS6AgCcQyV3BeXrdRdiJSy7V3y2/Aywm3yYuzqR
XcGj5pTJUO3cCz/Lh3PyoQK7c4eZ2YLdSn35J27OjmQa2EQVlz9VQC7FrAQgZ/hy
dsslVtrgGnge/WemraqW9hgjZFPBVHBSmE3zUYO6uYQJBwtGvtx/

)EOF";
//-----END RSA PRIVATE KEY-----

// This is the AWS IoT CA Certificate from:
// https://docs.aws.amazon.com/iot/latest/developerguide/managing-device-certs.html#server-authentication
// This one in here is the 'RSA 2048 bit key: Amazon Root CA 1' which is valid
// until January 16, 2038 so unless it gets revoked you can leave this as is:
const String caPemCrt = \
//-----BEGIN CERTIFICATE-----
"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF" \
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6" \
"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL" \
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv" \
"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj" \
"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM" \
"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw" \
"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6" \
"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L" \
"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm" \
"jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC" \
"AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA" \
"A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI" \
"U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs" \
"N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv" \
"o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU" \
"5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy" \
"rqXRfboQnoZsG4q5WTP468SQvvG5";
//-----END CERTIFICATE-----

WiFiClientSecure wiFiClient;
void msgReceived(char* topic, byte* payload, unsigned int len);
PubSubClient pubSubClient(awsEndpoint, 8883, msgReceived, wiFiClient);

void setup() {
  Serial.begin(115200); Serial.println();
  Serial.println("ESP8266 AWS IoT Example");

  Serial.print("Connecting to "); Serial.print(ssid);
  WiFi.begin(ssid, password);
  WiFi.waitForConnectResult();
  Serial.print(", WiFi connected, IP address: "); Serial.println(WiFi.localIP());

  // get current time, otherwise certificates are flagged as expired
  setCurrentTime();

  uint8_t binaryCert[certificatePemCrt.length() * 3 / 4];
  int len = b64decode(certificatePemCrt, binaryCert);
  wiFiClient.setCertificate(binaryCert, len);
 
  uint8_t binaryPrivate[privatePemKey.length() * 3 / 4];
  len = b64decode(privatePemKey, binaryPrivate);
  wiFiClient.setPrivateKey(binaryPrivate, len);

  uint8_t binaryCA[caPemCrt.length() * 3 / 4];
  len = b64decode(caPemCrt, binaryCA);
  wiFiClient.setCACert(binaryCA, len);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); //turn LED off, active low
}

unsigned long lastPublish;
int msgCount;

void loop() {

  pubSubCheckConnect();

  //Add a JSON package of fake data to deliver to AWS IoT
  //Uses snprintf but other viable options are: sprintf, strcpy, strncpy, or
  //Use the ArduinoJson library for Efficient JSON serialization
  char fakeData[128];
//choose your own number of variables and data types, but avoid data buffer size overload
  float Tem =  random(30,110); //fake number range, adjust as you like
  float Hum =  random(40,95);
  char *  Mes = "empty";

sprintf(fakeData,"{\"state\": {\"reported\": {\"uptime\": %lu, \"temperature\": %f, \"humidity\": %f, \"message\": \"%s\" }}}",millis()/1000,Tem,Hum,Mes);

  if (millis() - lastPublish > 10000) {

   pubSubClient.publish("$aws/things/<YOUR-THING>/shadow/update", fakeData);
   Serial.print("Published: "); Serial.println(fakeData);

   if(Tem > 90) //send off temp topic alert if high temperature
   {
       char fakeTemp[128];
       sprintf(fakeTemp,"{\"state\": {\"reported\": {\"temperature\": %f}}}",Tem);
       pubSubClient.publish("alert/highTemp",fakeTemp);
       Serial.print("Published: "); 
       Serial.println(fakeTemp); 
       
  for (int i = 0; i < 5; i++)
   {
        digitalWrite(LED_BUILTIN, LOW);
        delay(100);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
     }//end for
  
   } //end temp topic

   
    lastPublish = millis();
  }
}

void msgReceived(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received on "); Serial.print(topic); Serial.print(": ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void pubSubCheckConnect() {
  if ( ! pubSubClient.connected()) {
    Serial.print("PubSubClient connecting to: "); Serial.print(awsEndpoint);
    while ( ! pubSubClient.connected()) {
      Serial.print(".");
      pubSubClient.connect("<YOUR-THING-NAME>"); //publisher and subscriber should have DIFFERENT ClientID here.
    }
    Serial.println(" connected");
    pubSubClient.subscribe("inTopic");
   
  }
  pubSubClient.loop();
}

int b64decode(String b64Text, uint8_t* output) {
  base64_decodestate s;
  base64_init_decodestate(&s);
  int cnt = base64_decode_block(b64Text.c_str(), b64Text.length(), (char*)output, &s);
  return cnt;
}

void setCurrentTime() {
  configTime(3 * 3600, 0,"pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: "); Serial.print(asctime(&timeinfo));
}

