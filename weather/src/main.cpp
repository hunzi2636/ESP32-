#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "OLED_font.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <complex.h>
#include <Page0.h>
#include "Page1.h"
#include <Page2.h>
#include <Page3.h>
#include <Page4.h>
#include <SPIFFS.h>
#include <WebServer.h>


// const char* ssid = "ZTE";
// const char* password = "1233211234567";
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     4
#define OLED_SCL       17
#define OLED_SDA       5
uint8_t DHTPIN = 22; // 定义DHT11数据引脚连接到ESP32的GPIO14
const int buttonPin = 23; // 按键连接的引脚
const int outputPin = 2; // 输出高低电平的引脚


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Page0 page0;
static Page1 page1;
Page2 page2;
Page3 page3;
Page4 page4;
const char* ssid = "ZhangDeXiaoNaoZhong";
const char* password = "12345678";
extern int httpcode1;
extern int httpcode2;
WebServer server(80);
bool timeQuest = false;
int page = 1; //1是时钟页面
bool isConnectedDevice = false;

// Web服务器任务
void webServerTask(void* pvParameters)
{
    // 设置HTTP服务器路由
    server.on("/", HTTP_GET, []()
    {
        String html = "<h1 style='text-align:center;'>WiFi连接设置</h1>";

        // 读取保存的WiFi配置
        String savedSSID;
        String savedPassword;
        File file = SPIFFS.open("/config.txt", "r");
        if (file)
        {
            savedSSID = file.readStringUntil('\n'); // 读取SSID
            savedPassword = file.readStringUntil('\n'); // 读取密码
            file.close();
            html += "<p style='font-size:50px; display:block; margin:auto;text-align:center;'>当前保存的WiFi配置:</p>";
            html += "<p style='font-size:50px; display:block; margin:auto;text-align:center;'>SSID: " + savedSSID +
                "</p>";
            html += "<p style='font-size:50px; display:block; margin:auto;text-align:center;'>密码:" + savedPassword +
                "</p>";
        }
        else
        {
            html += "<p>未找到保存的WiFi配置。</p>";
        }

        html +=
            "<button style='font-size:50px; display:block; margin:auto;' onclick=\"location.href='/add'\">添加WiFi</button><br>";
        html +=
            "<button style='font-size:50px; display:block; margin:auto;' onclick=\"location.href='/delete'\">删除WiFi</button><br>";
        html +=
            "<button style='font-size:50px; display:block; margin:auto;' onclick=\"location.href='/update_time'\">更新时间天气</button><br>";
        // 新增按钮
        server.send(200, "text/html; charset=utf-8", html);
    });

    server.on("/add", HTTP_GET, []()
    {
        String html = "<h1 style='text-align:center; font-size:50px'>输入WiFi SSID</h1>";
        html += "<form action='/submit' method='POST' style='text-align:center;'>";
        html += "SSID: <input type='text' name='ssid' style='font-size:50px;'><br>";
        html += "Password: <input type='text' name='password' style='font-size:50px;'><br>";
        html += "<input type='submit' value='提交' style='font-size:50px;'>";
        html += "</form>";
        server.send(200, "text/html; charset=utf-8", html);
    });

    server.on("/submit", HTTP_POST, []()
    {
        String ssid = server.arg("ssid");
        String password = server.arg("password");

        // 保存SSID和密码到SPIFFS
        File file = SPIFFS.open("/config.txt", "w");
        if (file)
        {
            file.println(ssid);
            file.println(password);
            file.close();
        }

        // 串口输出SSID和密码
        Serial.println("SSID: " + ssid);
        Serial.println("Password: " + password);

        server.send(200, "text/html; charset=utf-8",
                    "<p style='text-align:center; font-size:50px'>已添加" + ssid + "到配置</p> "
                    "<meta http-equiv='refresh' content='2; URL=/' />");
        WiFi.begin(ssid.c_str(), password.c_str());
    });

    server.on("/delete", HTTP_GET, []()
    {
        // 删除WiFi配置逻辑
        SPIFFS.remove("/config.txt");
        server.send(200, "text/html; charset=utf-8",
                    "<p style='text-align:center; font-size:50px'>WiFi配置已删除</p> "
                    "<meta http-equiv='refresh' content='2; URL=/' />");
    });

    server.on("/update_time", HTTP_GET, []()
    {
        // 发送更新时间信号
        timeQuest = true;
        Serial.println("收到更新时间请求"); // 可以在这里处理更新时间逻辑
        server.send(200, "text/html; charset=utf-8",
                    "<p style='text-align:center; font-size:50px'>更新请求已发送</br>天气返回状态：" + String(httpcode1) +
                    "</br>时间请求返回状态：" +
                    String(httpcode2) + "</p>" "<meta http-equiv='refresh' content='2; URL=/' />");
    });

    server.begin();
    Serial.println("HTTP server started");

    // 循环处理客户端请求
    while (true)
    {
        server.handleClient();
        vTaskDelay(1); // 让出 CPU 给其他任务// 获取连接到热点的设备数量
        if (WiFi.softAPgetStationNum())
        {
            isConnectedDevice = true;
        }else
        {
            isConnectedDevice = false;
        }
    }
}

