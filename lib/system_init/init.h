
#ifndef __init_H
#define __init_H

    // #include "stdint.h"

    #ifdef __cplusplus
        extern "C" {
    #endif

    void stm32_peripherals_init(void);
    void mote_init(void);
    static void SystemClock_Config(void);

    #ifdef __cplusplus
        }
    #endif

#endif /* __init_H */





