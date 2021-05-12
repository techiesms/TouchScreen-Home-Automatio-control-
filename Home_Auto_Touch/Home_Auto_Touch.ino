#ifndef BOARD_HAS_PSRAM
#error "Please enable PSRAM !!!"
#endif

#include <Arduino.h>
#include <esp_task_wdt.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "epd_driver.h"
#include "logo.h"
#include "firasans.h"
#include <Wire.h>
#include <touch.h>
#include "lilygo.h"

#include <HTTPClient.h>
#include <WiFi.h>

#define TOUCH_INT   13
TouchClass touch;
uint8_t *framebuffer = NULL;

String Auth_token_board1 =  "Auth_Token_1";
String Auth_token_board2 =  "Auth_Token_2";
String auth_token_board;
String pin_number;
String value;


int cursor_x = 20;
int cursor_y = 60;

Rect_t area1 = {
  .x = 160,
  .y = 450,
  .width = 250,
  .height =  80
};
uint8_t state = 0;
uint8_t board = 1;
uint8_t buf[2] = {0xD1, 0X05};

const char* ssid     = "SSID";
const char* password = "PASS";

void setup()
{
  Serial.begin(115200);
  epd_init();

  pinMode(TOUCH_INT, INPUT_PULLUP);

  Wire.begin(15, 14);

  if (!touch.begin()) {
    Serial.println("start touchscreen failed");
    while (1);
  }
  Serial.println("Started Touchscreen poll...");


  framebuffer = (uint8_t *)ps_calloc(sizeof(uint8_t), EPD_WIDTH * EPD_HEIGHT / 2);
  if (!framebuffer) {
    Serial.println("alloc memory failed !!!");
    while (1);
  }
  memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);

  epd_poweron();
  epd_clear();
  // write_string((GFXfont *)&FiraSans, (char *)overview, &cursor_x, &cursor_y, framebuffer);

  //Draw Box
  epd_draw_rect(600, 450, 120, 60, 0, framebuffer);
  cursor_x = 615;
  cursor_y = 490;
  writeln((GFXfont *)&FiraSans, "1st", &cursor_x, &cursor_y, framebuffer);

  epd_draw_rect(740, 450, 120, 60, 0, framebuffer);
  cursor_x = 755;
  cursor_y = 490;
  writeln((GFXfont *)&FiraSans, "2nd", &cursor_x, &cursor_y, framebuffer);


  epd_draw_rect(300, 50, 200, 100, 0, framebuffer);
  cursor_x = 320;
  cursor_y = 110;
  writeln((GFXfont *)&FiraSans, "SW1 On", &cursor_x, &cursor_y, framebuffer);

  epd_draw_rect(510, 50, 200, 100, 0, framebuffer);
  cursor_x = 530;
  cursor_y = 110;
  writeln((GFXfont *)&FiraSans, "SW1 Off", &cursor_x, &cursor_y, framebuffer);

  epd_draw_rect(300, 200, 200, 100, 0, framebuffer);
  cursor_x = 320;
  cursor_y = 260;
  writeln((GFXfont *)&FiraSans, "SW2 On", &cursor_x, &cursor_y, framebuffer);

  epd_draw_rect(510, 200, 200, 100, 0, framebuffer);
  cursor_x = 530;
  cursor_y = 260;
  writeln((GFXfont *)&FiraSans, "SW2 Off", &cursor_x, &cursor_y, framebuffer);
  //    Rect_t area = {
  //      .x = 160,
  //      .y = 420,
  //      .width = lilygo_width,
  //      .height =  lilygo_height
  //    };
  //   // epd_copy_to_framebuffer(area, (uint8_t *) lilygo_data, framebuffer);
  //
  //    //epd_draw_rect(10, 20, EPD_WIDTH - 20, EPD_HEIGHT / 2 + 80, 0, framebuffer);

  epd_draw_grayscale_image(epd_full_screen(), framebuffer);
  epd_poweroff();

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  draw_switch_board1();
}




void loop()
{
  uint16_t  x, y;

  if (digitalRead(TOUCH_INT)) {
    if (touch.scanPoint()) {
      touch.getPoint(x, y, 0);
      y = EPD_HEIGHT - y;
      if ((x > 300 && x < 500) && (y > 50 && y < 150)) {
        state = 3;
        SW_control();
      } else if ((x > 510 && x < 710) && (y > 50 && y < 150)) {

        state = 4; SW_control();
      }
      else if ((x > 300 && x < 500) && (y > 200 && y < 300)) {

        state = 5; SW_control();
      } else if ((x > 510 && x < 710) && (y > 200 && y < 300)) {

        state = 6; SW_control();
      }
      else if ((x > 600 && x < 720) && (y > 450 && y < 510)) {
        board = 1;
        epd_clear_area(area1);
      } else if ((x > 740 && x < 860) && (y > 450 && y < 510)) {
        board = 2;
        epd_clear_area(area1);
      } else {
        return;
      }
      Serial.print(millis());
      Serial.print(":");
      Serial.println(state);
      epd_poweron();
      cursor_x = 20;
      cursor_y = 60;
      switch (board) {
        case 1:
          
          draw_switch_board1();
          break;
        case 2:
  
          draw_switch_board2();
          break;
        default:
          break;
      }
      epd_poweroff();
      while (digitalRead(TOUCH_INT)) {
      }
    }
  }
}

