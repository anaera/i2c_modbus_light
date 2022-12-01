#ifndef TIMERS_H
#define	TIMERS_H
#include <xc.h>
typedef union {
    unsigned char flag; /*!< байт флагов */

    struct {
        unsigned TIKFLG : 1; /*!< 1мс флаг */
        unsigned STPFLG : 1; /*!< 10мс флаг */
        unsigned SECFLG : 1; /*!< 1сек флаг */

    };
} TIMERS_FLAG;

typedef struct {
    TIMERS_FLAG flagTimers; /*!< таймерные флаги */
    unsigned char stpTime; /*!< счетчик 10мс */
    unsigned char secTime; /*!< счетчик 1 сек */
} TIMERS_DATA;

TIMERS_DATA TIMERS;


void startTimers(void);
inline void timeCount(void);

#endif	/* TIMERS_H */

