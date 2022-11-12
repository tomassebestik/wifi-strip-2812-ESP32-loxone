#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <FastLED.h>
#include "env_config.h"

#define PIN 13        // GPIO pin of ESP32 for LED strip signal wire (DI)
#define NUMPIXELS 144 // Number of LEDs on the strip
#define DELAYVAL 0    // Delay (ms) between individual pixels

CRGB leds[NUMPIXELS];   // Create LED array
char serprt_buffer[50]; // Create serial print buffer

AsyncWebServer server(80);         // Create AsyncWebServer object on port 80
const char *COLOR_INPUT = "color"; // GET request param query keyword

// PRESET: Enable pixels symmetrically from center to edges of the strip
void colorEnableFromCenter(int red, int green, int blue)
{
  for (int i = NUMPIXELS / 2; i >= 0; i--)
  {
    leds[i].setRGB(red, green, blue);
    leds[NUMPIXELS - i].setRGB(red, green, blue);
    FastLED.show();
    FastLED.delay(DELAYVAL);
  }
}

// PRESET: Disable pixels symmetrically from the edges to the center of the strip
void colorDisableToCenter(int red, int green, int blue)
{
  for (int i = 0; i < (NUMPIXELS / 2) + 1; i++)
  {
    leds[i].setRGB(red, green, blue);
    leds[NUMPIXELS - i].setRGB(red, green, blue);
    FastLED.show();
    FastLED.delay(DELAYVAL);
  }
}

// PRESET: Enable pixels from one end of the strip to the other
void colorEnableFromEdge(int red, int green, int blue)
{
  for (int i = NUMPIXELS; i >= 0; i = i - 3)
  // In groups of 3 pixels
  {
    leds[i].setRGB(red, green, blue);
    leds[i + 1].setRGB(red, green, blue);
    leds[i + 2].setRGB(red, green, blue);
    FastLED.show();
    FastLED.delay(DELAYVAL);
  }
}

// PRESET: Disable pixels from one end of the strip to the other (backwards than `colorEnableFromEdge`)
void colorDisableToEdge(int red, int green, int blue)
{
  for (int i = 0; i < NUMPIXELS; i = i + 3)
  // In groups of 3 pixels
  {
    leds[i].setRGB(red, green, blue);
    leds[i + 1].setRGB(red, green, blue);
    leds[i + 2].setRGB(red, green, blue);
    FastLED.show();
    FastLED.delay(DELAYVAL);
  }
}

void setup()
{
  FastLED.addLeds<WS2812B, PIN, GRB>(leds, NUMPIXELS); // Init LED ring

  Serial.begin(115200); // Start serial monitor

  // Connect to Wi-Fi
  const char *ssid = ENV_WIFI_SSID;
  const char *password = ENV_WIFI_PASS;
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi ...");
  }
  Serial.println(WiFi.localIP());

  // Send a GET request to http://<ESP_IP>/update?color=<inputMessage>
  // Example command: http://192.168.1.227/update?color=17100024
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String inputMessage;
    inputMessage = request->getParam(COLOR_INPUT)->value();

    int color = inputMessage.toInt();
    sprintf(serprt_buffer, "Color received from Loxone: %d", color);
    Serial.println(serprt_buffer);

    if (color == 0)
    {
      // colorDisableToCenter(0,0,0);
      colorDisableToEdge(0,0,0);
    }

    else
    {
      // Extract RED (0-100%)
      int red = color % 1000;
      color = (color - red) / 1000;

      // Extract GREEN (0-100%)
      int green = color % 1000;
      color = (color - green) / 1000;

      // Extract BLUE (0-100%)
      int blue = color;

      // Map percent values (0-100%) to RGB 8-bit values (0-255)
      red *= 2.55;
      green *= 2.55;
      blue *= 2.55;
      sprintf(serprt_buffer, "Color values to LED STRIP - RED: %d, GREEN: %d, BLUE: %d", red, green, blue);
      Serial.println(serprt_buffer);

      // Call LED preset
      // colorEnableFromCenter(red, green, blue);
      colorEnableFromEdge(red, green, blue);
    }

    request->send(200, "text/plain", "OK"); });

  // Start server
  server.begin();
}

void loop()
{
}
