//2/JAN/25  Marvin - Third Planet
#include <Arduino.h>//////NEWNEWNEW

#include <WiFi.h>
#include <HTTPClient.h>
#include <TFT_eSPI.h>
#include <time.h>
#include <math.h>

//Backlight
const uint8_t brightnessLevels[] = { 20, 60, 120, 180, 255 };
const uint8_t NUM_LEVELS = sizeof(brightnessLevels);
uint8_t brightnessIndex = 3;  // start fairly bright
#define BTN_BRIGHTNESS  0   // Left button
const int pwmChannel = 0;
#define BACKLIGHT_PIN 4
#define BACKLIGHT_CHANNEL 0
#define BACKLIGHT_FREQ 5000
#define BACKLIGHT_RES 8   // 8-bit (0–255)
#define BUTTON_PIN 0   // TTGO left front button controls screen brightness

// WiFi
const char* ssid = "YOUR_SSID"; // <<<<<<<<< EDIT  // "YOUR_SSID"
const char* password = "YOUR_SSID_PASSWORD";  //  <<<<<<<< EDIT  ?? "YOUR_SSID_PASSWORD"

// ECCC Current Weather SWOB  nav_canada/observation/atmospheric/surface_weather 
const char* CurrWXURL ="https://dd.weather.gc.ca/today/observations/swob-ml/latest/CYYC-MAN-swob.xml"; // CYYC  Calgary, AB
    //>>> GO HERE>> https://dd.weather.gc.ca/today/observations/swob-ml/latest/  ->> to obtain the the SWOB Weather feed for your local weather station or airport. 
    // Note: Not all data fields are availbe for all stations all the time.  Fields showing "--" may not be available for all fetches.
    // Note: I have noticed that the weather condtion codes are not always accurate from YYC.

    // ECCC Weather Alerts ATOM RSS feed
const char* ALERTS_URL = "https://weather.gc.ca/rss/battleboard/ab12_e.xml"; // CYYC  Calgary, AB
      //>>>> GO HERE>> https://weather.gc.ca/  ->> to obtain the the ALERT RSS feed for your local alerts
      //>>>>  Use the top right search bar and then scroll to the bottom.  Look for 'follow', the orange RSS feed icon, and select ALERTS

      // Timing
const unsigned long UPDATE_INTERVAL = 900000UL; // 15 min = 900000UL
// Environment Canada / NavCan usually updates weather information once per hour at the top of the hour.
// During unstable weather conditions and storms some MANUAL stations may update two or three times per hour.
const unsigned long ALERT_INTERVAL  = 1200000UL; //  20 minutes  1200000UL  
unsigned long lastUpdate = 0;
unsigned long lastAlertUpdate = 0;

// TFT
TFT_eSPI tft = TFT_eSPI();
int screenWidth;

// Weather values
String tempC="--", dewpoint="--", humidity="--";
String pressure="--", pressureSLP="--", windSpeed="--", windGust="--";
String windDir="--", windCardinal="--";
String visibility="--", cloudCeiling="--", vertVis="--";
String wxCode="--", wxText="--";
String obsUTC="--", obsLocal="--";
String conditionText = "--";
String stationName="----", stationId="----";

// Pressure trend
float prevPressure = NAN;
String pressureTrend = "→";

// Alerts
String alertTitle = "No alerts";
String alertSummary = "--";

// Scroll state
int scrollOffset = 0;
unsigned long lastScrollUpdate = 0;
const unsigned long SCROLL_INTERVAL = 250; // ms

// Prototypes
void connectWiFi();
void fetchECData();
void fetchAlerts();
void parseAlerts(const String&);
String extractSWOBValue(const String&, const String&);
String extractXmlValue(const String&, const String&);
String windDegToCardinal(float);
String decodeWeather(const String&);
String utcToLocal(const String&);
float windChill(float,float);
void drawDisplay();
void drawScrollingText(int x, int y, String text, int maxWidth);
String extractLabeledValue(const String &src, const String &label);

void showBootScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);

  tft.setTextSize(3);
  tft.drawString("Third", tft.width() / 2, 20);
  tft.drawString("Planet", tft.width() / 2, 50);
  tft.drawString("Weather", tft.width() / 2, 80);
  tft.drawString("Node", tft.width() / 2, 110);
  delay(1500);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("observation from:", tft.width() / 2, 140);
  tft.drawString("Environment Canada", tft.width() / 2, 150);
  tft.drawString("Nav Canada", tft.width() / 2, 160);
  delay(1500);
  tft.drawString("Firmware running", tft.width() / 2, 180);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  delay(1500);
  tft.drawString("Initializing...", tft.width() / 2, 220);
  delay(3500);
}

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(0);
  showBootScreen();  
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  screenWidth = tft.width();
  //Backlight
  pinMode(BTN_BRIGHTNESS, INPUT_PULLUP);
  ledcSetup(BACKLIGHT_CHANNEL, BACKLIGHT_FREQ, BACKLIGHT_RES);
  ledcAttachPin(BACKLIGHT_PIN, BACKLIGHT_CHANNEL);
  ledcWrite(BACKLIGHT_CHANNEL, 100);  // Start at medium low brightness

  connectWiFi();
  fetchECData();
  fetchAlerts();
  drawDisplay();
}

void handleBrightnessButton() {
  static bool lastState = HIGH;
  static unsigned long lastDebounce = 0;

  bool current = digitalRead(BTN_BRIGHTNESS);

  if (current != lastState) {
    lastDebounce = millis();
  }

  if ((millis() - lastDebounce) > 50) { 
    if (lastState == HIGH && current == LOW) {
      brightnessIndex++;
      if (brightnessIndex >= NUM_LEVELS) brightnessIndex = 0;

      ledcWrite(pwmChannel, brightnessLevels[brightnessIndex]);

      Serial.printf("Brightness: %d\n",
        brightnessLevels[brightnessIndex]);
    }
  }

  lastState = current;
}

uint16_t getAlertColor(const String &title) {
  String t = title;
  t.toUpperCase();
  //t.toLowerCase();

  if (t.indexOf("RED") >= 0) {
    return TFT_RED;
  }
   if (t.indexOf("ORANGE") >= 0) {
    return TFT_ORANGE;
  }
  if (t.indexOf("YELLOW") >= 0)  {
    return TFT_YELLOW;
  }
  if (t.indexOf("SPECIAL") >= 0) {
      return TFT_PINK;
  }

  return TFT_DARKGREY; // no alert
}

void loop() {
  unsigned long now = millis();

  if (now - lastUpdate > UPDATE_INTERVAL) {
    fetchECData();
    drawDisplay();
    lastUpdate = now;
  }

  if (now - lastAlertUpdate > ALERT_INTERVAL) {
    fetchAlerts();
    lastAlertUpdate = now;
  }

  delay(50); // minimal flicker

  //Brightness button
  static bool lastBtn = HIGH;
  bool btn = digitalRead(BTN_BRIGHTNESS);

  if (lastBtn == HIGH && btn == LOW) {
    brightnessIndex = (brightnessIndex + 1) % NUM_LEVELS;
    ledcWrite(BACKLIGHT_CHANNEL, brightnessLevels[brightnessIndex]);

    Serial.printf("Brightness level: %d\n",
                  brightnessLevels[brightnessIndex]);

    delay(250); 
  }
  lastBtn = btn;
}

////>>> New Improved wifi
void connectWiFi() {
 WiFi.begin(ssid, password);
  tft.setCursor(0, 0); tft.println("Connecting WiFi...\n\n");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    tft.println("WiFi connected\n\n");
    tft.println(WiFi.localIP().toString());
    tft.println("\n");
  tft.println(ssid);
    delay(1000);
  } else {
    tft.println("WiFi FAILED");
    delay(2000);
  }
}

