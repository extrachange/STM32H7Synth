# STM32H7Synth
Stupid B-3 simulation on Stm32H7, CubeIDE project

### File Structure: 

- main.c    All interrupts, track playback and DSP loop
- B3_Voice.h, B3_Voice.c    Oscillators, Envelopes, Chorus implementation     
- midi.h, midi.c    VoicePool, note and cc message handling
- tracks.h    typedefs for reading midi file
- tracksData.h    definition of midi file, I use midi csv to convert midi file, the first item(channel)  and everything besides channel message and end of track is deleted , and curly brackets+comma are added each line, then copied into tracksData.h

### Features:

6-Voice B-3 Structure additive synth with chorus and tremolo

MIDI over UART and (converted) midi file playback

Wavetable oscillator which can be modified

Audio output using DMA

12bit Audio out put using internal DAC (not a "feature")

