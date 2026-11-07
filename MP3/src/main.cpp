#include <Arduino.h>
#include "AudioTools.h"
#include <Wire.h>
#include <U8g2lib.h>

const int PIN_PLAY     = 13;
const int PIN_VOL_UP   = 14;
const int PIN_VOL_DOWN = 27;
const int PIN_SDA      = 32;
const int PIN_SCL      = 33;

// Choisis la ligne qui correspond à ton écran (une seule des deux)
//U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

AudioInfo info(44100, 2, 16);
SineWaveGenerator<int16_t> sineWave(16000);
GeneratedSoundStream<int16_t> sound(sineWave);
I2SStream i2s;
VolumeStream volume(i2s);
StreamCopy copier(volume, sound);

float vol = 0.1;
bool playing = true;

struct Button {
  uint8_t pin;
  bool last = HIGH;
  uint32_t t = 0;
  Button(uint8_t p) : pin(p) {}
  void begin() { pinMode(pin, INPUT_PULLUP); }
  bool pressed() {
    bool now = digitalRead(pin);
    if (now != last && millis() - t > 30) {
      t = millis();
      last = now;
      if (now == LOW) return true;
    }
    return false;
  }
};

Button btnPlay(PIN_PLAY), btnUp(PIN_VOL_UP), btnDown(PIN_VOL_DOWN);

void drawScreen() {
  int pct = (int)(vol * 100);
  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.drawStr(0, 14, "Test boutons");

  u8g2.setFont(u8g2_font_7x13_tr);
  u8g2.drawStr(0, 34, playing ? "Lecture" : "Pause");

  char buf[16];
  snprintf(buf, sizeof(buf), "Vol %d%%", pct);
  u8g2.drawStr(0, 52, buf);

  u8g2.drawFrame(64, 42, 62, 12);                 // cadre de la barre
  u8g2.drawBox(66, 44, (58 * pct) / 100, 8);      // remplissage

  u8g2.sendBuffer();
}

void applyVolume() {
  volume.setVolume(playing ? vol : 0.0);
  Serial.printf("%s | volume %d %%\n", playing ? "Lecture" : "Pause", (int)(vol * 100));
  drawScreen();
}

void setup() {
  Serial.begin(115200);
  AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Warning);

  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(400000);
  u8g2.begin();

  btnPlay.begin();
  btnUp.begin();
  btnDown.begin();

  auto cfg = i2s.defaultConfig(TX_MODE);
  cfg.copyFrom(info);
  cfg.pin_bck  = 26;
  cfg.pin_ws   = 25;
  cfg.pin_data = 22;
  i2s.begin(cfg);

  auto vcfg = volume.defaultConfig();
  vcfg.copyFrom(info);
  volume.begin(vcfg);

  sineWave.begin(info, N_B4);
  applyVolume();
}

void loop() {
  copier.copy();

  if (btnPlay.pressed()) { playing = !playing; applyVolume(); }
  if (btnUp.pressed())   { vol = min(vol + 0.05f, 1.0f); applyVolume(); }
  if (btnDown.pressed()) { vol = max(vol - 0.05f, 0.0f); applyVolume(); }
}