// ----- EC SWOB -----
void fetchECData() {
  HTTPClient http;
  http.begin(CurrWXURL);
  int code = http.GET();
  Serial.printf("\nEC HTTP code: %d\n", code);

  if (code == 200) {
    String xml = http.getString();

    tempC       = extractSWOBValue(xml,"air_temp");
    dewpoint    = extractSWOBValue(xml,"dwpt_temp");
    humidity    = extractSWOBValue(xml,"rel_hum");
    pressure    = extractSWOBValue(xml,"stn_pres");
    pressureSLP = extractSWOBValue(xml,"mslp");
    windSpeed   = extractSWOBValue(xml,"avg_wnd_spd_10m_pst2mts");
    windGust    = extractSWOBValue(xml,"max_wnd_gst_spd_10m_pst10mts");
    windDir     = extractSWOBValue(xml,"avg_wnd_dir_10m_pst2mts");
    visibility  = extractSWOBValue(xml,"vis");
    cloudCeiling = extractSWOBValue(xml,"cld_bas_hgt_1");
    wxCode      = extractSWOBValue(xml,"prsnt_wx_1");
    obsUTC      = extractSWOBValue(xml,"date_tm");
    vertVis     = extractSWOBValue(xml,"vert_vis");
    stationName = extractSWOBValue(xml,"stn_nam");
    stationId   = extractSWOBValue(xml,"icao_stn_id");
    conditionText = wxCode;

    if (windDir != "--") windCardinal = windDegToCardinal(windDir.toFloat());
    if (obsUTC != "--") obsLocal = utcToLocal(obsUTC);
    wxText = decodeWeather(wxCode);

    // ---- SERIAL DEBUG ----
    Serial.println("Parsed EC data:");
    Serial.println("Station:" + stationName);
    Serial.println("StnID:" + stationId);
    Serial.println(" Temp: " + tempC);
    Serial.println(" Humi: " + humidity);
    Serial.println(" dew: " + dewpoint);
    Serial.println(" Press: " + pressure);
    Serial.println(" SLP: " + pressureSLP);
    Serial.println(" Vis: " + visibility);
    Serial.println(" Ceil: " + cloudCeiling);
    Serial.println("VertVis" + vertVis);
    Serial.println(" Wind: " + windSpeed);
    Serial.println(" Gust: " + windGust);
    Serial.println(" Wx code: " + wxCode + " (" + wxText + ")");
    Serial.println(" Local time: " + obsLocal);

    Serial.println("ALERT TEXT:");

    Serial.println("----------------------");
  }
  http.end();
}

// ----- ECCC Alerts -----
void fetchAlerts() {
  HTTPClient http;
  http.begin(ALERTS_URL);
  int code = http.GET();
  Serial.printf("\nAlerts HTTP code: %d\n", code);

  if (code == 200) {
    String xml = http.getString();
    Serial.println("Raw alerts XML (first 500 chars):");
    Serial.println(xml.substring(0, 500));
    parseAlerts(xml);
  }
  http.end();
}

void parseAlerts(const String& xml) {
  int pos = 0;
  String foundTitle = "No alerts";
  //String foundSummary = "--";

  while (true) {
    int start = xml.indexOf("<entry>", pos);
    if (start == -1) break;
    int end   = xml.indexOf("</entry>", start);
    if (end == -1) break;

    String entry = xml.substring(start, end);
    String title = extractXmlValue(entry,"title");
    String summary = extractXmlValue(entry,"summary");
    String area = extractXmlValue(entry,"cap:areaDesc");

    // Accept first alert found (any region)
if (title.length() > 0 && title != "No alerts") {
  foundTitle = title;
  break;
}
    pos = end + 8;
  }

  alertTitle = foundTitle;
  

  //scrollOffset = 0;

  Serial.println("Parsed Alert:");
  Serial.println(alertTitle);
  Serial.println(alertSummary);
}

// ----- Helpers -----

