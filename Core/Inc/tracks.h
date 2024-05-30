/*
 * tracks.h
 *	这个文件本来想用于实现MIDI播放相关函数，但是后来发现并不需要，只留下了类定义
 *  Created on: May 28, 2024
 *      Author: chenfeng
 */

#ifndef INC_TRACKS_H_
#define INC_TRACKS_H_
#include "stdint.h"

#ifndef Note_on_c
#define Note_on_c 1
#endif

#ifndef Note_off_c
#define Note_off_c 0
#endif

#ifndef End_track
#define End_track 9
#endif

#ifndef Control_c
#define Control_c 2
#endif

#ifndef midi_file
typedef struct {
    uint32_t time;
    uint8_t msg;
    uint8_t channel;
    uint8_t note;
    uint8_t velocity;
} midi_file;
#endif

#endif /* INC_TRACKS_H_ */