void buttonTask(void* pvParameters)
{
    while (true)
    {
        if (digitalRead(buttonPin) == HIGH)
        {
            vTaskDelay(pdMS_TO_TICKS(20));
            while (digitalRead(buttonPin) == HIGH)
            {
            }
            vTaskDelay(pdMS_TO_TICKS(20));
            digitalWrite(outputPin, !digitalRead(outputPin));
            page++;
            page %= 5;
        }
    }
}

void setup()
{
    // 初始化SPIFFS
    if (!SPIFFS.begin())
    {
        Serial.println("SPIFFS-An error occurred while mounting SPIFFS");
        // 格式化SPIFFS分区
        if (SPIFFS.format())
        {
            Serial.println("SPIFFS partition formatted successfully");
            ESP.restart(); // 重启ESP32
        }
        else
        {
            Serial.println("SPIFFS partition format failed");
        }
        return;
    }

    pinMode(buttonPin,INPUT_PULLDOWN);
    pinMode(outputPin,OUTPUT);
    Wire.begin(OLED_SDA, OLED_SCL); // sda, scl
    Serial.begin(115200);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println(F("SSD1306 allocation failed"));
        return;
    }


    // 读取保存的WiFi配置
    File file = SPIFFS.open("/config.txt", "r");
    if (file)
    {
        String savedSSID = file.readStringUntil('\n'); // 读取SSID
        String savedPassword = file.readStringUntil('\n'); // 读取密码
        file.close();
        // 去除空格和换行符
        savedSSID.trim();
        savedPassword.trim();
        // 尝试连接保存的WiFi
        display.setTextColor(WHITE);
        display.setTextSize(1);
        if (savedSSID.isEmpty())
        {
            display.clearDisplay();
            display.println("please connect the wifi was called:ZhangDeXiaoNaoZhong,password:12345678");
            display.display();
            display.clearDisplay();
        }
        else
        {
            display.clearDisplay();
            display.println("try to connect " + savedSSID + " " + savedPassword);
            Serial.println("尝试连接到保存的WiFi..." + savedSSID + " " + savedPassword);
            WiFi.begin(savedSSID.c_str(), savedPassword.c_str());
            display.display();
        }

        // 等待连接
        unsigned long startAttemptTime = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 20000)
        {
            delay(1000); // 等待10秒以内
            Serial.print(".");
        }
        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("连接成功");
            //wifi成功连接，展示wifi图标
            display.clearDisplay();
            display.drawBitmap(118, 0, signBitmap, 10, 6,WHITE);
            display.display();
            delay(100);
        }
        else
        {
            Serial.println("无法连接到WiFi，使用热点模式");
        }

        xTaskCreate(buttonTask
                    , "ButtonTask" //任务名
                    , 2048 // 栈大小
                    , NULL
                    , 1 // 任务优先级
                    , NULL);
        // 创建任务
        xTaskCreate(webServerTask, "WebServerTask", 8192, NULL, 1, NULL);
    }
    else
    {
        Serial.println("未找到保存的WiFi配置，使用热点模式");
        display.clearDisplay();
        display.println("please connect the wifi was called:xiaonaozhong,password:12345678");
        display.display();
        display.clearDisplay();
    }
    // 进入热点模式
    WiFi.softAP(ssid, password);
}

void ShowPage(int pageIndex, int xOffset)
{
    switch (pageIndex)
    {
    case 0:
        page0.PMainShow(xOffset);
        break;
    case 1:
        page1.PMainShow(xOffset);
        break;
    case 2:
        page2.PMainShow(xOffset);
        break;
    case 3:
        page3.PMainShow(xOffset);
        break;
    case 4:
        page4.PMainShow(xOffset);
        break;
    default:
        break;
    }
}

/*
 * @param right为ture时,->滑动
 *        right为false时,<-滑动
 */
void Slide(int fromPage, int toPage)
{
    // bool right = fromPage - toPage > 0;
    bool right = false;
    int start = right ? SCREEN_WIDTH : 0;
    int end = right ? 0 : SCREEN_WIDTH;

    if (right)
    {
        // for (int xOffset = start; xOffset >= end; xOffset -= 16)
        // {
        //     ShowPage(fromPage, xOffset - SCREEN_WIDTH);
        //     ShowPage(toPage, xOffset);
        // }
    }
    else
    {
        for (int xOffset = start; xOffset <= end; xOffset += 16)
        {
            // 显示滑出页面
            ShowPage(fromPage, xOffset);
            // 显示滑入页面
            ShowPage(toPage, xOffset - SCREEN_WIDTH);
        }
    }
}

int cruPage = 1;

void loop()
{
    if (page - cruPage > 0)
    {
        Slide(cruPage, page);
    }
    else if (cruPage - page == 4)
    {
        Slide(4, 0);
    }
    cruPage = page;
    switch (cruPage)
    {
    case 0:
        page0.PMainShow(0);
        break;
    case 1:
        page1.PMainShow(0);
        break;
    case 2:
        page2.PMainShow(0);
        break;
    case 3:
        page3.PMainShow(0);
        break;
    case 4:
        page4.PMainShow(0);
        break;
    default:
        break;
    }
}
