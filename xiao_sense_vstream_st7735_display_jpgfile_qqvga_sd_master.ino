//**********************************************************************************************************
//*    INET CAMERA using XIAO ESP32S3 SENSE  -- Support JPG PHOTO and Stream, it is able to save JPG PHOTO 
//                                              on builtin SD Card. 
//                                              Support TFT Display ST7735 QQVGA(128x240) as Monitor.
//                                              The camera can be controlled from the browser. 
//
//     First release on 5/2025, VER 0.55
//
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <TJpg_Decoder.h>
#include "esp_camera.h"
#include "FS.h"
#include "SD.h"
#include <Preferences.h>        // For permanent data
//#include <WiFi.h>
#include "WiFiMulti.h"
#include <WebServer.h>
#include <TimeLib.h>            // https://github.com/PaulStoffregen/Time
#include <esp_sntp.h>           // esp lib

#define CAMERA_MODEL_XIAO_ESP32S3 // Has PSRAM

#include "camera_pins.h"

#define TFT_MOSI 9   //
#define TFT_SCLK 7   //
#define TFT_CS   3   // Chip select control pin 
#define TFT_DC   4   // Data Command control pin
#define TFT_RST  2

#define SHOOT_PIN  1   // To take a photo
#define LED_BUILTIN 21 // same as SD control pin

//Adafruit_ST7735 tft = Adafruit_ST7735(&SPI, TFT_CS, TFT_DC, TFT_RST);
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
WiFiMulti wifiMulti;

int photo_Count = 1;             // File Counter
bool camera_ok = false;          // Check camera status
bool sd_ok = false;              // Check sd status
bool shoot_s = false;            // shoot on/off by browser
unsigned long ct2,pt2;  // interval time
//WiFi
String ssid = "ssid"; 
String password = "password";
String ssid2 =     "ssid2";      // WiFi 2, uncomment wifiMulti.addAP() below
String password2 = "password2";  // if you want to specify it
// web server
WebServer server(80);

String msg = "none"; // Response message
int stoken = 0;      // server token, count up 

// time
struct tm *tm;
int d_mon ;
int d_mday ;
int d_hour ;
int d_min ;
int d_sec ;
int d_year ;
//const long  gmtOffset_sec = 32400;

//byte readArray[40000]; // 40k  for SVGA
byte readArray[200000]; // 200k  for XGA

const int cbl = 20; // circular buffer length
String cb[cbl];     // circular buffer to store SD file name

bool web_authorised = false; // Login control

const char* STREAM_HEADER = 
  "HTTP/1.1 200 OK\r\n"
  "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n"
  "\r\n";

const char* FRAME_HEADER = 
  "--frame\r\n"
  "Content-Type: image/jpeg\r\n"
  "Content-Length: %d\r\n"
  "\r\n";

const char* JPG_HEADER = 
  "HTTP/1.1 200 OK\r\n"
  "Content-Type: image/jpeg\r\n"
  "Content-Length: %d\r\n"
  "\r\n";

Preferences preferences;  // to save parameters

