/**********************************/
/*以範例CAMERAWEBSERVER下去修改*/
/*先給esp32cam餵人臉照片，最大number可到7個*/
/*餵飽之後，辨識成功電磁鎖開鎖，失敗不理人*/
/**********************************/

#include "esp_camera.h"
#include <WiFi.h>
#define CAMERA_MODEL_AI_THINKER
#define Relay 2
#define Red 13
#define Green 12
#include "AI_THINKER.h"

//SSID 密碼//
const char *ssid = "";
const char *password = "";

void startCameraServer();

boolean matchFace = false;
boolean activateRelay = false;
long prevMillis = 0;
int interval = 5000;

void setup()
{
    pinMode(Relay, OUTPUT);
    pinMode(Red, OUTPUT);
    pinMode(Green, OUTPUT);
    digitalWrite(Relay, LOW);
    digitalWrite(Red, HIGH); //起始狀態
    digitalWrite(Green, LOW);

    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.println();

    configSetup();

    sensor_t *s = esp_camera_sensor_get();
    // initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV3660_PID)
    {
        s->set_vflip(s, 1);       // flip it back
        s->set_brightness(s, 1);  // up the blightness just a bit
        s->set_saturation(s, -2); // lower the saturation
    }
    // drop down frame size for higher initial frame rate
    s->set_framesize(s, FRAMESIZE_QVGA);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");

    startCameraServer();

    Serial.print("Camera Ready! Use 'http://");
    Serial.print(WiFi.localIP());
    Serial.println("' to connect");
}

void loop()
{
    if (matchFace == true && activateRelay == false)
    {
       //辨識成功則開綠燈//
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
