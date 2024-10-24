#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <ESP32Time.h>
#include <ArduinoJson.h>

#include "lvgl.h"

#include "ui/ui.h"


#define ENABLE_GxEPD2_GFX 0
#include <GxEPD2_BW.h>

GxEPD2_BW<GxEPD2_579_GDEY0579T93, GxEPD2_579_GDEY0579T93::HEIGHT> display(GxEPD2_579_GDEY0579T93(/*CS=5*/ 45, /*DC=*/ 46, /*RST=*/ 47, /*BUSY=*/ 48)); // GDEY0579T93 792x272, SSD1683 (FPC-E004 22.04.13)

#define GxEPD2_DISPLAY_CLASS GxEPD2_BW
#define GxEPD2_DRIVER_CLASS GxEPD2_579_GDEY0579T93 // GDEY0579T93 792x272, SSD1683 (FPC-E004 22.04.13)


#define SCR_WIDTH 792
#define SCR_HEIGHT 272
#define LVBUF ((SCR_WIDTH * SCR_HEIGHT / 8) + 8 ) 

static lv_display_t *lvDisplay;
static uint8_t lvBuffer[2][LVBUF];

WiFiMulti wifiMulti;
DynamicJsonDocument apps(2048);
ESP32Time rtc(3600*3);


void my_disp_flush(lv_display_t *disp, const lv_area_t *area, unsigned char *data)
{
  int16_t width = area->x2 - area->x1 + 1;
  int16_t height = area->y2 - area->y1 + 1;
  display.drawImage((uint8_t *)data + 8, area->x1, area->y1, width, height);

  lv_display_flush_ready(disp);
}


static uint32_t my_tick(void)
{
    return millis();
}

void epd_setup(){

  SPI.begin(12, -1, 11, 45);
  display.init(115200, true, 2, false);
  if (display.pages() > 1)
  {
    delay(100);
    Serial.print("pages = "); Serial.print(display.pages()); Serial.print(" page height = "); Serial.println(display.pageHeight());
    delay(1000);
  }
  //display.clearScreen(); return;
  // first update should be full refresh
  delay(1000);
}

void getTime()
{
  HTTPClient http;
  http.begin("https://iot.fbiego.com/api/v1/time");
  int code = http.GET();
  String payload = http.getString();

  if (code == HTTP_CODE_OK){
    DynamicJsonDocument json(2048);
    deserializeJson(json, payload);
    uint64_t timestamp = json["timestamp"].as<uint64_t>();
    rtc.setTime(timestamp); // set the rtc time
  }
  http.end();
}

void getData(){
  HTTPClient http;
  http.begin("https://raw.githubusercontent.com/fbiego/google-play-scraper/refs/heads/master/console.json");
  int code = http.GET();
  String payload = http.getString();

  if (code == HTTP_CODE_OK){
    deserializeJson(apps, payload);
    
  }
  http.end();

}

void setup() {

  Serial.begin(115200);


  wifiMulti.addAP("__WIFI__", "__PASSWORD__");
  wifiMulti.addAP("__WIFI1__", "__PASSWORD1__");
  wifiMulti.addAP("__WIFI2__", "__PASSWORD2__");

  while(wifiMulti.run() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected: ");
  Serial.println(WiFi.SSID());
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());


  delay(500);

  getData();

  getTime();


  // Initialization settings, executed once when the program starts
  pinMode(7, OUTPUT);        // Set pin 7 to output mode
  digitalWrite(7, HIGH);     // Set pin 7 to high level to activate the screen power

  epd_setup();

  lv_init();

  lv_tick_set_cb(my_tick);

  lvDisplay = lv_display_create(SCR_WIDTH, SCR_HEIGHT);
  lv_display_set_flush_cb(lvDisplay, my_disp_flush);
  lv_display_set_buffers(lvDisplay, lvBuffer[0], lvBuffer[1], LVBUF, LV_DISPLAY_RENDER_MODE_PARTIAL);

  // lv_obj_t *label1 = lv_label_create(lv_scr_act());
  // lv_obj_set_align(label1, LV_ALIGN_CENTER);
  // lv_obj_set_width(label1, 400);
  // lv_label_set_text(label1, "Hello from Elecrow ePaper, LVGL 9 here!");
  // lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  // lv_obj_set_style_text_font(label1, &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_init();

  String tm = "Last Update: " + rtc.getTimeDate();
  lv_label_set_text(ui_Label16, tm.c_str());

  lv_obj_clean(ui_Panel1);

  for (JsonPair app : apps.as<JsonObject>()){
    JsonObject k = app.value().as<JsonObject>();

    // add_app(const char* name, const char* version, const char* category, float rate, int installs)
    add_app(k["title"].as<String>().c_str(), k["version"].as<String>().c_str(), 
    k["genre"].as<String>().c_str(), k["rate"].as<float>(), k["installs"].as<int>());

  }


}

void loop() {
  lv_timer_handler(); // Update the UI-
  delay(1000);
}
