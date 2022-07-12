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
#include "camera_pins.h"

//密碼//
const char *ssid = " ";
const char *password = " ";

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
    config.pixel_format = PIXFORMAT_JPEG;
    // init with high specs to pre-allocate larger buffers
    if (psramFound())
    {
        config.frame_size = FRAMESIZE_UXGA;
        config.jpeg_quality = 10;
        config.fb_count = 2;
    }
    else
    {
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }

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
