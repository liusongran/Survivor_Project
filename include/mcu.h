#ifndef MCU_H_
#define MCU_H_
#include <driverlib.h>

#define __nv            __attribute__((section(".nv_vars")))
#define __elk           __attribute__((section(".elk_vars")))
#define __elk_du        __attribute__((section(".elk_vars")))
#define __elk_cst       __attribute__((section(".elk_cst_vars")))
#define __sub_cksum     __attribute__((section(".elk_sub_cksum")))

#define __sv            __attribute__((section(".sv_vars")))
#define __glbal         __attribute__((section(".elk_glbal_vars")))

#endif /* MCU_MCU_H_ */
