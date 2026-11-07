#include "AudioTools.h"

const int PIN_PLAY     = 13;
const int PIN_VOL_UP   = 14;
const int PIN_VOL_DOWN = 27;

AudioInfo info(44100, 2, 16);
SineWaveGenerator<int16_t> sineWave(16000);
GeneratedSoundStream<int16_t> sound(sineWave);
I2SStream i2s;
VolumeStream volume(i2s);
StreamCopy copier(volume, sound);

float vol = 0.1;       // volume de départ : 10 %
bool playing = true;

// Bouton avec anti-rebond : pressed() renvoie true une seule fois par appui
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

void applyVolume() {
  volume.setVolume(playing ? vol : 0.0);
  Serial.printf("%s | volume %d %%\n", playing ? "Lecture" : "Pause", (int)(vol * 100));
}

void setup() {
  Serial.begin(115200);
  AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Warning);

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