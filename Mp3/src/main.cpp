#include "AudioTools.h"

AudioInfo info(44100, 2, 16);
SineWaveGenerator<int16_t> sineWave(3000);   // amplitude faible, volontairement
GeneratedSoundStream<int16_t> sound(sineWave);
I2SStream out;
StreamCopy copier(out, sound);

void setup() {
  Serial.begin(115200);
  AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Info);

  auto cfg = out.defaultConfig(TX_MODE);
  cfg.copyFrom(info);
  cfg.pin_bck  = 26;
  cfg.pin_ws   = 25;
  cfg.pin_data = 22;
  out.begin(cfg);

  sineWave.begin(info, N_B4);   // note si (~494 Hz)
}

void loop() {
  copier.copy();
}