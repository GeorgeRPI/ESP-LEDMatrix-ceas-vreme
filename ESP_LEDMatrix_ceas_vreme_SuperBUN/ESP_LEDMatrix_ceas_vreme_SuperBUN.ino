// ---------------------------------------------------------
/* Ceas cu Ora serverNTP 24H Vara/Iarna.... cu vreme Nu afiseaza diacritice

* Presupunând că plăcile ESP sunt deja instalate. 
* Trebuie să utilizați această bibliotecă:- 
 * ESP8266WiFi at version 1.0
 * ArduinoJson at version 6.21.2
 * WiFiUdp
 * NTPClient
 * SPI
  Descărcați aceste biblioteci de la managerul bibliotecii.
 * ESP8266 + matrix LED7219
*/

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <SPI.h>
#include <Time.h>


WiFiClient client;

String weatherMain = "";
String weatherDescription = "";
String weatherLocation = "";
String country;
int humidity;
int pressure;
float temp;
float tempMin, tempMax;
int clouds;
float windSpeed;
String date;

String currencyRates;
String weatherString;

#define NUM_MAX 4
#define LINE_WIDTH 16
#define ROTATE 0

//for NodeMCU 1.0
#define DIN_PIN 13  // D7
#define CS_PIN  15  // D8
#define CLK_PIN 14  // D5

#include "max7219.h"
#include "fonts.h"

// Prototipul funcției (declarație înainte de utilizare)
int calculateLastSunday(int month, int year);

// Variabile globale
int currentMonth;
int currentYear;

// =======================================================================
// SCHIMBĂ-ȚI CONFIGURAȚIA AICI:
// =======================================================================
const char* ssid     = "Izolare";     // Nume retea wifi
const char* password = "alexandru";   // Parola retea WIFi
String weatherKey = "609f1174ea77b612982fa0ebd6fc86db"; // api key de la openweather.org 
String weatherLang = "&lang=en";
String cityID = "683506"; // ID - Bucuresti
//String cityID = "666253"; // ID - Stefan cel mare - OLT
// Citiți descrierea API OpenWeather pentru mai multe informații
// =======================================================================

// Inițializare WiFi și NTP
WiFiUDP udp;
NTPClient timeClient(udp, "0.ro.pool.ntp.org", 7200, 60000);  // UTC +0, poți schimba dacă ești într-o altă fus orar

// Variabile pentru controlul afișajului
bool displayOn = true;  // Pornit sau oprit

void printStringWithShift(String text, int delayTime);
void getWeatherData();
void getTime();
void updateTime();
void showSimpleClock();
void showDigit(char digit, int position, const uint8_t* font);

unsigned long previousMillis = 0;
const long interval = 1000; // 1 secundă

void setup() 
{
  Serial.begin(115200);

  // Inițializare SPI și MAX7219
  pinMode(CS_PIN, OUTPUT);
  SPI.begin();
  sendCmdAll(CMD_SHUTDOWN, 1); // Pornim afișajul


  initMAX7219();
  sendCmdAll(CMD_SHUTDOWN,1);
  displayOn = true; // Asigură-te că afișajul este pornit la început
  sendCmdAll(CMD_INTENSITY,1);  // Regraj intensitate Led MAX7219 de la 0 la 15
  Serial.print("Ma Conectez... ");
  WiFi.begin(ssid, password);
  printStringWithShift("Conectat la WiFi ",40);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Conectat: "); Serial.println(WiFi.localIP());

    // Inițializare NTPClient
  timeClient.begin();
  timeClient.setTimeOffset(0); // Important: Start with UTC+0

      // Așteptă sincronizarea
  while (!timeClient.update()) {
    timeClient.forceUpdate();
    delay(500);
    }

}
// =======================================================================
// Implementare funcție
int calculateLastSunday(int month, int year) {
    struct tm tm = {0};
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = 31;
    mktime(&tm);
    return tm.tm_mday - (tm.tm_wday % 7);
}
// =======================================================================

