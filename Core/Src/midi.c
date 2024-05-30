/*
 * midi.c
 *
 *  Created on: May 20, 2024
 *      Author: chenfeng
 */

#include "midi.h"
extern volatile uint32_t trackTime;
extern volatile uint32_t trackIndex;
extern const midi_file track1[];
// 在新音符被演奏时调用
int voicePool_noteOn(uint8_t note, volatile voice voicePool[]) {
    int i;
    int maxTimeIndex = 0;
    uint8_t maxTime = 0;

    // 遍历voicePool中所有的voice
    for (i = 0; i < 6; i++) {	//尝试分配当前未发声的voice
        // 如果复音正在使用，则time++
        if (voicePool[i].notOver == 1 || voicePool[i].on == 1) {
            voicePool[i].time++;
        } else {					// 未使用的复音
        	voicePool[i].note = note;
        	voicePool[i].on = 1;
        	voicePool[i].time = 0; // 新音符的时间戳归零
        	return i;
        }
    }

    for (i = 0; i < 6; i++) {//尝试分配当前未使用的voice
        // 如果复音正在使用，则time++
        if (voicePool[i].on != 1) {// 未使用的复音
        	voicePool[i].note = note;
        	voicePool[i].on = 1;
        	voicePool[i].time = 0; // 新音符的时间戳归零
        	return i;
        }
    }
    // 如果没有未使用的复音，则找到time值最大的复音
    for (i = 0; i < 6; i++) {
        if (voicePool[i].time > maxTime) {
            maxTime = voicePool[i].time;
            maxTimeIndex = i;
        }
    }

    // 替换time值最大的复音
    voicePool[maxTimeIndex].note = note;
    voicePool[maxTimeIndex].on = 1;
    voicePool[maxTimeIndex].time = 0; // 新音符的时间戳归零
    return maxTimeIndex;
}

// 在音符停止演奏时调用，遍历voicePool，找到note值与传入note值相同的voice停止
void voicePool_noteOff(uint8_t note, volatile voice voicePool[]) {
    int i;
    for (i = 0; i < 6; i++) {
        if (voicePool[i].note == note && voicePool[i].on == 1) {
            voicePool[i].on = 0;
            voicePool[i].time = 0; // 停止演奏后time归零
        }
    }
}
void voicePool_panic(){				//一键停止所有音符
    for (int i = 0; i < 6; i++) {
            voicePool[i].on = 0;
            voicePool[i].time = 0;
    }
}
void midi_cc(uint8_t cc, uint8_t value){
	switch(cc){
	case 1: LFO1_Rate = (float)(0.5 + 10.0f * value/127.0f);break;
	case 14: B3_OscVol[0] = (uint8_t)(value/14);break;
	case 15: B3_OscVol[1] = (uint8_t)(value/14);break;
	case 16: B3_OscVol[2] = (uint8_t)(value/14);break;
	case 17: B3_OscVol[3] = (uint8_t)(value/14);break;
	case 18: B3_OscVol[4] = (uint8_t)(value/14);break;
	case 19: B3_OscVol[5] = (uint8_t)(value/14);break;
	case 20: B3_OscVol[6] = (uint8_t)(value/14);break;
	case 21: B3_OscVol[7] = (uint8_t)(value/14);break;
	case 22: B3_OscVol[8] = (uint8_t)(value/14);break;
	}
}

void midi_setTiming(uint32_t tickrate, uint32_t tempo){
	htim7.Init.Period = 5208;
	HAL_TIM_Base_Init(&htim7);
}

void midi_playFile(const midi_file track[]){
	midi_setTiming(96, 500000);
	HAL_TIM_Base_Start_IT(&htim7);
}

void midi_playFile_tick(const midi_file track1[]){
	if(track1[trackIndex].time <= trackTime){
		if(track1[trackIndex].msg == Note_on_c){
			voicePool_noteOn(track1[trackIndex].note, voicePool);
			trackIndex++;
		}else if(track1[trackIndex].msg == Note_off_c){
			voicePool_noteOff(track1[trackIndex].note, voicePool);
			trackIndex++;
		}else if(track1[trackIndex].msg == End_track){
			HAL_TIM_Base_Stop_IT(&htim7);
			trackIndex = 0;
			trackTime = 0;
		}else if(track1[trackIndex].msg == Control_c){
			midi_cc(track1[trackIndex].note, track1[trackIndex].velocity);
			trackIndex++;
		}
		midi_playFile_tick(track1);
	} else{
		return;
	}
}