// Web server
void handleRoot(void)
{
  String html;
  String val1;
  String val2;
  String val3;
  String val4;

  bool responsed = false;

  Serial.println("web received");
  msg = "";
  if (server.method() == HTTP_POST) { // submitted with string
    val1 = server.arg("take_photo");
    val2 = server.arg("pass_code");
    val3 = server.arg("log_in");
    val4 = server.arg("stoken");
    if (val4.length() != 0) { // server token
      Serial.print("stoken:");
      String s_stoken = server.arg("stoken");
      int t_stoken = s_stoken.toInt();
      Serial.println(s_stoken);
      msg = "stoken:" + s_stoken;
      if (stoken > t_stoken) {
        Serial.println("redirect");
        msg = "Post converted to Get";
        responsed = true;
        server.send(307, "text/html", "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><meta http-equiv=\"refresh\" content=\"0;url=/\"></head></html>");
      }
    }
  }
  if (!responsed) {
    if (val3.length() != 0 && val2.length() != 0 && val2.equals("123")) { // pass_code check. ### Change pass_code for security.###   
      Serial.println("authorised ok");
      msg = "authorised ok";
      web_authorised = true;
    }
    if (val3.length() != 0 && val2.length() == 0) { // log out.   
      Serial.println("logout ok");
      msg = "logout ok";
      web_authorised = false;
    }
   if (val1.length() != 0) {
        Serial.println("tk_p");
        shoot_s = true; 
        msg = "control take_photo";
    } else {
      //nop
    }
  }
  if (!responsed) {
    html = "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>Inet Camera</title>";
    html += "</head><body><p><h3>Inet Camera</h3> (ver 0.58)</p><form action=\"\" method=\"post\">";
    html += "<style>.lay_i input:first-of-type{margin-right: 20px;}</style>";
    html += "<style>.btn {width: 300px; padding: 10px; box-sizing: border-box; border: 1px solid #68779a; background: #cbe8fa; cursor: pointer;}</style>";
    html += "<p><input type=\"text\" id=\"pass_code\" name=\"pass_code\" size=\"15\" value=\"\">";
    html += "<button type=\"submit\" name=\"log_in\" value=\"log_set\">LOGIN/LOGOUT</button></p>";
    html += "<input type=\"hidden\" name=\"stoken\" value=\"";
    stoken += 1;
    html += stoken;
    html += "\">"; 
    if (web_authorised) {
      html += "<p><div class=\"lay_i\"><input type=\"submit\" name=\"take_photo\"  value=\"take a photo\" class=\"btn\"></div></p>";
      html += "<p>Response: " + msg + "</p>";
      html += "<p><a href='/stream'>Stream Video</a></p>";
      html += "<p><h3>Files:</h3></p>";
      File root = SD.open("/");
      bool isDir = false;
      String fname;
      int cbix = 0; // circular buffer index
      int fcnt = 0;
      while (true) {
        String filename = root.getNextFileName(&isDir);
        if (filename == "") break; // nomore files
        if (!isDir) { // not directory
          fname = filename.substring(1,28);
          cb[cbix] = fname;
          cbix++;
          fcnt++;
          if (cbix >= cbl) cbix = 0; // reset index
        }
      }
      Serial.printf("filecount: %d\n", fcnt);
      if (fcnt > 0) { //Are there Files?
        int rdix = (cbix - 20 + cbl) % cbl;
        for (int i = 0; i < 20; i++) {
          if (rdix >=  cbl) rdix = 0;
          html += "<p><a href='/photo?fname=" + cb[rdix] + "'>" + cb[rdix] + "</a></p>";
          rdix++;
        }
      } else {
        html += "<p>no files.</p>";
      }   
      root.close();
    }
    html += "</form></body>";
    html += "</html>";

    server.send(200, "text/html", html);
  }
  Serial.println("web send response");
}

void handleJPGStream() {
  WiFiClient client = server.client();
  if (!client.connected()) {
    Serial.println("Client disconnected");
    return;
  }
  Serial.println("Stream start");
  camera_fb_t * fb = NULL;
  sensor_t *s = esp_camera_sensor_get();
  //s->set_framesize(s, FRAMESIZE_SVGA);
  s->set_framesize(s, FRAMESIZE_XGA);
  delay(100);

  client.print(STREAM_HEADER);
  while (client.connected()) {
    //
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }
    client.printf(FRAME_HEADER, fb->len);
    client.write(fb->buf, fb->len);  //send JPEG data
    esp_camera_fb_return(fb);
    delay(66); // 15 fps
  }
  s->set_framesize(s, FRAMESIZE_QQVGA);
  Serial.println("Stream end");
  delay(66); 
}

void handlePhoto() {
  String val1;
  Serial.println("Photo start");
  val1 = server.arg(0);
  val1 = "/" + val1;
  Serial.println(val1);
  WiFiClient client = server.client();
  if (!client.connected()) {
    Serial.println("Client disconnected");
    return;
  }
  char tstr[32] = {'/n'};
  val1.toCharArray(tstr, 29);
  Serial.println(tstr);
  //File jpgfile = SD.open("/img20250523184944_00070.jpg", FILE_READ);
  File jpgfile = SD.open((char *)tstr, FILE_READ);
  long fsize = 0;
  if (jpgfile) { // open ok
    fsize = jpgfile.size();
    Serial.printf("filesize: %d\n", fsize);
    long i = 0;
    //if (fsize < 40000) {  // VGA
    if (fsize < 200000) {   // XGA
      while(jpgfile.available()) {
        byte rb = jpgfile.read();
        readArray[i++] = rb;
      }
      Serial.printf("readsize: %d\n", i);
      if (i!=0) {
        client.printf(JPG_HEADER, i);
        int j = i / 1000;
        int k = i % 1000;
        int m = 0;
        for (int n = 0; n < j ; n++) {
          client.write((uint8_t *)readArray + m, 1000);
          m = m + 1000;
        }
        if (k > 0) client.write((uint8_t *)readArray + m, k);
        client.print("\r\n");
      } else {
        server.send(200, "text/plain", "file size = 0");
      }

    } else {
      Serial.println("File size is too big");
    }
    jpgfile.close();
  } else {
    Serial.println("file open err");
    server.send(200, "text/plain", "file open err");
  }
  Serial.println("Photo end");
}
void handleNotFound(void)
{
  server.send(404, "text/plain", "Not Found.");
}