// Adaugă aici funcția updateTimeOffset()
void updateTimeOffset() {
    timeClient.update();
    int year = 1970;
    unsigned long rawTime = timeClient.getEpochTime();

// Convertim epoch time la data curentă
    int days = rawTime / 86400;
    while (days >= 365) {
        if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
            if (days >= 366) {
                days -= 366;
                year++;
            } else {
                break;
            }
        } else {
            days -= 365;
            year++;
        }
    }

    int month = 1;
    int dayOfMonth = days;
    int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
        daysInMonth[1] = 29; // Februarie are 29 zile în anii bisecți
    }

    for (int i = 0; i < 12; i++) {
        if (dayOfMonth < daysInMonth[i]) {
            month = i + 1;
            break;
        }
        dayOfMonth -= daysInMonth[i];
    }

    int weekday = (rawTime / 86400 + 4) % 7; // 1970-01-01 a fost joi (ziua 4)

// Calculează ultima duminică din martie
    int lastSundayMarch = 31 - ((weekday + (31 - dayOfMonth)) % 7);
// Calculează ultima duminică din octombrie
    int lastSundayOctober = 31 - ((weekday + (31 - dayOfMonth)) % 7);

// Verifică dacă suntem în intervalul de vară
    if ((month > 3 && month < 10) || (month == 3 && dayOfMonth >= lastSundayMarch) || (month == 10 && dayOfMonth < lastSundayOctober)) {
        timeClient.setTimeOffset(10800); // UTC+3 (Ora de vară)
    } else {
        timeClient.setTimeOffset(7200);  // UTC+2 (Ora standard)
    }
}

// =======================================================================

#define MAX_DIGITS 16
byte dig[MAX_DIGITS]={0};
byte digold[MAX_DIGITS]={0};
byte digtrans[MAX_DIGITS]={0};
int updCnt = 0;
int dots = 0;
long dotTime = 0;
long clkTime = 0;
int dx=0;
int dy=0;
byte del=0;
int h,m,s;

// =======================================================================

void loop() {
  timeClient.update();
  updateTime();

// Adaugă aici verificarea DST
  time_t epoch = timeClient.getEpochTime();
  struct tm *timeinfo = gmtime(&epoch);

// Debug output
  Serial.print("UTC Timp: ");
  Serial.print(timeClient.getHours());
  Serial.print(":");
  Serial.print(timeClient.getMinutes());
  Serial.print(":");
  Serial.println(timeClient.getSeconds());

  Serial.print("RO Timp: ");
  Serial.print(h);
  Serial.print(":");
  Serial.print(m);
  Serial.print(":");
  Serial.println(s);

//  Serial.print("Luna: ");
//  Serial.println(timeinfo->tm_mon + 1); // Current month (1-12)
  Serial.print("Data: ");
  Serial.print(timeinfo->tm_mday);
  Serial.print("/");
  Serial.print(currentMonth); // Folosește variabila globală
  Serial.print("/");
  Serial.println(currentYear); // Folosește variabila globală

  Serial.print("DST Timp: ");
  Serial.println(isDaylightSavingTime(currentMonth, timeinfo->tm_mday, timeClient.getHours()) ? "UTC+3 (vară)" : "UTC+2 (iarnă)");
  Serial.println("---------------------");

// Display on LED matrix
  showSimpleClock();

  if(updCnt<=0) { // la  10 scrolls, ~450s=7.5m
    updCnt = 3;
    Serial.println("Getting data ...");
    printStringWithShift("   Astept datele",40);
    getWeatherData();

//    getTime();
    Serial.println("Data loaded");
    clkTime = millis();
  }
 
  if(millis()-clkTime > 30000 && !del && dots) { // clock for 30s, then scrolls for about 30s
    printStringWithShift(date.c_str(),40);
    printStringWithShift(currencyRates.c_str(),35);
    printStringWithShift(weatherString.c_str(),40);
    updCnt--;
    clkTime = millis();
  }
  if(millis()-dotTime > 500) {
    dotTime = millis();
    dots = !dots;
  }
  updateTime();
  showSimpleClock();

// Oprește afișajul la ora 1:30
  if (h == 1 && m == 30 && displayOn) {
    Serial.println("Oprire afisaj!");
    sendCmdAll(CMD_SHUTDOWN, 0);
    displayOn = false;
    Serial.println("Afișaj OPRIT la ora 1:30");
  }

// Pornește afișajul la ora 06:00
  if (h == 6 && m == 00 && !displayOn) {
    Serial.println("Pornire afisaj!");
    sendCmdAll(CMD_SHUTDOWN, 1);
    displayOn = true;
    Serial.println("Afișaj PORNIT la ora 06:00");
  }

  delay(1000);  // Verifică la fiecare secunda
}

