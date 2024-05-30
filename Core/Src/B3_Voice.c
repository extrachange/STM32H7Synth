#include "B3_Voice.h"
// 默认采样率48000
#ifndef SAMPLE_RATE
#define SAMPLE_RATE 48000
#endif

// 如果TABLE_SIZE没有被定义，则定义为256
#ifndef TABLE_SIZE
#define TABLE_SIZE 4096
#endif

#ifndef VOICE_COUNT
#define VOICE_COUNT 6
#endif

#define ONE_8Q16 0x00010000
void SineOsc_tick(SineOsc *osc) {
	osc->output = 0;
    		osc->phase += osc->phaseInc;
    		if (osc->phase >= 0x10000 * TABLE_SIZE) {
    			osc->phase = osc->phase & 0x001111111;
    		}
    		int index = osc->phase >> 16;
    		uint32_t sample = osc->SineLut[index];
    		osc->output = sample;
}
void SineOsc_init(SineOsc *osc, uint16_t *sineTable){
	osc->phase = 0;
	osc->output = 0;
	osc->SineLut = sineTable;
	SineOsc_setFreq(osc, 5);
}

void SineOsc_setFreq(SineOsc *osc, float frequency){
	osc->frequency = frequency;
	osc->phaseInc = 0x10000 * frequency * (float)TABLE_SIZE / (float)SAMPLE_RATE;
}
//振荡器相位计算为16位定点小数
void SineOsc9x_tick(SineOsc9x *osc) {
	osc->output = 0;
    for(int i = 0; i < 9; i++){
    	if(osc->tooHigh[i] == 0){
    		osc->phase[i] += osc->phaseInc[i];
    		if (osc->phase[i] >= 0x10000 * TABLE_SIZE) {	//相位截断
    			osc->phase[i] = osc->phase[i] & 0x001111111;
    		}
    		int index = osc->phase[i] >> 16;
    		/*											//线性插值, 很影响性能，保留仅供参考，stDelay里用了相同的算法
    		uint16_t frac = (osc->phase[i] & 0x0000FFFF);
    		int nextIndex = index + 1;
    		if (nextIndex >= TABLE_SIZE){
    			nextIndex -= TABLE_SIZE;
    		}
    		uint32_t sample = (0x00010000 - frac) * osc->SineLut[index] + frac * osc->SineLut[nextIndex];    		 */
    		uint32_t sample = 0x00010000* osc->SineLut[index];
    		osc->output += (sample * osc->volume[i]) >> 6;
        }
    }
}

void SineOsc9x_init(SineOsc9x *osc, uint16_t *sineTable){
	for(int i = 0; i < 9; i++){
		osc->phase[i] = 0;
		SineOsc9x_setFreq(osc, 440);
		osc->output = 0;
		osc->volume[i] = 0;
		osc->SineLut = sineTable;
		osc->tooHigh[i] = 0;
	}
	SineOsc9x_setVol(osc, B3_OscVol);
}

void SineOsc9x_setFreq(SineOsc9x *osc, float frequency){
	osc->frequency = frequency;
	osc->phaseInc[0] = 0x10000 * frequency * (float)TABLE_SIZE / (float)SAMPLE_RATE;
	osc->phaseInc[1] = 2 * osc->phaseInc[0];
	osc->phaseInc[2] = 3 * osc->phaseInc[0];
	osc->phaseInc[3] = 4 * osc->phaseInc[0];
	osc->phaseInc[4] = 6 * osc->phaseInc[0];
	osc->phaseInc[5] = 8 * osc->phaseInc[0];
	osc->phaseInc[6] = 10 * osc->phaseInc[0];
	osc->phaseInc[7] = 12 * osc->phaseInc[0];
	osc->phaseInc[8] = 16 * osc->phaseInc[0];
	for(int i = 0; i < 9; i++){
		if(osc->phaseInc[i] > 0x10000 * 0.5 * (float)TABLE_SIZE){
			osc->tooHigh[i] = 1;
		} else{
			osc->tooHigh[i] = 0;
		}
	}
}

