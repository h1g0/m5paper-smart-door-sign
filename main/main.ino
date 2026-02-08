#include <M5Unified.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

#include "config.h"

void setup()
{
  auto cfg = M5.config();
  M5.begin(cfg);

  M5.Display.setRotation(1);
  M5.Display.setTextSize(1);

  M5.Display.setFont(&fonts::efontJA_24);

  M5.Display.print("Connecting to WiFi...");
  WiFi.begin(SSID, PASS);
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    M5.Display.print(".");
    timeout++;
    if (timeout > 20)
    { // Give up and sleep if not connected within 10 seconds
      shutdownDevice();
    }
  }

  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);

  String statusText = fetchStatus();
  String updateDateTime = getUpdateDateTimeString();

  drawStatus(statusText, updateDateTime);

  shutdownDevice();
}

void loop()
{
  // loop will not be reached due to Deep Sleep
}

String fetchStatus()
{
  HTTPClient http;
  // setFollowRedirects is required because GAS URLs are often redirected
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.begin(GAS_URL);

  int httpCode = http.GET();
  String result = "Error";

  if (httpCode == HTTP_CODE_OK)
  {
    String payload = http.getString();

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (!error)
    {
      const char *status = doc["status"];
      result = String(status);
    }
  }
  http.end();
  return result;
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

void drawStatus(String text, String updateDateTime)
{
  M5.Display.startWrite();
  M5.Display.clear(TFT_WHITE); // Fill with white
  M5.Display.setTextColor(TFT_BLACK, TFT_WHITE);

  int x = M5.Display.width() / 2;
  int y = M5.Display.height() / 2;

  M5.Display.setTextDatum(middle_center);
  M5.Display.setTextSize(8);
  M5.Display.drawString(text, x, y);

  drawFooter(updateDateTime);

  M5.Display.endWrite();

  delay(2000);
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

void shutdownDevice()
{
  WiFi.disconnect(true);

  M5.Power.timerSleep(SLEEP_MIN * 60);
}