// =======================================================================

void showSimpleClock()
{
  dx=dy=0;
  clr();
  showDigit(h/10,  0, dig4x8);
  showDigit(h%10,  5, dig4x8);
  showDigit(m/10, 12,  dig4x8);
  showDigit(m%10, 17, dig4x8);
  showDigit(s/10, 24, dig3x5);
  showDigit(s%10, 28, dig3x5);
  setCol(10,dots ? B00100100 : 0);
  setCol(22,dots ? B00100100 : 0);
  refreshAll();
}

// =======================================================================

void showAnimClock()
{
  byte digPos[6]={0,5,12,17,22,28};
  int digHt = 12;
  int num = 6; 
  int i;
  if(del==0) {
    del = digHt;
    for(i=0; i<num; i++) digold[i] = dig[i];
    dig[0] = h/10 ? h/10 : 10;
    dig[1] = h%10;
    dig[2] = m/10;
    dig[3] = m%10;
    dig[4] = s/10;
    dig[5] = s%10;
    for(i=0; i<num; i++)  digtrans[i] = (dig[i]==digold[i]) ? 0 : digHt;
  } else
    del--;
  
  clr();
  for(i=0; i<num; i++) {
    if(digtrans[i]==0) {
      dy=0;
      showDigit(dig[i], digPos[i], dig3x8);
    } else {
      dy = digHt-digtrans[i];
      showDigit(digold[i], digPos[i], dig6x8);
      dy = -digtrans[i];
      showDigit(dig[i], digPos[i], dig6x8);
      digtrans[i]--;
    }
  }
  dy=0;
  setCol(10,dots ? B00100100 : 0);
  setCol(22,dots ? B00100100 : 0);
  refreshAll();
  delay(30);
}

// =======================================================================

void showDigit(char ch, int col, const uint8_t *data)
{
  if(dy<-8 | dy>8) return;
  int len = pgm_read_byte(data);
  int w = pgm_read_byte(data + 1 + ch * len);
  col += dx;
  for (int i = 0; i < w; i++)
    if(col+i>=0 && col+i<8*NUM_MAX) {
      byte v = pgm_read_byte(data + 1 + ch * len + 1 + i);
      if(!dy) scr[col + i] = pgm_read_byte(data + 1 + ch * len + 1 + i);
    }
}

// =======================================================================

void setCol(int col, byte v)
{
  if(dy<-8 | dy>8) return;
  col += dx;
  if(col>=0 && col<8*NUM_MAX)
    if(!dy) scr[col] = v; else scr[col] |= dy>0 ? v>>dy : v<<-dy;
}
byte reverseByte(byte b) {
  b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
  b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
  b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
  return b;
}

// =======================================================================

int showChar(char ch, const uint8_t *data)
{
  int len = pgm_read_byte(data);
  int i,w = pgm_read_byte(data + 1 + ch * len);
  for (i = 0; i < w; i++)
    scr[NUM_MAX*8 + i] = pgm_read_byte(data + 1 + ch * len + 1 + i);
  scr[NUM_MAX*8 + i] = 0;
  return w;
}

