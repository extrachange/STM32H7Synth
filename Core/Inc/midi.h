/*
 * midi.h
 *
 *  Created on: May 20, 2024
 *      Author: chenfeng
 */

#ifndef SRC_MIDI_H
#define SRC_MIDI_H

#ifndef VOICE_COUNT
#define VOICE_COUNT 6
#endif

#include "stdint.h"
#include <math.h>
#include "tracks.h"
#include "stm32h7xx_hal.h"

typedef struct {
    uint8_t note; // MIDI消息中的音符值
    uint8_t on;   // 复音的使用情况
    uint8_t time;// 复音的时间戳
    uint8_t notOver;//包络尚未结束
} voice;

extern volatile uint8_t B3_OscVol[9];
extern volatile voice voicePool[VOICE_COUNT];
extern float LFO1_Rate;
extern TIM_HandleTypeDef htim7;

void midi_init();
int voicePool_noteOn(uint8_t note, volatile voice voicePool[]);		//返回值是新分配音符的复音池索引
void voicePool_noteOff(uint8_t note, volatile voice voicePool[]);
void voicePool_panic();

void midi_cc(uint8_t cc, uint8_t value);					//响应CC输入
void midi_setTiming(uint32_t tickrate, uint32_t tempo);		//用定时器播放midi文件，应该能用，暂时弃用
void midi_playFile(const midi_file track[]);
void midi_playFile_tick(const midi_file track[]);

#endif /* SRC_MIDI_H_ */
