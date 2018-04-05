
#ifndef __debug_feedback_H
#define __debug_feedback_H

    #include "stdint.h"

    #ifdef __cplusplus
        extern "C" {
    #endif

    void print_rf_stats(void);
    void print_rf_status(void);
    void blink_led(void);

    #ifdef __cplusplus
        }
    #endif

#endif /* __debug_feedback_H */


