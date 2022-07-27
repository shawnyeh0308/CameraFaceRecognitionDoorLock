/**********************************/
/*以範例CAMERAWEBSERVER下去修改*/
/*先給esp32cam餵人臉照片，最大number可到7個*/
/*餵飽之後，如果辨識成功電磁鎖開鎖，失敗不理人*/
/**********************************/

#include "esp_camera.h"
#include <WiFi.h>
#define CAMERA_MODEL_AI_THINKER
#define Relay 2
#define Red 13
#define Green 12
#include "AI_THINKER.h" //自創H檔，CAMERA_MODEL_AI_THINKER裡的接腳定義、各項參數和esp初始化

//SSID 密碼//
const char *ssid = "";
const char *password = "";

void startCameraServer(); //WEBSERVER函數

boolean matchFace = false;
boolean activateRelay = false;
long prevMillis = 0;
int interval = 5000;

void setup()
{   
    //起始狀態,繼電器和Led
    pinMode(Relay, OUTPUT);
    pinMode(Red, OUTPUT);
    pinMode(Green, OUTPUT);
    digitalWrite(Relay, LOW);
    digitalWrite(Red, HIGH); 
    digitalWrite(Green, LOW);

    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.println();

    configSetup();

    sensor_t *s = esp_camera_sensor_get();
    // 攝影鏡頭初始化
    if (s->id.PID == OV3660_PID)
    {
        s->set_vflip(s, 1);       // 鏡頭翻轉
        s->set_brightness(s, 1);  // 調高亮度
        s->set_saturation(s, -2); // 降低飽和度
    }
    // 下拉幀數大小以獲得更高的初始幀數速率
    s->set_framesize(s, FRAMESIZE_QVGA);
    //解析度 UXGA(1600x1200), SXGA(1280x1024), XGA(1024x768), SVGA(800x600), VGA(640x480), CIF(400x296)
    //QVGA(320x240), HQVGA(240x176), QQVGA(160x120), QXGA(2048x1564 for OV3660)


    WiFi.begin(ssid, password);
    //開始連接WIFI

    while (WiFi.status() != WL_CONNECTED) //連接未成功
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");

    startCameraServer(); //開啟WEBSERVER

    Serial.print("Camera Ready! Use 'http://");
    Serial.print(WiFi.localIP()); //取得WEBSERVER的IP位址
    Serial.println("' to connect");
}

void loop()
{
    if (matchFace == true && activateRelay == false)
    {
       //辨識成功則開綠燈，電磁鎖解鎖//
        activateRelay = true;
        digitalWrite(Relay, HIGH);
        digitalWrite(Green, HIGH);
        digitalWrite(Red, LOW);
        prevMillis = millis();
    }
    if (activateRelay == true && millis() - prevMillis > interval)
    {
        //辨識失敗紅燈不理人//
        activateRelay = false;
        matchFace = false;
        digitalWrite(Relay, LOW);
        digitalWrite(Green, LOW);
        digitalWrite(Red, HIGH);
    }
}