// Save pictures to SD card
void photo_save(const char * fileName) {
  camera_fb_t *fb;
  // Take a photo ,  warm-up exercises are necessary to get clear photo
  for (int i = 1; i < 10; i++) {
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Failed to get camera frame buffer");
      return;
    }
    esp_camera_fb_return(fb);
    delay(33);
  }
  // use last 1 shoot
  fb = esp_camera_fb_get();
  // Save photo to file
  writeFile(SD, fileName, fb->buf, fb->len);
  
  // Release image buffer
  esp_camera_fb_return(fb);

  Serial.println("Photo saved to file");
}

// SD card write file
void writeFile(fs::FS &fs, const char * path, uint8_t * data, size_t len){
    Serial.printf("Writing file: %s\r\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.write(data, len) == len){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
   // Stop further decoding as image is running off bottom of screen
  if ( y >= tft.height() ) return 0;
   tft.drawRGBBitmap(x, y, bitmap, w, h);
   // Return 1 to decode next block
   return 1;
}

bool set_camera(framesize_t f_size) {
  // Camera pinout
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = f_size;
  //config.frame_size = FRAMESIZE_240X240;
  //config.frame_size = FRAMESIZE_XGA;
  //config.frame_size = FRAMESIZE_QQVGA;  // 120x160
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  //config.pixel_format = PIXFORMAT_RGB565;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;
  
  if(config.pixel_format == PIXFORMAT_JPEG){
    if(psramFound()){
      Serial.println("PSRAM found");
      config.jpeg_quality = 12;  //  old value is 10, this value is higher
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
      // Limit the frame size when PSRAM is not available
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  } else {
    // Best option for face detection/recognition
    config.frame_size = FRAMESIZE_240X240;
    //config.frame_size = FRAMESIZE_QSXGA;
#if CONFIG_IDF_TARGET_ESP32S3
    config.fb_count = 2;
#endif
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return(false);
  }
  Serial.println("Camera ready");
  sensor_t *s = esp_camera_sensor_get();
  //s->set_vflip(s, 0);
  //s->set_framesize(s, FRAMESIZE_SVGA);
  s->set_framesize(s, FRAMESIZE_XGA);
  camera_ok = true; // Camera initialization check passes
  return(true);
}

void setup() {
  Serial.begin(115200);
  pinMode(SHOOT_PIN, INPUT_PULLUP);
  attachInterrupt(SHOOT_PIN,shoot_setting, FALLING); // to take a photo
  preferences.begin("photo_espcam", false);
  photo_Count = preferences.getInt("photo_no", -1);
  if (photo_Count < 0) photo_Count = 1;
  // WiFi
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(ssid.c_str(), password.c_str());  
  //wifiMulti.addAP(ssid2.c_str(), password2.c_str());  // uncomment if you want second wifi access point
  wifiMulti.run();   // It may be connected to strong one
  //WiFi.begin(ssid, password);
  int n = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(2000);
    Serial.println("Connecting to WiFi...");
    n++;
    if (n > 90) break; 
  }
  if (n <= 90) { 
    Serial.println("Connected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    //time set
    wifisyncjst();
  } else {
    Serial.println("No WiFi connection"); // JPG file name is wrong    
  }

  // camera init
  //if (!set_camera(FRAMESIZE_SVGA)){  // for 600x800 stream
  if (!set_camera(FRAMESIZE_XGA)){  // for 1600x1200 stream
    // nop 
  };
  /*
  if (!set_camera(FRAMESIZE_QQVGA)){  // for 120x160 stream 
    // nop;
  };*/
  sensor_t *s = esp_camera_sensor_get();
  //s->set_vflip(s, 0);
  s->set_framesize(s, FRAMESIZE_QQVGA); // to show on display 

  // Initialize SD card
  sd_ok = true; // sd initialization check passes
  if(!SD.begin(21)){  // cs pin, same as led
    Serial.println("Card Mount Failed");
    sd_ok = false;
  }
  uint8_t cardType = 99;
  if (sd_ok) {
    cardType = SD.cardType();
  }
  // Determine if the type of SD card is available
  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    sd_ok = false;
  }

  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
    Serial.println("MMC");
  } else if(cardType == CARD_SD){
    Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
    sd_ok = false;
  }

  // if sd_ok == true, sd initialization check passes

  //webserver
  server.on("/stream", HTTP_GET, handleJPGStream);
  server.on("/photo", HTTP_GET, handlePhoto);
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("Web server start");

  //tft
  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
  tft.initR(INITR_BLACKTAB);                //Init ST7735S 
  tft.fillScreen(ST77XX_BLACK);              
  tft.setRotation(1);                       
  tft.setTextSize(1);                      
  tft.setCursor(20, 50);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(2);
  if (n <= 90)  tft.println("Hello"); else tft.println("No WiFi!");
  TJpgDec.setJpgScale(1);
  TJpgDec.setCallback(tft_output);
  pt2 = millis();

  delay(2000); // you can see "Hello" on display

  Serial.println("Set up OK.");
}
void take_picture(){
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  if (!fb) {
      Serial.println("Camera capture failed");
      return;
  }
  //char tstr[60];
  //sprintf(tstr,"buf,len:%3d,%d",fb->buf,fb->len);
  //Serial.println(tstr);
  uint16_t w = 0, h = 0;
  TJpgDec.getJpgSize(&w, &h, fb->buf, fb->len);
  TJpgDec.drawJpg(0, 0, fb->buf, fb->len);

  esp_camera_fb_return(fb);
}   