void drawWordWrappedText(int x, int y, int maxWidth, const String &text) {
  tft.setCursor(x, y);
  tft.setTextWrap(false);

  int spaceWidth = tft.textWidth(" ");
  int lineHeight = tft.fontHeight();

  int cursorX = x;
  int cursorY = y;

  int start = 0;
  while (start < text.length()) {
    int end = text.indexOf(' ', start);
    if (end == -1) end = text.length();

    String word = text.substring(start, end);
    int wordWidth = tft.textWidth(word);

    // Move to next line if word won't fit
    if (cursorX + wordWidth > x + maxWidth) {
      cursorX = x;
      cursorY += lineHeight;
      tft.setCursor(cursorX, cursorY);
    }

    tft.print(word);

    // Print space after word
    if (end < text.length()) {
      tft.print(" ");
      cursorX += wordWidth + spaceWidth;
    } else {
      cursorX += wordWidth;
    }

    start = end + 1;
  }
}

String extractSWOBValue(const String& xml, const String& tag) {
  String key = "name=\"" + tag + "\"";
  int i = xml.indexOf(key);
  if (i == -1) return "--";
  int v = xml.indexOf("value=\"", i);
  if (v == -1) return "--";
  v += 7;
  int e = xml.indexOf("\"", v);
  if (e == -1) return "--";
  return xml.substring(v, e);
}

String extractXmlValue(const String& xml, const String& tag) {
  String open = "<" + tag + ">";
  String close = "</" + tag + ">";
  int start = xml.indexOf(open);
  int end   = xml.indexOf(close);
  if (start == -1 || end == -1) return "--";
  start += open.length();
  return xml.substring(start,end);
}

String windDegToCardinal(float d) {
  const char* dirs[]={"N","NNE","NE","ENE","E","ESE","SE","SSE",
                      "S","SSW","SW","WSW","W","WNW","NW","NNW"};
  return dirs[int((d+11.25)/22.5)%16];
}

String utcToLocal(const String& utc) {
  struct tm tm{};
  if (!strptime(utc.c_str(), "%Y-%m-%dT%H:%M:%S", &tm)) return "--";

  // Force UTC
  setenv("TZ","UTC0",1);
  tzset();
  time_t t = mktime(&tm);

  // Calgary TZ
  setenv("TZ","MST7MDT,M3.2.0,M11.1.0",1);
  tzset();
  struct tm *lt = localtime(&t);

  char buf[32];
  strftime(buf,sizeof(buf),"%H:%M %Z",lt); // strftime(buf,sizeof(buf),"%H:%M %b %d %Z",lt); 
  return String(buf);
}

float windChill(float t,float w){
  if(t>10||w<4.8) return t;
  return 13.12+0.6215*t-11.37*pow(w,0.16)+0.3965*t*pow(w,0.16);
}

String decodeWeather(const String& c) {
  // ---- Clear / Cloud ----
  if (c=="0")   return "Clear";
  if (c=="1")   return "Mainly Clear";
  if (c=="2")   return "Partly Cloudy";
  if (c=="3")   return "Cloudy";
  if (c=="4")   return "Overcast";
  if (c=="125") return "Mostly Cloudy"; 

  // ---- Fog / Reduced Visibility ----
  if (c=="10")  return "Mist"; 
  if (c=="40")  return "Fog";
  if (c=="41")  return "Patches of Fog";
  if (c=="45")  return "Freezing Fog";
  if (c=="48")  return "Ice Fog";
  if (c=="28")  return "Haze";
  if (c=="27")  return "Smoke";
  if (c=="139")  return "Freezing Fog"; 

  // ---- Drizzle ----
  if (c=="50")  return "Light Drizzle";
  if (c=="51")  return "Drizzle";
  if (c=="52")  return "Heavy Drizzle";
  if (c=="156") return "Freezing Drizzle";
  if (c=="166") return "Hvy Frezng Drizl";

  // ---- Rain ----
  if (c=="60")  return "Light Rain";
  if (c=="61")  return "Rain";
  if (c=="62")  return "Heavy Rain";
  if (c=="81")  return "Rain Showers";
  if (c=="82")  return "Hvy Rain Shwrs";

  // ---- Freezing Rain ----
  if (c=="66")  return "Freezing Rain";
  if (c=="67")  return "Hvy Frezng Rain";
  if (c=="173") return "Ice Pellets";

  // ---- Snow ----
  if (c=="80")  return "Snow"; 
  if (c=="70")  return "Light Snow";
  if (c=="71")  return "Snow";
  if (c=="72")  return "Heavy Snow";
  if (c=="79")  return "Light Snow";
  if (c=="85")  return "Snow Showers";
  if (c=="86")  return "Hvy Snow Shwrs";
  if (c=="88")  return "Snow Grains";

  // ---- Blowing / Drifting Snow 
  if (c=="120") return "Blowing Snow";
  if (c=="121") return "Drifting Snow";
  if (c=="122") return "Hvy Blowing Snow";
  if (c=="132") return "Drifting Snow"; 

  // ---- Mixed Precip ----
  if (c=="68")  return "Rain & Snow";
  if (c=="69")  return "Frzng Rain & Snow";
  if (c=="83")  return "Rain & Snow Shwrs";

  // ---- Thunderstorms ----
  if (c=="95")  return "T-storm";
  if (c=="96")  return "T-storm w/ Rain";
  if (c=="99")  return "Severe T-storm";
  if (c=="184") return "T-storm";
  if (c=="185") return "T-storm w/ Rain";

  // ---- Dust / Sand -----
  if (c=="30")  return "Dust Storm";
  if (c=="31")  return "Blowing Dust";
  if (c=="33")  return "Sandstorm";

  // ---- Fallback ----
  if (c=="300")  return "N/A this station";

  return "unknown"; //return "unknown code " + c;
  
}