// =======================================================================
int dualChar = 0;

unsigned char convertPolish(unsigned char _c)
{
  unsigned char c = _c;
  if(c==196 || c==197 || c==195) {
    dualChar = c;
    return 0;
  }
  if(dualChar) {
    switch(_c) {
      case 133: c = 1+'~'; break; // 'ą'
      case 135: c = 2+'~'; break; // 'ć'
      case 153: c = 3+'~'; break; // 'ę'
      case 130: c = 4+'~'; break; // 'ł'
      case 132: c = dualChar==197 ? 5+'~' : 10+'~'; break; // 'ń' and 'Ą'
      case 179: c = 6+'~'; break; // 'ó'
      case 155: c = 7+'~'; break; // 'ś'
      case 186: c = 8+'~'; break; // 'ź'
      case 188: c = 9+'~'; break; // 'ż'
      //case 132: c = 10+'~'; break; // 'Ą'
      case 134: c = 11+'~'; break; // 'Ć'
      case 152: c = 12+'~'; break; // 'Ę'
      case 129: c = 13+'~'; break; // 'Ł'
      case 131: c = 14+'~'; break; // 'Ń'
      case 147: c = 15+'~'; break; // 'Ó'
      case 154: c = 16+'~'; break; // 'Ś'
      case 185: c = 17+'~'; break; // 'Ź'
      case 187: c = 18+'~'; break; // 'Ż'
      default:  break;
    }
    dualChar = 0;
    return c;
  }    
  switch(_c) {
    case 185: c = 1+'~'; break;
    case 230: c = 2+'~'; break;
    case 234: c = 3+'~'; break;
    case 179: c = 4+'~'; break;
    case 241: c = 5+'~'; break;
    case 243: c = 6+'~'; break;
    case 156: c = 7+'~'; break;
    case 159: c = 8+'~'; break;
    case 191: c = 9+'~'; break;
    case 165: c = 10+'~'; break;
    case 198: c = 11+'~'; break;
    case 202: c = 12+'~'; break;
    case 163: c = 13+'~'; break;
    case 209: c = 14+'~'; break;
    case 211: c = 15+'~'; break;
    case 140: c = 16+'~'; break;
    case 143: c = 17+'~'; break;
    case 175: c = 18+'~'; break;
    default:  break;
  }
  return c;
}

// =======================================================================

void printCharWithShift(unsigned char c, int shiftDelay) {
  c = convertPolish(c);
  if (c < ' ' || c > '~'+25) return;
  c -= 32;
  int w = showChar(c, font);
  for (int i=0; i<w+1; i++) {
    delay(shiftDelay);
    scrollLeft();
    refreshAll();
  }
}

// =======================================================================

void printStringWithShift(const char* s, int shiftDelay) {
    while (*s) {
        Serial.print(*s); // Verifică ce literă este trimisă
        printCharWithShift(*s, shiftDelay);
        s++;
    }
    Serial.println(); // Nouă linie după text
}

// =======================================================================
// Preluăm vremea de pe site-ul openweathermap.org
// =======================================================================

const char *weatherHost = "api.openweathermap.org";
String name;  // Declararea variabilei
float feels_like;  // Variabilă globală pentru temperatura resimțită

