#include <ArduinoJson.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <Wire.h> // Must include Wire library for I2C
#include <SparkFun_MMA8452Q.h> // Includes the SFE_MMA8452Q library

#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
/*
   This sample code demonstrates the normal use of a TinyGPS++ (TinyGPSPlus) object.
   It requires the use of SoftwareSerial, and assumes that you have a
   4800-baud serial GPS device hooked up on pins 4(rx) and 3(tx).
*/
static const int RXPin = 13, TXPin = 1;
static const uint32_t GPSBaud = 9600;
const int window=5;
float Acc1_max;
float Acc1_min;
float Acc2_max;
float Acc2_min;
int i;
int Acc1_max_index;
int Acc1_min_index;
int Acc2_max_index;
int Acc2_min_index;
float arrayAcc1[5];
float arrayAcc2[5];

MMA8452Q accel(0x1C);
MMA8452Q accel2(0x1D);
// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

// Set these to run example.
#define FIREBASE_HOST "biketeam2-93c1b.firebaseio.com"
#define FIREBASE_AUTH "NSSRveqD785HDX6BYM1LCpYk0OsPLV422VB0H6pf"
#define WIFI_SSID "Xperia"
#define WIFI_PASSWORD "12345678"
void setup()
{
  Serial.begin(115200);
  ss.begin(GPSBaud);
  Wire.begin(D2,D1);
  accel.init(SCALE_8G, ODR_50);
  accel2.init(SCALE_8G, ODR_50);
  //Begin wifi connecting
 WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  i=0;
}

