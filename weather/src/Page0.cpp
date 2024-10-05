#include <Adafruit_Sensor.h>
#include "DHT.h"   // 包含DHT库
#include "Page0.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "OLED_font.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
extern uint8_t DHTPIN;

#define DHTTYPE DHT11   // 定义传感器类型为DHT11
DHT dht(DHTPIN, DHTTYPE); // 创建DHT传感器对象
extern Adafruit_SSD1306 display;
extern char firstDay_uvIndex[4], firstDay_humidity[4];

Page0::Page0()
{
    dht.begin(); // 初始化DHT11传感器
}

int cnt0 = 0;
float h, t;

void Page0::PMainShow(int xOffset)
{
    if (cnt0 >= 2)
    {
        cnt0 = 0;
        // 读取湿度和温度值
        h = dht.readHumidity(); // 读取湿度
        t = dht.readTemperature(); // 读取温度

        // 检查读取是否成功
        if (isnan(h) || isnan(t))
        {
            Serial.println("读取DHT11失败！"); // 如果读取失败，在串口监视器打印失败信息
            return;
        }
    }

    // 串口打印温湿度信息
    // Serial.printf("湿度: %.1f%% 温度: %.1f°C\n", h, t);  // 格式化输出湿度和温度

    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.drawBitmap(0 - xOffset, 0, shijBitmap, 16, 16,WHITE);
    display.drawBitmap(16 - xOffset, 0, jiBitmap, 16, 16,WHITE);
    display.drawBitmap(32 - xOffset, 0, wenBitmap, 16, 16,WHITE);
    display.drawBitmap(48 - xOffset, 0, duBitmap, 16, 16,WHITE);
    display.setCursor(64 - xOffset, 1);
    display.print(":");
    display.print(String(t).substring(0, 4));

    display.drawBitmap(0 - xOffset, 16, shijBitmap, 16, 16,WHITE);
    display.drawBitmap(16 - xOffset, 16, jiBitmap, 16, 16,WHITE);
    display.drawBitmap(32 - xOffset, 16, shiqqBitmap, 16, 16,WHITE);
    display.drawBitmap(48 - xOffset, 16, duBitmap, 16, 16,WHITE);
    display.setCursor(64 - xOffset, 17);
    display.print(":");
    display.print(String(h).substring(0, 2));
    display.print("%");


    display.drawBitmap(0 - xOffset, 32, yu_siBitmap, 16, 16,WHITE);
    display.drawBitmap(16 - xOffset, 32, baoBitmap, 16, 16,WHITE);
    display.drawBitmap(32 - xOffset, 32, shiqqBitmap, 16, 16,WHITE);
    display.drawBitmap(48 - xOffset, 32, duBitmap, 16, 16,WHITE);
    display.setCursor(64 - xOffset, 33);
    display.print(":");
    display.print(String(firstDay_humidity));
    display.print("%");

    display.drawBitmap(0 - xOffset, 48, ziBitmap, 16, 16,WHITE);
    display.drawBitmap(16 - xOffset, 48, waiBitmap, 16, 16,WHITE);
    display.drawBitmap(32 - xOffset, 48, xian_siBitmap, 16, 16,WHITE);
    display.setCursor(48 - xOffset, 49);
    display.print(":");
    display.print(String(firstDay_uvIndex));
    display.display();
    // delay(2000); // 等待2秒钟再次读取，DHT11的读取间隔建议不少于2秒
    display.clearDisplay();
}
