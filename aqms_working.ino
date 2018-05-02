/* 
Copyright (C) 2018 Siddharth Chandra
Email: siddharthchandragzb@gmail.com

*/

#include <SoftwareSerial.h>
#include <MQ135.h>
#include <DHT.h>
SoftwareSerial softSerial(2, 3);      // RX, TX pin for Arduino

#define DHTPIN A0                     // Analog pin for DHT11
#define DHTTYPE DHT11 

MQ135 gasSensor = MQ135(A1);
DHT dht(DHTPIN, DHTTYPE);

#define SSID "sid"                     // SSID - name of wifi (hotspot)
#define PASS "123456789"               // PASS - password required to access wifi (hotspot)
#define IP "184.106.153.149"           // ThingSpeak IP

float t;
float h;
float f;
float hi;
float air_quality;
String result;
int ledPin = 13;

void setup() 
{
   
   uint32_t baud = 115200;
   Serial.begin(baud);
   dht.begin();
   softSerial.begin(baud);
   pinMode(ledPin, OUTPUT);
   connectWiFi();
}

void loop() 
{
  delay(6000);

  Serial.println("DHT11 and MQ135 test!");
  
  air_quality = gasSensor.getPPM();
  h = dht.readHumidity();
  t = dht.readTemperature();
  f = dht.readTemperature(true);
  
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  hi = dht.computeHeatIndex(f, h);

  Serial.print("Humidity: "); 
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: "); 
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hi);
  Serial.println(" *F");
  Serial.print("Gas Level PPM : ");
  Serial.print(air_quality);
  Serial.println(" *PPM");

  if (air_quality<=700)
  {
    digitalWrite(ledPin, HIGH);
    delay(5000);
    digitalWrite(ledPin, LOW);
    result = "0";       //"Pure Air"
  }

  else if(air_quality<=1500 && air_quality>700)
  {
    result = "1";       //"Poor Air"
  }

  else if (air_quality>1500 )
  {
    result = "2";       //"Danger! Move to Fresh Air"
  }
  
  
  updateTS();

}

void updateTS()
{
    String cmd = "AT+CIPSTART=\"TCP\",\"";// Setup TCP connection
    cmd += IP;
    cmd += "\",80";
    sendDebug(cmd);
    delay(6000);

    String url = "GET /update?key=V8ICHOQB51BYJ8F4&field1="+String(t)+"&field2="+String(h)+"&field3="+String(hi)+"&field4="+String(f)+"&field5="+String(air_quality)+"&field6="+result+"\r\n\r\n\r\n\r\n\r\n\r\n";
    
    String stringLength="AT+CIPSEND=";
    
    stringLength +=String(url.length());

    Serial.println(stringLength);
    
    softSerial.println(stringLength);

   if( softSerial.find( ">" ) )
    {
      
     Serial.print(">");
     softSerial.print(url);
     Serial.print(url);
     delay(24000);
     
    }
    
   else
    {
     Serial.println("AT+CIPCLOSE Executing : ");
     sendDebug( "AT+CIPCLOSE" );    //close TCP connection
    }

    
}

void sendDebug(String cmd)
{
      Serial.print("SEND: ");
      softSerial.println(cmd);
      Serial.println(cmd);
}

boolean connectWiFi()
{
      softSerial.println("AT+CWMODE=1");      //Single mode of communication
      delay(6000);
      
      String cmd="AT+CWJAP=\"";               // Join accespoint (AP) with given SSID and PASS to be able to send data on cloud
      cmd+=SSID;
      cmd+="\",\"";
      cmd+=PASS;
      cmd+="\"";
      
      sendDebug(cmd);
      delay(6000);
      
      if(softSerial.find("OK"))
      {
       Serial.println("RECEIVED: OK");
       return true;
      }
      
      else
      {
       Serial.println("RECEIVED: Error");
       return false;
      }
}
