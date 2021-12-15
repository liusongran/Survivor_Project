#include <msp430.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "mcu.h"
/* -----------
 * USER SPACE:
 */

//#define SRT
//#define AR
//#define DIJ
#define BC
//#define RSA
//#define CK
//#define CEM
//#define CRC
//#define BC
//#define FFT
//#define ADPCM


/* ----------
 * ELK SPACE:
 */
#ifdef SRT
#define APP_TASK_NUM 5
void _benchmark_sort_init();
#endif

#ifdef AR
#define APP_TASK_NUM 8
void _benchmark_ar_init();
#endif

#ifdef DIJ
#define APP_TASK_NUM 5
void _benchmark_dijkstra_init();
#endif

#ifdef BC
#define APP_TASK_NUM 11
void _benchmark_bc_init();
#endif


#ifdef CK
#define APP_TASK_NUM 8
void _benchmark_cuckoo_init();
#endif

#ifdef RSA
#define APP_TASK_NUM 8
void _benchmark_rsa_init();
#endif

#ifdef CRC
#define APP_TASK_NUM 4
void _benchmark_crc_init();
#endif

#ifdef CEM
#define APP_TASK_NUM 11
void _benchmark_cem_init();
#endif

#ifdef FFT
#define APP_TASK_NUM 6
void _benchmark_fft_init();
#endif

#ifdef ADPCM
#define APP_TASK_NUM 8
void _benchmark_adpcm_init();
#endif


