#include <Arduino.h>  // This must be included first to use Arduino libraries/compilation env

// Choose large buffer size for ESP32 JSON
#define LARGE_JSON_BUFFERS 1
#include "Thing.h"
#include "WebThingAdapter.h"
#include <analogWrite.h>

// MATRIX packages
#include "everloop.h"
#include "everloop_image.h"
#include "microphone_array.h"
#include "microphone_core.h"
#include "voice_memory_map.h"
#include "wishbone_bus.h"

#define RATE 16000

// MATRIX Voice data bus, mic, and LED objects
namespace hal = matrix_hal;
static hal::WishboneBus wb;
static hal::Everloop everloop;
static hal::MicrophoneArray mics;
static hal::EverloopImage image1d;

// Create a WebThing Adapter
WebThingAdapter* adapter;

// Create WebThing object, type, and properties
const char* ledTypes[] = {"OnOffSwitch", "Light", "ColorControl", "MultiLevelSwitch", nullptr};
ThingDevice led("board", "MATRIX Voice", ledTypes);
ThingProperty ledOn("on", "The on/off status of the LEDs", BOOLEAN, "OnOffProperty");
ThingProperty ledLevel("level", "The level of light from 0-100", NUMBER, "BrightnessProperty");
ThingProperty ledColor("color", "The color of light in RGB", STRING, "ColorProperty");
ThingProperty gpio12out("gpio12level", "Analog output pin", NUMBER, "LevelProperty");
ThingProperty gpio25out("gpio25level", "Analog output pin", NUMBER, "LevelProperty");
ThingProperty gpio26out("gpio26level", "Analog output pin", NUMBER, "LevelProperty");
ThingProperty gpio27out("gpio27level", "Analog output pin", NUMBER, "LevelProperty");


// Define exposed ESP32 GPIOs
const int gpio_12 = 12;
const int gpio_25 = 25;
const int gpio_26 = 26;
const int gpio_27 = 27;


bool lastOn = false;
String lastColor = "#ffffff";

// Function for setting LED colors. This function sets all of the LEDs to the same color.
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

  // Setup mics
  mics.Setup(&wb);
  mics.SetSamplingRate(RATE);
  // mics.SetGain(5);

  // Microphone core initialization
  hal::MicrophoneCore mic_core(mics);
  mic_core.Setup(&wb);

  // Connect to WiFi
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


  // Add created properties to WebThing device and set defaults
  led.addProperty(&ledOn);
  ThingPropertyValue levelValue;
  levelValue.number = 100; //default brightness
  ledLevel.setValue(levelValue);
  led.addProperty(&ledLevel);

  ThingPropertyValue colorValue;
  colorValue.string = &lastColor; //default color is white
  ledColor.setValue(colorValue);
  led.addProperty(&ledColor);

  //////////////////////////////////
  // GPIO setting //////////////////
  ThingPropertyValue analogVal;
  analogVal.number = 0; //default analog value is 0

  gpio12out.title = "Set Pin IO12";
  gpio12out.minimum = 0;
  gpio12out.maximum = 255;
  gpio12out.setValue(analogVal);
  led.addProperty(&gpio12out);

  gpio25out.title = "Set Pin IO25";
  gpio25out.minimum = 0;
  gpio25out.maximum = 255;
  gpio25out.setValue(analogVal);
  led.addProperty(&gpio25out);

  gpio26out.title = "Set Pin IO26";
  gpio26out.minimum = 0;
  gpio26out.maximum = 255;
  gpio26out.setValue(analogVal);
  led.addProperty(&gpio26out);

  gpio27out.title = "Set Pin IO27";
  gpio27out.minimum = 0;
  gpio27out.maximum = 255;
  gpio27out.setValue(analogVal);
  led.addProperty(&gpio27out);
  //////////////////////////////////

  adapter->addDevice(&led);
  adapter->begin();
  Serial.println("HTTP server started");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.print("/things/");
  Serial.println(led.id);

  Serial.println("[SETUP] MatrixVoice done.");
}

// Update everloop LEDs with hex color noted
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
  // Update values received from Gateway
  adapter->update();
  bool on = ledOn.getValue().boolean;
  int level = ledLevel.getValue().number;
  int gpio12_val = gpio12out.getValue().number;
  int gpio25_val = gpio25out.getValue().number;
  int gpio26_val = gpio26out.getValue().number;
  int gpio27_val = gpio27out.getValue().number;

  // Update LEDs based on On/Off toggle, color, and brightness level
  if (on) {
    update(&lastColor, level);
  } else {
    everloopSet(0,0,0,0);
  }

  // Set LEDs to value received from Gateway
  analogWrite(gpio_12, gpio12_val);
  analogWrite(gpio_25, gpio25_val);
  analogWrite(gpio_26, gpio26_val);
  analogWrite(gpio_27, gpio27_val);

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