void getWeatherData() {
  Serial.print("connecting to "); Serial.println(weatherHost);
  
  if (client.connect(weatherHost, 80)) {
      client.print("GET /data/2.5/weather?id=");
      client.print(cityID);
      client.print("&units=metric&appid=");
      client.print(weatherKey);
      client.println(" HTTP/1.1");
      client.print("Host: ");
      client.println(weatherHost);
      client.println("Connection: close");
      client.println();
      
// Construim textul pentru afișaj
// weatherString = "Temperatura " + String(temp) + "°C  " + weatherDescription +
weatherString = "  " + name +
                "  Temperatura  " + String(temp) + " °C  " +
                "Temp.Resimtita  " + String(feels_like) + " °C  " + 
                "Umiditate  " + String(humidity) + " % " + 
                "Presiune  " + String(pressure) + " hPa " + 
                "Vant  " + String(windSpeed) + " m/s           ";
      Serial.println("Cerere trimisă către OpenWeatherMap.");
      Serial.println(weatherString);
// Afișează datele extrase
//    Serial.print("Descriere: "); Serial.println(weatherDescription);
  Serial.print("Temperatura: "); Serial.println(temp);
  Serial.print("Temp.Resimtita "); Serial.println(feels_like);    
  Serial.print("Umiditate: "); Serial.println(humidity);
  Serial.print("Presiune: "); Serial.println(pressure);
  Serial.print("Viteza vânt: "); Serial.println(windSpeed);
  Serial.println("Oras: " + name);
  } else {
  Serial.println("Conectarea la server a eșuat!");
  return;

}

String payload = "";
unsigned long timeout = millis();
while (client.available() || (millis() - timeout < 5000)) {
  if (client.available()) {
    char c = client.read();
    payload += c;
    timeout = millis();  // Resetează timeout-ul
  }
}

  client.stop();
  Serial.println("Server Response:");
  Serial.println(payload);

// Caută începutul JSON-ului (ignoră header-ele HTTP)
  int jsonIndex = payload.indexOf('{');
  if (jsonIndex == -1) {
    Serial.println("Eroare: JSON invalid!");
    return;
  }
  String jsonString = payload.substring(jsonIndex);

// Parsare JSON folosind ArduinoJson v6
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, jsonString);

  if (error) {
    Serial.print("Eroare la parsare JSON: ");
    Serial.println(error.f_str());
    return;
  }

// Extrage datele meteo
  name = doc["name"].as<String>();
  name = "Bucuresti"; // <-- Afiseaza Numele Orasului.. nu din OpenWeatherMap
//  weatherDescription = doc["weather"][0]["description"].as<String>();
  temp = doc["main"]["temp"];
  humidity = doc["main"]["humidity"];
  pressure = doc["main"]["pressure"];
  tempMin = doc["main"]["temp_min"];
  tempMax = doc["main"]["temp_max"];
  windSpeed = doc["wind"]["speed"];
  clouds = doc["clouds"]["all"];
  feels_like = doc["main"]["feels_like"];
}

// =======================================================================

bool isDaylightSavingTime(int month, int day, int utcHour) {
    if (month > 3 && month < 10) return true;
    if (month < 3 || month > 10) return false;
    
    int lastSunday = calculateLastSunday(month, currentYear); // Folosește variabila globală
    
    if (month == 3) {
        return (day > lastSunday) || (day == lastSunday && utcHour >= 1);
    } else {
        return (day < lastSunday) || (day == lastSunday && utcHour < 1);
    }
}

// =======================================================================

void updateTime() {
    timeClient.update();
    
    // Obține timpul UTC
    int utcHour = timeClient.getHours();
    m = timeClient.getMinutes();
    s = timeClient.getSeconds();
    
    // Obține data curentă
    time_t epoch = timeClient.getEpochTime();
    struct tm *timeinfo = gmtime(&epoch);
    
    // Actualizează variabilele globale
    currentMonth = timeinfo->tm_mon + 1; // Lunile încep de la 0
    currentYear = timeinfo->tm_year + 1900;
    int day = timeinfo->tm_mday;
    
    // Aplică offset-ul corect
    if (isDaylightSavingTime(currentMonth, day, utcHour)) {
        h = utcHour + 3; // UTC+3 (vară)
    } else {
        h = utcHour + 2; // UTC+2 (iarnă)
    }
    
    // Corecție overflow
    if (h >= 24) h -= 24;
    if (h < 0) h += 24;
}
// =======================================================================
