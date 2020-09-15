#include <msp430.h>
#include <stdint.h>
#include <driverlib.h>
#include "simulator.h"

extern uint8_t elkCurMode;
extern uint8_t elkClearMark;
extern uint16_t svVrfiedBp;

void __simu_init() {
    //LOG: power-on signal <--- P3.1.
    GPIO_selectInterruptEdge(GPIO_PORT_P3, GPIO_PIN1, GPIO_LOW_TO_HIGH_TRANSITION);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P3, GPIO_PIN1);
    //GPIO_clearInterrupt(GPIO_PORT_P3, GPIO_PIN1);
    P3IFG = 0;
    //LOG: power-warning   <--- P4.1.
    GPIO_selectInterruptEdge(GPIO_PORT_P4, GPIO_PIN1, GPIO_LOW_TO_HIGH_TRANSITION);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN1);
    //GPIO_clearInterrupt(GPIO_PORT_P4, GPIO_PIN1);
    P4IFG = 0;
    //LOG: power-off       <--- P5.1.
    GPIO_selectInterruptEdge(GPIO_PORT_P5, GPIO_PIN1, GPIO_LOW_TO_HIGH_TRANSITION);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P5, GPIO_PIN1);
    //GPIO_clearInterrupt(GPIO_PORT_P5, GPIO_PIN1);
    P5IFG = 0;
    //LOG: bit-flip        <--- P1.2~P1.5.
    //GPIO_selectInterruptEdge(GPIO_PORT_P1, GPIO_PIN2, GPIO_LOW_TO_HIGH_TRANSITION);
    //GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN2);
    //GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN2);

    GPIO_enableInterrupt(GPIO_PORT_P3, GPIO_PIN1);
    GPIO_enableInterrupt(GPIO_PORT_P4, GPIO_PIN1);
    GPIO_enableInterrupt(GPIO_PORT_P5, GPIO_PIN1);
    //GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN2);
}

/* ------------------
 * LOG: power-warning   <--- P4.1.
 */
/*
#pragma vector=PORT4_VECTOR
__interrupt void port4IsrHandler(void) {
    elkCurMode = 1;
    elkClearMark = 1;
}
*/