// ----- Display -----
void drawDisplay() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0,0);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.printf("%s",stationName.c_str());  // Weather Station Name
  tft.setCursor(0,12);
  tft.setTextSize(1);
  tft.printf("%s Obs @:%s\n\n",stationId.c_str(),obsLocal.c_str()); // Time stamp of weather data - if you edit the char buff - strftime above you can include the date
  tft.setCursor (0,30);
  tft.setTextSize(4);
  tft.printf("%s\n",tempC.c_str()); // Temperature
  tft.setTextSize(2);
  tft.setTextColor(TFT_CYAN); //CYAN
  drawWordWrappedText(0, 64, tft.width(),decodeWeather(conditionText).c_str()); // Weather Condtion text
  tft.printf("\n");
  tft.setTextSize(1);
  tft.setTextColor(TFT_PINK); 
  tft.printf("(%s) ",conditionText.c_str());  // weather condition code for debug = sometimes this data is inaccurate from ECCC
  tft.setCursor(0,96); 
  tft.setTextSize(2); 
  tft.setTextColor(TFT_BLUE);
  tft.printf("FL:%.0f\n",windChill(tempC.toFloat(),windSpeed.toFloat())); // Windchill - feel like temp
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1); 
  tft.printf("Humi: %s %%\n",humidity.c_str()); // humidity
  tft.printf("Vis: %s km\n",visibility.c_str());  //  visibility
  tft.printf("Wind: %s km/h <%s>\n",windSpeed.c_str(),windCardinal.c_str());  // avg windspeed + avg wind direction
  tft.printf("Gust: %s km/h\n",windGust=="MSNG"?"--":windGust.c_str()); // max wind gust when available
  tft.printf("cClng: %s m\n",cloudCeiling.c_str()); // cloud ceiling when available  
  tft.printf("vVis: %s m\n",vertVis.c_str()); // vertical visibility when available
  tft.printf("Dew: %s C\n",dewpoint.c_str()); // dewpoint
  tft.printf("Stnpres: %s hPa \n",pressure.c_str());   // Station barometric pressure before sea level altitude ajustment
  tft.printf("SLPres: %s hPa %s\n",pressureSLP.c_str(),pressureTrend.c_str()); // barometric pressure after sea level ajustment

  //EC Weather Alerts 
  tft.setCursor(0, 190);
  tft.setTextSize(1);
  uint16_t alertColor = getAlertColor(alertTitle);
  tft.setTextColor(alertColor);
  tft.println("       ALERT:");
  tft.println("- - - - - - - - - - -");
  drawWordWrappedText(0, 210, tft.width(),alertTitle.c_str());

}





