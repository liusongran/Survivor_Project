#define __nv        __attribute__((section(".nv_vars")))
#define __sv        __attribute__((section(".sv_vars")))    //sram-volatile variables
#define __elk       __attribute__((section(".elk_vars")))
#define __elk_du    __attribute__((section(".elk_du_vars")))