void SineOsc9x_setVol(SineOsc9x *osc, volatile uint8_t params[]){
	int total = 0;
	for(int i = 0; i < 9; i++){
		osc->volume[i] = params[i];
		total += params[i];
	}
	total--;
	//osc->vol_scale = (uint16_t)(sqrt(0.4f*total));
	osc->vol_scale = (uint16_t)(0xFFFF * (float)(13 - (uint16_t)(sqrt(total))/13.0f));
}

void Envelope_tick(Envelope env[], volatile voice voicePool[]) {
	for(int i = 0; i < VOICE_COUNT; i++){
		if(voicePool[i].on == 0){
		}
		if(voicePool[i].on){
			if(env[i].value < 0xffff - env[i].attack){
				env[i].value += env[i].attack;
			} else{
				env[i].value = 0xffff;
			}
		} else if(voicePool[i].on == 0){
			if(env[i].value >= env[i].release){
				env[i].value -= env[i].release;
			}else{
				env[i].value = 0;
			}
		}
		if(env[i].value > 0){
			voicePool[i].notOver = 1;
		} else {
			voicePool[i].notOver = 0;
		}
	}
}

void Envelope_init(Envelope env[]) {
	for(int i = 0; i < VOICE_COUNT; i++){
		env[i].attack = 250;
		env[i].release = 200;
		env[i].value = 0;
	}
}

void Envelope_setSpeed(Envelope env[],int speed) {
	if(speed == 0){
		for(int i = 0; i < VOICE_COUNT; i++){
			env[i].attack = 250;
			env[i].release = 200;
		}
	}else {
		for(int i = 0; i < VOICE_COUNT; i++){
			env[i].attack = 4;
			env[i].release = 1;
		}
	}
}

void stDelay_tick(stDelay *delay, uint16_t inL, uint16_t inR){
    delay->in_index_l++;								//本Delay的输入采样率固定而输出采样率可变
    if(delay->in_index_l > 2047) delay->in_index_l =0;
    float o_inc_l = delay->samplerate_l/48000;			//计算读取指针位置
    float o_inc_r = delay->samplerate_r/48000;
    delay->out_index_l+=o_inc_l;
    delay->out_index_r+=o_inc_r;
    if(delay->out_index_l > 2047) delay->out_index_l -=2048;
    if(delay->out_index_r > 2047) delay->out_index_r -=2048;
    delay->buffer_l[delay->in_index_l] = inL;
    //从左右声道的指针位置线性插值读出采样，不加插值会很难听
	uint16_t index_l = floor(delay->out_index_l);
	float frac_l = delay->out_index_l - index_l;
	uint16_t nextIndex_l = index_l + 1;
	if (nextIndex_l > 2047){
		nextIndex_l -= 2048;
	}
	uint16_t index_r = floor(delay->out_index_r);
	uint16_t nextIndex_r = index_r + 1;
	float frac_r = delay->out_index_r - index_r;
	if (nextIndex_r > 2047){
		nextIndex_r -= 2048;
	}
	delay->out_l = (uint16_t)((1 - frac_l) * delay->buffer_l[index_l] + frac_l * delay->buffer_l[nextIndex_l]);
	delay->out_r = (uint16_t)((1 - frac_r) * delay->buffer_l[index_r] + frac_r * delay->buffer_l[nextIndex_r]);
}
void stDelay_init(stDelay *delay){
	for(int i = 0; i < 2048; i++){
	    delay->buffer_l[i] = 0;
	}
    delay->in_index_l = 270;
    delay->out_index_l = 0;
    delay->out_index_r = 0;
    delay->samplerate_r = 48000;
    delay->samplerate_l = 48000;
}
void stDelay_timeMod(stDelay *delay, int16_t mod){
	delay->samplerate_l = 48000 + mod;
	delay->samplerate_r = 48000 - mod;
}