void SW_control()
{
  if ((WiFi.status()  == WL_CONNECTED)) {

    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    // configure traged server and url
    //http.begin("https://www.howsmyssl.com/a/check", ca); //HTTPS
    Serial.print("Board - "); Serial.println(board);
    Serial.print("state - "); Serial.println(state);

    if (board == 1)
    {
      if (state == 3 )
      {
        auth_token_board = Auth_token_board1;
        pin_number = "V1";
        value = "0";
      }
      if (state == 4)
      {
        auth_token_board = Auth_token_board1;
        pin_number = "V1";
        value = "1";
      }
      if (state == 5)
      {
        auth_token_board = Auth_token_board1;
        pin_number = "V2";
        value = "0";
      }
      if (state == 6 )
      {
        auth_token_board = Auth_token_board1;
        pin_number = "V2";
        value = "1";
      }
    }
    if (board == 2)
    {
      if (state == 3 )
      {
        auth_token_board = Auth_token_board2;
        pin_number = "V1";
        value = "0";
      }
      if (state == 4)
      {
        auth_token_board = Auth_token_board2;
        pin_number = "V1";
        value = "1";
      }
      if (state == 5)
      {
        auth_token_board = Auth_token_board2;
        pin_number = "V2";
        value = "0";
      }
      if (state == 6 )
      {
        auth_token_board = Auth_token_board2;
        pin_number = "V2";
        value = "1";
      }
    }

    http.begin("http://blynk-cloud.com/" + auth_token_board + "/update/" + pin_number + "?value=" + value); //HTTP
    Serial.println("http://blynk-cloud.com/" + auth_token_board + "/update/" + pin_number + "?value=" + value);
    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }
  Serial.println("sw1 On");
}

//void sw1_off()
//{
//  if ((WiFi.status()  == WL_CONNECTED)) {
//
//    HTTPClient http;
//
//    Serial.print("[HTTP] begin...\n");
//    // configure traged server and url
//    //http.begin("https://www.howsmyssl.com/a/check", ca); //HTTPS
//    http.begin("http://blynk-cloud.com/fAdFW1gKTybAqa8S21wUAuIFFMvZGyk8/update/V1?value=1"); //HTTP
//
//    Serial.print("[HTTP] GET...\n");
//    // start connection and send HTTP header
//    int httpCode = http.GET();
//
//    // httpCode will be negative on error
//    if (httpCode > 0) {
//      // HTTP header has been send and Server response header has been handled
//      Serial.printf("[HTTP] GET... code: %d\n", httpCode);
//
//      // file found at server
//      if (httpCode == HTTP_CODE_OK) {
//        String payload = http.getString();
//        Serial.println(payload);
//      }
//    } else {
//      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
//    }
//
//    http.end();
//  }
//  Serial.println("sw1 Off");
//}
//
//
//void sw2_off()
//{
//  if ((WiFi.status()  == WL_CONNECTED)) {
//
//    HTTPClient http;
//
//    Serial.print("[HTTP] begin...\n");
//    // configure traged server and url
//    //http.begin("https://www.howsmyssl.com/a/check", ca); //HTTPS
//    http.begin("http://blynk-cloud.com/fAdFW1gKTybAqa8S21wUAuIFFMvZGyk8/update/V2?value=1"); //HTTP
//
//    Serial.print("[HTTP] GET...\n");
//    // start connection and send HTTP header
//    int httpCode = http.GET();
//
//    // httpCode will be negative on error
//    if (httpCode > 0) {
//      // HTTP header has been send and Server response header has been handled
//      Serial.printf("[HTTP] GET... code: %d\n", httpCode);
//
//      // file found at server
//      if (httpCode == HTTP_CODE_OK) {
//        String payload = http.getString();
//        Serial.println(payload);
//      }
//    } else {
//      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
//    }
//
//    http.end();
//  }
//  Serial.println("sw2 Off");
//}
//
//void sw2_on()
//{
//  if ((WiFi.status()  == WL_CONNECTED)) {
//
//    HTTPClient http;
//
//    Serial.print("[HTTP] begin...\n");
//    // configure traged server and url
//    //http.begin("https://www.howsmyssl.com/a/check", ca); //HTTPS
//    http.begin("http://blynk-cloud.com/fAdFW1gKTybAqa8S21wUAuIFFMvZGyk8/update/V2?value=0"); //HTTP
//
//    Serial.print("[HTTP] GET...\n");
//    // start connection and send HTTP header
//    int httpCode = http.GET();
//
//    // httpCode will be negative on error
//    if (httpCode > 0) {
//      // HTTP header has been send and Server response header has been handled
//      Serial.printf("[HTTP] GET... code: %d\n", httpCode);
//
//      // file found at server
//      if (httpCode == HTTP_CODE_OK) {
//        String payload = http.getString();
//        Serial.println(payload);
//      }
//    } else {
//      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
//    }
//
//    http.end();
//  }
//  Serial.println("sw2 on");
//}

void draw_switch_board1()
{
  char *string1 = "Board1 Active";
  cursor_x = 160;
  cursor_y = 500;
  writeln((GFXfont *)&FiraSans, string1, &cursor_x, &cursor_y, NULL);
}

void draw_switch_board2()
{
  char *string1 = "Board2 Active";
  cursor_x = 160;
  cursor_y = 500;
  writeln((GFXfont *)&FiraSans, string1, &cursor_x, &cursor_y, NULL);
}
