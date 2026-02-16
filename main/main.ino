#include <M5Unified.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

#include "config.h"

namespace
{
  struct Rect
  {
    int x;
    int y;
    int w;
    int h;
  };

  const int kPowerButtonMargin = 8;
  const int kPowerButtonWidth = 72;
  const int kPowerButtonHeight = 36;
  const uint32_t kWifiScreenMinShowMs = 1000;
  const uint32_t kWifiConnectTimeoutMs = 10000;
  const uint32_t kWifiDotIntervalMs = 500;

  Rect powerButtonRect()
  {
    Rect r;
    r.x = M5.Display.width() - kPowerButtonMargin - kPowerButtonWidth;
    r.y = kPowerButtonMargin;
    r.w = kPowerButtonWidth;
    r.h = kPowerButtonHeight;
    return r;
  }
}

void setup()
{
  auto cfg = M5.config();
  M5.begin(cfg);

  M5.Display.setRotation(1);
  M5.Display.setTextSize(1);

  M5.Display.setFont(&fonts::efontJA_24);
  WiFi.begin(SSID, PASS);

  M5.Display.startWrite();
  M5.Display.clear(TFT_WHITE);
  M5.Display.setTextColor(TFT_BLACK, TFT_WHITE);
  M5.Display.setTextDatum(top_left);
  M5.Display.drawString("Connecting to WiFi", 8, 8);
  drawPowerButton();
  M5.Display.endWrite();

  uint32_t start = millis();
  uint32_t lastDot = start;
  int dotCount = 0;
  while (true)
  {
    M5.update();
    if (isPowerButtonPressed())
    {
      showPowerOffScreen();
      powerOffDevice();
    }

    uint32_t now = millis();
    if (now - lastDot >= kWifiDotIntervalMs)
    {
      if (dotCount < 20)
      {
        M5.Display.drawString(".", 8 + 14 * dotCount, 40);
        dotCount++;
      }
      lastDot = now;
    }

    if (now - start > kWifiConnectTimeoutMs)
    { // Give up and sleep if not connected within 10 seconds.
      shutdownDevice();
    }

    if (WiFi.status() == WL_CONNECTED && now - start >= kWifiScreenMinShowMs)
    {
      break;
    }

    delay(20);
  }

  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);

  String mainText;
  int mainSize;
  String subText;
  int subSize;
  fetchStatus(mainText, mainSize, subText, subSize);
  String updateDateTime = getUpdateDateTimeString();

  drawStatus(mainText, mainSize, subText, subSize, updateDateTime);
  shutdownDevice();
}

void loop()
{
  // loop will not be reached due to Deep Sleep
}

void fetchStatus(String &mainText, int &mainSize, String &subText, int &subSize)
{
  mainText = "Error";
  mainSize = 8;
  subText = "";
  subSize = 4;

  HTTPClient http;
  // setFollowRedirects is required because GAS URLs are often redirected
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.begin(GAS_URL);

  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK)
  {
    String payload = http.getString();

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (!error)
    {
      if (doc["main"].is<JsonObject>())
      {
        JsonObject mainObj = doc["main"];
        const char *mainTextVal = mainObj["text"] | "Error";
        mainText = String(mainTextVal);
        mainSize = mainObj["size"] | 8;
      }
      else
      {
        // Backward compatibility: accept old payload with `status` only.
        const char *status = doc["status"] | "Error";
        mainText = String(status);
      }

      if (doc["sub"].is<JsonObject>())
      {
        JsonObject subObj = doc["sub"];
        const char *subTextVal = subObj["text"] | "";
        subText = String(subTextVal);
        subSize = subObj["size"] | 4;
      }
    }
  }
  http.end();
}

String getUpdateDateTimeString()
{
  struct tm timeinfo;
  if (getLocalTime(&timeinfo, 5000))
  {
    char buf[20];
    if (strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &timeinfo))
    {
      return String(buf);
    }
  }
  return String("Unknown");
}

void drawStatus(String mainText, int mainSize, String subText, int subSize, String updateDateTime)
{
  M5.Display.startWrite();
  M5.Display.clear(TFT_WHITE); // Fill with white
  M5.Display.setTextColor(TFT_BLACK, TFT_WHITE);

  int x = M5.Display.width() / 2;
  int y = M5.Display.height() / 2;
  int subGap = 24;
  int safeMainSize = mainSize > 0 ? mainSize : 8;
  int safeSubSize = subSize > 0 ? subSize : 4;
  bool hasSub = subText.length() > 0;

  M5.Display.setTextDatum(middle_center);
  M5.Display.setTextSize(safeMainSize);
  int mainHeight = M5.Display.fontHeight();

  int subHeight = 0;
  if (hasSub)
  {
    M5.Display.setTextSize(safeSubSize);
    subHeight = M5.Display.fontHeight();
  }

  int totalHeight = mainHeight + (hasSub ? (subGap + subHeight) : 0);
  int topY = y - (totalHeight / 2);
  int mainY = topY + (mainHeight / 2);

  M5.Display.setTextSize(safeMainSize);
  M5.Display.drawString(mainText, x, mainY);

  if (hasSub)
  {
    int subY = topY + mainHeight + subGap + (subHeight / 2);
    M5.Display.setTextSize(safeSubSize);
    M5.Display.drawString(subText, x, subY);
  }

  drawFooter(updateDateTime);

  M5.Display.endWrite();
}

void drawFooter(String updateDateTime)
{
  int margin = 8;
  int right = M5.Display.width() - margin;
  int bottom = M5.Display.height() - margin;
  int battery = M5.Power.getBatteryLevel();

  M5.Display.setFont(&fonts::efontJA_24);
  M5.Display.setTextSize(1);
  M5.Display.setTextDatum(bottom_right);

  String line1 = String("Updated: ") + updateDateTime;
  String line2 = String("Battery: ") + String(battery) + "%";
  int lineHeight = M5.Display.fontHeight();

  M5.Display.drawString(line2, right, bottom);
  M5.Display.drawString(line1, right, bottom - lineHeight - 2);
}

void drawPowerButton()
{
  Rect r = powerButtonRect();

  M5.Display.drawRoundRect(r.x, r.y, r.w, r.h, 6, TFT_BLACK);
  M5.Display.setTextDatum(middle_center);
  M5.Display.setTextSize(1);
  M5.Display.drawString("OFF", r.x + r.w / 2, r.y + r.h / 2);
}

bool isPowerButtonPressed()
{
  Rect r = powerButtonRect();
  auto detail = M5.Touch.getDetail();
  if (!detail.wasPressed())
  {
    return false;
  }

  return detail.x >= r.x && detail.x <= (r.x + r.w) &&
         detail.y >= r.y && detail.y <= (r.y + r.h);
}

void showPowerOffScreen()
{
  M5.Display.startWrite();
  M5.Display.clear(TFT_WHITE);
  M5.Display.setTextColor(TFT_BLACK, TFT_WHITE);
  M5.Display.setFont(&fonts::efontJA_24);
  M5.Display.setTextDatum(middle_center);
  M5.Display.setTextSize(3);
  M5.Display.drawString("Power off", M5.Display.width() / 2, M5.Display.height() / 2);
  M5.Display.endWrite();
  delay(500);
}

void powerOffDevice()
{
  WiFi.disconnect(true);
  M5.Power.powerOff();
  M5.Power.deepSleep();
}

void shutdownDevice()
{
  WiFi.disconnect(true);

  M5.Power.timerSleep(SLEEP_MIN * 60);
}
