#ifndef DATA
#define DATA

#include "client.h"

#define SAMPLES 1000

extern vec3f_t DAT_MAG_RAW[SAMPLES];
extern vec3f_t DAT_MAG_CAL[SAMPLES];
extern vec3f_t DAT_MAG_EST[SAMPLES];
extern vec3f_t DAT_ACC_CAL;
extern sysSnap_t DAT_SNAPS[SAMPLES];
extern int DAT_CUR_IDX;

#endif