void loop() {
  server.handleClient();
  take_picture();
  delay(66);  // 33:30 fps -> 66:15 FPS
  if (camera_ok && sd_ok) {
     // status ok
  } else {
    // bad status now
    digitalWrite(LED_BUILTIN, HIGH);  // turn the LED off (HIGH is the voltage level)
    delay(66);                      // wait for a bit
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED on by making the voltage LOW
    delay(66);                      // wait for a bit
  }
  // Camera & SD available, start taking pictures
  if(camera_ok && sd_ok && shoot_s){
    shoot_s = false; 
    char filename[32];
    sensor_t *s = esp_camera_sensor_get();
    //s->set_vflip(s, 0);
    //s->set_framesize(s, FRAMESIZE_SVGA);
    //s->set_framesize(s, FRAMESIZE_VGA);
    s->set_framesize(s, FRAMESIZE_XGA);
    s->set_exposure_ctrl(s, 1); // on
    s->set_aec2(s, 1);  // auto exposure on 
    s->set_ae_level(s, 2);  // max level 
    s->set_brightness(s, 1);  // max-1 level
    s->set_contrast(s, 1);  // max-1 level
    delay(66);
    // current time
    time_t t = time(NULL);
    tm = localtime(&t);
    d_mon  = tm->tm_mon+1;
    d_mday = tm->tm_mday;
    d_hour = tm->tm_hour;
    d_min  = tm->tm_min;
    d_sec  = tm->tm_sec;
    d_year = tm->tm_year;
    sprintf(filename, "/img%04d%02d%02d%02d%02d%02d_%05d.jpg", d_year + 1900, d_mon, d_mday, d_hour, d_min, d_sec, photo_Count);
    photo_save(filename);
    Serial.printf("Saved picture: %s\r\n", filename);
    photo_Count++;
    preferences.putInt("photo_no", photo_Count);
    s->set_framesize(s, FRAMESIZE_QQVGA);  // reset
    //
    tft.fillScreen(ST77XX_BLACK);              
    tft.setRotation(1);                       
    tft.setCursor(0, 50);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(1);
    //tft.println(filename);
    tft.printf("%.24s\r\n",filename);
    tft.setCursor(60, 70);
    tft.println("Saved");
    delay(3000);    
  }
}
void wifisyncjst() {
  // get jst from NTP server
  int lcnt = 0;
  configTzTime("JST-9", "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");
  delay(500);
  // get sync time
  while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET) {
    delay(500);
    lcnt++;
    if (lcnt > 100) {
      Serial.println("time not sync within 50 sec");
      break;
    }
  }
}
void shoot_setting(){
  ct2=millis();
  if ((ct2-pt2)>250) {
    shoot_s = true;  // toggle shoot on/off
  }
  pt2=ct2;
}
