#include <Arduino.h>  // This must be included first!

// Choose large buffer size for ESP32 JSON
#define LARGE_JSON_BUFFERS 1
#include "Thing.h"
#include "WebThingAdapter.h"

#include "everloop.h"
#include "everloop_image.h"
#include "microphone_array.h"
#include "microphone_core.h"
#include "voice_memory_map.h"
#include "wishbone_bus.h"

#define RATE 16000

// Matrix Voice
namespace hal = matrix_hal;
static hal::WishboneBus wb;
static hal::Everloop everloop;
static hal::MicrophoneArray mics;
static hal::EverloopImage image1d;

WebThingAdapter* adapter;

const char* ledTypes[] = {"OnOffSwitch", "Light", "ColorControl", nullptr};
ThingDevice led("board", "MATRIX Voice", ledTypes);
ThingProperty ledOn("on", "The on/off status of the LEDs", BOOLEAN, "OnOffProperty");
ThingProperty ledLevel("level", "The level of light from 0-100", NUMBER, "BrightnessProperty");
ThingProperty ledColor("color", "The color of light in RGB", STRING, "ColorProperty");

bool lastOn = false;
String lastColor = "#ffffff";

void everloopSet(int r, int g, int b, int w) {
  for (hal::LedValue &led : image1d.leds) {
    led.red = r;
    led.green = g;
    led.blue = b;
    led.white = w;
  }

  everloop.Write(&image1d);
}

void setup() {
  Serial.begin(115200);
  Serial.println("[SETUP] MatrixVoice init..");

  wb.Init();
  everloop.Setup(&wb);

  // setup mics
  mics.Setup(&wb);
  mics.SetSamplingRate(RATE);
  // mics.SetGain(5);

  // microphone core init
  hal::MicrophoneCore mic_core(mics);
  mic_core.Setup(&wb);

  Serial.println("");
  Serial.print("Connecting to \"");
  Serial.print(WIFI_SSID);
  Serial.println("\"");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.println("");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    everloopSet(10,0,0,0);
  }
  everloopSet(0,10,0,0);

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  adapter = new WebThingAdapter("matrix-voice", WiFi.localIP());

  led.addProperty(&ledOn);
  ThingPropertyValue levelValue;
  levelValue.number = 100; //default brightness
  ledLevel.setValue(levelValue);
  led.addProperty(&ledLevel);

  ThingPropertyValue colorValue;
  colorValue.string = &lastColor; //default color is white
  ledColor.setValue(colorValue);
  led.addProperty(&ledColor);

  adapter->addDevice(&led);
  adapter->begin();
  Serial.println("HTTP server started");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.print("/things/");
  Serial.println(led.id);

  Serial.println("[SETUP] MatrixVoice done.");
}

unsigned counter = 0;

void everloopAnimation() {
  for (hal::LedValue &led : image1d.leds) {
    led.red = 0;
    led.green = 0;
    led.blue = 0;
    led.white = 0;
  }
  image1d.leds[(counter / 2) % image1d.leds.size()].red = 20;
  image1d.leds[(counter / 7) % image1d.leds.size()].green = 30;
  image1d.leds[(counter / 11) % image1d.leds.size()].blue = 30;
  image1d.leds[image1d.leds.size() - 1 - (counter % image1d.leds.size())]
      .white = 10;

  ++counter;
  everloop.Write(&image1d);
  usleep(25000);
}

void update(String* color, int const level) {
  if (!color) return;
  float dim = level/100.0;
  int red,green,blue;
  if (color && (color->length() == 7) && color->charAt(0) == '#') {
    const char* hex = 1+(color->c_str()); // skip leading '#'
    sscanf(0+hex, "%2x", &red);
    sscanf(2+hex, "%2x", &green);
    sscanf(4+hex, "%2x", &blue);
  }
  red = int (red*dim);
  green = int (green*dim);
  blue = int (blue*dim);
  everloopSet(red,green,blue,0);
}

void loop() {
  // put your main code here
  adapter->update();
  bool on = ledOn.getValue().boolean;
  int level = ledLevel.getValue().number;
  if (on) {
    update(&lastColor, level);
  } else {
    everloopSet(0,0,0,0);
  }

  if (on != lastOn) {
    lastOn = on;
    Serial.print(led.id);
    Serial.print(": ");
    Serial.println(on);
    Serial.print(", level: ");
    Serial.print(level);
    Serial.print(", color: ");
    Serial.println(lastColor);
  }
}
