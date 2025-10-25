#include <BluetoothSerial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#define TFT_CS   5
#define TFT_DC   2
#define TFT_RST  4

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
BluetoothSerial SerialBT;

// ======== CONFIG ========
bool DEMO_MODE = false;        // true = fake car acceleration, false = Bluetooth input
float displaySpeed = 0;
float targetSpeed = 0;
float smoothing = 0.15;       // smaller = slower transition, larger = snappier
unsigned long lastUpdate = 0;
bool phoneConnected = false;  
int speedLimit = -1; 
float lastParsedSpeed = 0;     // raw parsed speed (before buffering)
bool blinkState = false;
unsigned long lastBlink = 0;

// ======== CALLBACK ========
void btCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
  if (event == ESP_SPP_SRV_OPEN_EVT) {
    phoneConnected = true;
    Serial.println("📱 Phone connected via Bluetooth!");
    char macStr[18];
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
            param->srv_open.rem_bda[0], param->srv_open.rem_bda[1],
            param->srv_open.rem_bda[2], param->srv_open.rem_bda[3],
            param->srv_open.rem_bda[4], param->srv_open.rem_bda[5]);
    Serial.print("Device MAC: ");
    Serial.println(macStr);
  }
  if (event == ESP_SPP_CLOSE_EVT) {
    phoneConnected = false;
    Serial.println("❌ Phone disconnected.");
  }
}

// ======== LOGO BITMAP (placeholder) ========
const unsigned char epd_bitmap_Bitmap [] PROGMEM = {
	//PUT YOUR OWN BITMAP ARRAY HERE
  };

// ======== BOOT ANIMATION ========
#define BACKLIGHT_PIN 19
#define PWM_FREQ 5000
#define PWM_RESOLUTION 8   // 8-bit

void showBMWLogo() {
  const int logoSize = 100;
  const int logoX = (tft.width() - logoSize) / 2;
  const int logoY = (tft.height() - logoSize) / 2;

  ledcAttach(BACKLIGHT_PIN, PWM_FREQ, PWM_RESOLUTION);

  tft.fillScreen(ST77XX_BLACK);
  tft.drawBitmap(logoX, logoY, epd_bitmap_Bitmap, logoSize, logoSize, ST77XX_WHITE);

  for (int brightness = 0; brightness <= 255; brightness++) {
    ledcWrite(BACKLIGHT_PIN, brightness);
    delay(8);
  }

  delay(1000);
  ledcWrite(BACKLIGHT_PIN, 255);
  tft.fillScreen(ST77XX_BLACK);
}

// ======== SETUP ========
void setup() {
  Serial.begin(115200);
  SerialBT.register_callback(btCallback);
  SerialBT.begin("ESP32_HUD");

  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST77XX_BLACK);
  tft.setRotation(1);
  tft.setTextWrap(false);

  ledcWrite(0, 0);
  showBMWLogo();

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(10, 10);
  tft.print("HUD Ready");
  delay(1000);
  tft.fillScreen(ST77XX_BLACK);

  Serial.println("=========== HUD READY ===========");
  Serial.println("Bluetooth name: ESP32_HUD");
  Serial.println(DEMO_MODE ? "Mode: DEMO" : "Mode: BLUETOOTH");
}

// ======== LOOP ========
void loop() {
  // --- DEMO MODE ---
  if (DEMO_MODE) {
    static unsigned long lastFakeUpdate = 0;
    static float fakeSpeed = 0;
    static float accel = 0.3;

    if (millis() - lastFakeUpdate > 100) {
      lastFakeUpdate = millis();
      fakeSpeed += accel;
      if (fakeSpeed > 120 || fakeSpeed < 0) accel = -accel;
      targetSpeed = fakeSpeed;
    }
  }
  // --- BLUETOOTH MODE ---
  else {
    if (SerialBT.available()) {
  String line = SerialBT.readStringUntil('\n');
  line.trim();
  if (line.startsWith("LIMIT:")) {
    String value = line.substring(6);
    int limit = value.toInt();
    if (limit > 0 && limit <= 260) {
      speedLimit = limit;
      Serial.print("📍 Speed limit updated: ");
      Serial.println(speedLimit);
    }
  } else {
    float parsed = line.toFloat();
    if (parsed >= 0 && parsed <= 260) {
      lastParsedSpeed = parsed; // store raw
      float buffered = parsed * 1.03 + 1.0;
      targetSpeed = buffered;
    }
  }
}
  }

  // --- SMOOTH INTERPOLATION ---
  displaySpeed += (targetSpeed - displaySpeed) * smoothing;

  // --- REFRESH DISPLAY (only when speed changes) ---
static int lastSpeedInt = -1;   // remember last drawn speed

int speedInt = (int)displaySpeed;

// Draw connection indicator circle (always update this)
  uint16_t circleColor = phoneConnected ? ST77XX_GREEN : ST77XX_RED;
  tft.fillCircle(8, 8, 4, circleColor);

if (speedInt != lastSpeedInt) {
  lastSpeedInt = speedInt;

  // Clear just the speed area
  tft.fillRect(0, 20, 160, 80, ST77XX_BLACK);

  // Draw speed number (centered)
  String speedStr = String(speedInt);
  tft.setTextSize(7);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);

  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(speedStr, 0, 0, &x1, &y1, &w, &h);

  int xPos = (tft.width() - w) / 2;
  int yPos = (tft.height() - h) / 2 - 10;
  tft.setCursor(xPos, yPos);
  tft.print(speedStr);

  // Draw km/h unit
  tft.setTextSize(2);
  String unit = "km/h";
  tft.getTextBounds(unit, 0, 0, &x1, &y1, &w, &h);
  xPos = (tft.width() - w) / 2;
  tft.setCursor(xPos, yPos + 70);
  tft.print(unit);
}
// === Draw speed limit sign ===
if (speedLimit > 0) {
  bool overLimit = lastParsedSpeed > speedLimit;

  // Blink toggle if over limit
  if (overLimit && millis() - lastBlink > 500) {
    blinkState = !blinkState;
    lastBlink = millis();
  }

  // Square position and size (bottom-right corner)
  int boxSize = 40;
  int x0 = tft.width() - boxSize - 5;   // 5 px margin from right edge
  int y0 = tft.height() - boxSize - 5;  // 5 px margin from bottom

  // Background color logic
  uint16_t bgColor = ST77XX_WHITE;
  uint16_t borderColor = ST77XX_RED;
  if (overLimit && blinkState) {
    bgColor = ST77XX_RED;       // whole box flashes red
    borderColor = ST77XX_WHITE; // invert border for contrast
  }

  // Draw filled square
  tft.fillRect(x0, y0, boxSize, boxSize, bgColor);
  // Draw border
  tft.drawRect(x0, y0, boxSize, boxSize, borderColor);

  // Draw limit number centered
  String limitStr = String(speedLimit);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_BLACK, bgColor);
  int16_t x1, y1; uint16_t w, h;
  tft.getTextBounds(limitStr, 0, 0, &x1, &y1, &w, &h);
  int cx = x0 + (boxSize - w) / 2;
  int cy = y0 + (boxSize - h) / 2;
  tft.setCursor(cx, cy);
  tft.print(limitStr);
}
  delay(10);
}
