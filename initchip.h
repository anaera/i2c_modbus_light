
#ifndef INITCHIP_H
#define	INITCHIP_H

#include <xc.h> // include processor files - each processor file is guarded.  


#include "const.h"


void initCore(void);
void initDrive(void);
void initCPU(void);
void initANALOG(void);
void initPORTA(void);
void setPINS(void);
void initUART(uint8_t speed);
void initI2C(void);
void initTMR0(void);
void initTMR1(void);
void initWDT(void);
void initINT(void);

#endif	/* INITCHIP_H */

