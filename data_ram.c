#include <xc.h>
#include "const.h"

uint8_t regALL; //всего регистров оборудования
uint8_t regINP; //регистров оборудования in
uint8_t regOUT; //регистров оборудования Out
uint8_t busINP; //modbus регистров in
uint8_t busOUT; //modbus регистров Out 

//volatile time_flags_t tFlag; // флаги управления
uint8_t debounceCycleCnt; //кол-во системных тиков debounce
uint8_t syncCycleCnt; //кол-во системных тиков цикла SYNC

uint8_t regCnt; //текущий регистр
uint8_t regMask; //маска текущего регистра
uint8_t volatile regAct; //маска активных регистров
uint8_t rdMask; //маска регистров чтения
core_i2c_t wrkReg; //адрес, данные для/от текущего регистра

uint8_t timers[8 * TM_REG];
COMMON_DATA volatile COMMON;
// DEBOUNCE MASTER_DATA
core_data_t coreData[8]; //рабочая область для debounce регистров (макс 8)

void (*loopFunc)(void);
void (*loopStep)(void);