void loop()
{
  static const double LONDON_LAT = 51.508131, LONDON_LON = -0.128002;

  //printInt(gps.satellites.value(), gps.satellites.isValid(), 5);
 // printFloat(gps.hdop.hdop(), gps.hdop.isValid(), 6, 1);
  printFloat(gps.location.lat(), gps.location.isValid(), 11, 6);
  printFloat(gps.location.lng(), gps.location.isValid(), 12, 6);
 //printInt(gps.location.age(), gps.location.isValid(), 5);
  printDateTime(gps.date, gps.time);
  //printFloat(gps.altitude.meters(), gps.altitude.isValid(), 7, 2);
  //printFloat(gps.course.deg(), gps.course.isValid(), 7, 2);
  //printFloat(gps.speed.kmph(), gps.speed.isValid(), 6, 2);
  //printStr(gps.course.isValid() ? TinyGPSPlus::cardinal(gps.course.deg()) : "*** ", 6);

  unsigned long distanceKmToLondon =
    (unsigned long)TinyGPSPlus::distanceBetween(
      gps.location.lat(),
      gps.location.lng(),
      LONDON_LAT, 
      LONDON_LON) / 1000;
  //printInt(distanceKmToLondon, gps.location.isValid(), 9);

  double courseToLondon =
    TinyGPSPlus::courseTo(
      gps.location.lat(),
      gps.location.lng(),
      LONDON_LAT, 
      LONDON_LON);

  //printFloat(courseToLondon, gps.location.isValid(), 7, 2);

  const char *cardinalToLondon = TinyGPSPlus::cardinal(courseToLondon);

 // printStr(gps.location.isValid() ? cardinalToLondon : "*** ", 6);

  //printInt(gps.charsProcessed(), true, 6);
  //printInt(gps.sentencesWithFix(), true, 10);
  //printInt(gps.failedChecksum(), true, 9);
  if (accel.available())
  {
    // First, use accel.read() to read the new variables:
    accel.read();
  }else{
    return;
    }
  if(accel2.available()){
     accel2.read();
    }else{
      return;
      }
  
 //Capture the maximum value and minimum value for each window
  float Acc1_x;
  float Acc1_y;
  float Acc1_z;
  float Acc2_x;
  float Acc2_y;
  float Acc2_z;

  Acc1_x=String(accel.cx,2).toFloat();
  Acc1_y=String(accel.cy,2).toFloat();
  Acc1_z=String(accel.cz,2).toFloat();
  Acc2_x=String(accel2.cx,2).toFloat();
  Acc2_y=String(accel2.cy,2).toFloat();
  Acc2_z=String(accel2.cz,2).toFloat();
  float Acc1=sqrt(sq(Acc1_x)+sq(Acc1_y)+sq(Acc1_z));
  float Acc2=sqrt(sq(Acc2_x)+sq(Acc2_y)+sq(Acc2_z));
  Acc1=String(Acc1,2).toFloat();
  Acc2=String(Acc2,2).toFloat();
  Serial.print(Acc1);
  Serial.print(",");
  Serial.println(Acc2);
  arrayAcc1[i]=Acc1;
  arrayAcc2[i]=Acc2;
  if(i==0){
    Acc1_max=arrayAcc1[i];
    Acc2_max=arrayAcc2[i];
    Acc1_min=arrayAcc1[i];
    Acc2_min=arrayAcc2[i];
    Acc1_max_index=0;
    Acc1_min_index=0;
    Acc2_max_index=0;
    Acc2_min_index=0;
    
    }else{
      if(arrayAcc1[i]>=Acc1_max){
        Acc1_max=arrayAcc1[i];
        Acc1_max_index=i;
        }
      if(arrayAcc1[i]<=Acc1_min){
        Acc1_min=arrayAcc1[i];
        Acc1_min_index=i;
        }

       if(arrayAcc2[i]>=Acc2_max){
        Acc2_max=arrayAcc2[i];
        Acc2_max_index=i;
        }

        if(arrayAcc2[i]<=Acc2_min){
        Acc2_min=arrayAcc2[i];
        Acc2_min_index=i;
        }
      }

      
  if(i==window-1){
        Acc1_max=String(Acc1_max, 2).toFloat();
        Acc1_min=String(Acc1_min, 2).toFloat();
        Acc2_max==String(Acc2_max, 2).toFloat();
        Acc2_min=String(Acc2_min, 2).toFloat();
        StaticJsonBuffer<100> jsonBuffer;
        JsonObject& obj = jsonBuffer.createObject();

        StaticJsonBuffer<100> jsonBuffer2;
        JsonObject& obj2 = jsonBuffer2.createObject();
        
        obj["latitude"]=gps.location.lat();
        obj["longitude"]=gps.location.lng();

        obj2["latitude"]=gps.location.lat();
        obj2["longitude"]=gps.location.lng();
        
        if(Acc1_max_index<Acc2_min_index&&Acc2_max_index<Acc2_min_index){
        Serial.print("max value of Acc1:");
        Serial.print(Acc1_max);
        Serial.print(",");
        Serial.print("max value of Acc2:");
        Serial.println(Acc2_max);

        Serial.print("min value of Acc1:");
        Serial.println(Acc1_min);
        Serial.print(",");
        Serial.print("min value of Acc2:");
        Serial.println(Acc2_min);
        
        obj["Acc1"]=Acc1_max;
        obj["Acc2"]=Acc2_max;
        String name=Firebase.push("bike2/location",obj);
        smartDelay(10);
        obj2["Acc1"]=Acc1_min;
        obj2["Acc2"]=Acc2_min;
        String name1=Firebase.push("bike2/location",obj2);
        }else if(Acc1_max_index<Acc2_min_index&&Acc2_max_index>Acc2_min_index){
        Serial.print("max value of Acc1:");
        Serial.print(Acc1_max);
        Serial.print(",");
        Serial.print("min value of Acc2:");
        Serial.println(Acc2_min);

        Serial.print("min value of Acc1:");
        Serial.print(Acc1_min);
        Serial.print(",");
        Serial.print("max value of Acc2:");
        Serial.println(Acc2_max);
        obj["Acc1"]=Acc1_max;
        obj["Acc2"]=Acc2_min;
        String name=Firebase.push("bike2/location",obj);
        smartDelay(10);
        obj2["Acc1"]=Acc1_min;
        obj2["Acc2"]=Acc2_max;
        String name1=Firebase.push("bike2/location",obj2);
        }else if(Acc1_max_index>Acc2_min_index&&Acc2_max_index<Acc2_min_index){
        Serial.print("min value of Acc1:");
        Serial.print(Acc1_min);
        Serial.print(",");
        Serial.print("max value of Acc2:");
        Serial.println(Acc2_max);

        Serial.print("max value of Acc1:");
        Serial.print(Acc1_max);
        Serial.print(",");
        Serial.print("min value of Acc2:");
        Serial.println(Acc2_min);
        
        obj["Acc1"]=Acc1_min;
        obj["Acc2"]=Acc2_max;
        String name=Firebase.push("bike2/location",obj);
        smartDelay(10);
        obj2["Acc1"]=Acc1_max;
        obj2["Acc2"]=Acc2_min;
        String name1=Firebase.push("bike2/location",obj2);
        
       }else{
        Serial.print("min value of Acc1:");
        Serial.print(Acc1_min);
        Serial.print(",");
        Serial.print("min value of Acc2:");
        Serial.println(Acc2_min);

        Serial.print("max value of Acc1:");
        Serial.print(Acc1_max);
        Serial.print(",");
        Serial.print("max value of Acc2:");
        Serial.println(Acc2_max);
        
        obj["Acc1"]=Acc1_min;
        obj["Acc2"]=Acc2_min;
        String name=Firebase.push("bike2/location",obj);
        smartDelay(20);
        obj2["Acc1"]=Acc1_max;
        obj2["Acc2"]=Acc2_max;
        String name1=Firebase.push("bike2/location",obj2);
        }
      
     
     
//      if(Firebase.failed()) {
//      Serial.print("pushing /logs failed:");
//      Serial.println(Firebase.error());  
//      return;
//      }
      
        float arrayAcc1=NULL;
        float arrayAcc2=NULL;
        i=0;
        Acc1_max=NULL;
        Acc2_max=NULL;
        smartDelay(20);
        }else{
          i++;
           smartDelay(40);
          }
 
  if (millis() > 5000 && gps.charsProcessed() < 10)
    Serial.println(F("No GPS data received: check wiring"));
}

// This custom version of delay() ensures that the gps object
// is being "fed".
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

static void printFloat(float val, bool valid, int len, int prec)
{
  if (!valid)
  {
    while (len-- > 1)
      Serial.print('*');
    Serial.print(' ');
  }
  else
  {
    Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i=flen; i<len; ++i)
      Serial.print(' ');
  }
  smartDelay(0);
}

static void printInt(unsigned long val, bool valid, int len)
{
  char sz[32] = "*****************";
  if (valid)
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i=strlen(sz); i<len; ++i)
    sz[i] = ' ';
  if (len > 0) 
    sz[len-1] = ' ';
  Serial.print(sz);
  smartDelay(0);
}

static void printDateTime(TinyGPSDate &d, TinyGPSTime &t)
{
  if (!d.isValid())
  {
    Serial.print(F("********** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d ", d.month(), d.day(), d.year());
    Serial.print(sz);
  }
  
  if (!t.isValid())
  {
    Serial.print(F("******** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d:%02d:%02d ", t.hour(), t.minute(), t.second());
    Serial.print(sz);
  }

  printInt(d.age(), d.isValid(), 5);
  smartDelay(0);
}

static void printStr(const char *str, int len)
{
  int slen = strlen(str);
  for (int i=0; i<len; ++i)
    Serial.print(i<slen ? str[i] : ' ');
  smartDelay(0);
}
