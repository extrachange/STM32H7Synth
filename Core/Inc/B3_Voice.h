#ifndef B3_VOICE_H
#define B3_VOICE_H

#include <stdint.h>
#include "midi.h"
#include <math.h>

extern volatile uint8_t B3_OscVol[9];
typedef struct {
    uint32_t phaseInc;		//相位累加量，只在更改频率的时候更新
    uint32_t phase; 		//Q16.16定点
    uint32_t output;
    uint16_t *SineLut;
    float frequency;
} SineOsc;

typedef struct {
    uint32_t phaseInc[9];
    uint32_t phase[9]; 		//Q16.16定点
    uint16_t *SineLut;
    uint16_t vol_scale;
    uint64_t output;
    int volume[9];
    int tooHigh[9];
    float frequency;
} SineOsc9x;

typedef struct {
    uint16_t attack;
    uint16_t release;
    uint16_t value;
} Envelope;

typedef struct {
    uint16_t buffer_l[2048];
    uint16_t out_l;
    uint16_t out_r;
    uint16_t in_index_l;	//输入是单声道的所以这个变量名末尾的l没有意义，但我不想改了
    float out_index_l;
    float out_index_r;
    float samplerate_l;
    float samplerate_r;
} stDelay;

void SineOsc_tick(SineOsc *osc);
void SineOsc_init(SineOsc *osc, uint16_t *sineTable);
void SineOsc_setFreq(SineOsc *osc, float frequency);

void SineOsc9x_tick(SineOsc9x *osc);
void SineOsc9x_init(SineOsc9x *osc, uint16_t *sineTable);
void SineOsc9x_setFreq(SineOsc9x *osc, float frequency);
void SineOsc9x_setVol(SineOsc9x *osc, volatile uint8_t params[]);

void Envelope_tick(Envelope env[], volatile voice voicePool[]);
void Envelope_init(Envelope env[]);
void Envelope_setSpeed(Envelope env[],int speed);

void stDelay_tick(stDelay *delay, uint16_t inL, uint16_t inR);
void stDelay_timeMod(stDelay *delay, int16_t mod);
void stDelay_init(stDelay *delay);

#endif // B3_VOICE_H
