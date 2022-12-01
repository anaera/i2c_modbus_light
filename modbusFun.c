//#define DREG_CNT  8  /*!< Количество цифровых регистров в пакете (ограничим -8) */
#include "const.h"
#include "modbus.h"
#include "proc_epp.h"
extern uint8_t regALL; //всего регистров оборудования
extern uint8_t busINP; //modbus регистров in
extern uint8_t busOUT; //modbus регистров Out 
extern COMMON_DATA volatile COMMON;

//------------------------------------------------------------------------------
/**
 * \file
 * \brief Файл реализаций всех стандартных функций протокола MODBUS.
 * 
 * Специфика данной реализации для 8 битных микроконтроллеров заключается
 * в приведении всех 16 битных параметров к 8 битным, посредством усечения
 * старшей 8 битной части. Предварительно проверяется равентсво ее нулю
 * и формирования пакета ошибки в противном случае.
 */
//------------------------------------------------------------------------------
/// Функция чтении мультирегистров DO
/**
 * 01 (0x01)	Чтение DO	Read Coil Status	Дискретное	Чтение
 */
//------------------------------------------------------------------------------

void readCoilStatus01(void) {
    readDigitalReg(DREG_CNT, busINP, (uint8_t) (busOUT << 3));
}
//------------------------------------------------------------------------------
/// Функция чтении мультирегистров DI
/**
 * 02 (0x02)	Чтение DI	Read Input Status	Дискретное	Чтение
 */
//------------------------------------------------------------------------------

void readInputStatus02(void) {
    readDigitalReg(DREG_CNT, 0, (uint8_t) (busINP << 3));
}
//------------------------------------------------------------------------------
/// Функция чтении мультирегистров любого цифрового регистра (DI или DO)
/**
 * Читаются регистры из области определякмой параметрами offInDevice и regInDevice
 * \param [in] regInPack количество регистров разрешенных в запросе (в битах)
 * \param [in] offInDevice смещение читаемой группы регистров в общей области памяти (в байтах)
 * \param [in] regInDevice общее количество регистров данного типа в устрорйстве (в битах)
 * 
 * для регистров DO (regInPack = DREG_CNT; offInDevice = 0; regInDevice = DO_REG)
 * 
 * для регистров DI (regInPack = DREG_CNT; offInDevice = AO_REG; regInDevice = DI_REG)
 */
//------------------------------------------------------------------------------

void readDigitalReg(char regInPack, char offInDevice, char regInDevice) {
    if (MODBUS.buffer[4] == 0) { //hiNum == 0
        if (MODBUS.buffer[2] == 0) { //hiAddr == 0
            if (MODBUS.buffer[5] < regInPack + 1) { //loNum < 8
                //loAddr+loNum << maxNumReg
                if ((MODBUS.buffer[3] + MODBUS.buffer[5]) < regInDevice + 1) {
                    char hiTag;
                    char loTag;
                    char off;
                    unsigned int val;
                    loTag = MODBUS.buffer[3] & 0x07; //младшая часть адреса первого регистра
                    hiTag = offInDevice + (MODBUS.buffer[3] >> 3); //старшая часть адреса первого регистра

                    if (hiTag == ((busINP + busOUT) - 1)) {
                        if (loTag == 0) { //адрес кратный 8
                            if (MODBUS.buffer[5] == 8) { //  длина = 8
                                COMMON.registrTable[hiTag] = DATAEE_ReadByte(COMMON.registrTable[hiTag - 1]);
                            }
                        }
                    }

                    val = (unsigned int) (COMMON.registrTable[hiTag + 1] << 8); //старшие биты облласти регистров
                    val = val | COMMON.registrTable[hiTag]; //младшие биты облласти регистров
                    val = val >> loTag; //смещаем к нулевому биту (вправо))
                    off = (MODBUS.buffer[5] & 0x07);
                    if (off == 0) {
                        off = 8;
                    }
                    MODBUS.buffer[3] = (char) (val & (~(unsigned int) (0xFF << off))); //!!
                    MODBUS.buffer[4] = 0;
                    MODBUS.buffer[2] = 2; //количество байт в пакете
                    MODBUS.sendLen = 7;
                    return;
                }
            }
        }
    }
    makeErrorFrame(0x02); //область памяти не поддерживается
    return;
}
//------------------------------------------------------------------------------
/// Функция записи одного регистра DO
/**
 * 05 (0x05)	Запись одного DO	Force Single Coil	Дискретное	Запись
 */
//------------------------------------------------------------------------------

void forceSingleCoil05(void) {
    if (MODBUS.buffer[2] == 0) {
        if (MODBUS.buffer[3] < ((uint8_t) (busOUT << 3))) { //номер бита в диапазоне
            //в нашем адресном пространстве
            if (MODBUS.buffer[5] == 0) {
                unsigned char numBit, numByte;
                numBit = MODBUS.buffer[3]&7;
                numByte = MODBUS.buffer[3] >> 3;
                numByte = numByte + busINP;
                MODBUS.sendLen = 8;
                if (MODBUS.buffer[4] == 0) {
                    COMMON.registrTable[numByte] = COMMON.registrTable[numByte] & (~(unsigned char) (1 << numBit));
                    return;
                }
                if (MODBUS.buffer[4] == 0xFF) {
                    COMMON.registrTable[numByte] = COMMON.registrTable[numByte] | (unsigned char) (1 << numBit);
                    return;
                }
            }
            //недопустимое значение данных
            makeErrorFrame(0x03);
            return;
        }
    }
    //область памяти не поддерживается
    makeErrorFrame(0x02); //область памяти не поддерживается
}
//------------------------------------------------------------------------------
/// Функция записи мультирегистров DO
/**15 (0x0F)	Запись нескольких DO	Force Multiple Coils	Дискретное	Запись
 */
//------------------------------------------------------------------------------

void forceMultipleCoils15(void) {
    if (MODBUS.buffer[4] == 0) { //HI кол-во регистров
        if (MODBUS.buffer[2] == 0) { //HI адрес первого регистра
            if (MODBUS.buffer[5] < DREG_CNT + 1) { //LO кол-во регистров < DREG_CNT + 1
                if ((MODBUS.buffer[3] + MODBUS.buffer[5]) < (((uint8_t) (busOUT << 3)) + 1)) { //LO адрес последнего регистра < DO_REG + 1
                    char hiTag;
                    char loTag;
                    //                    unsigned int val;
                    //                    unsigned int mask;
                    unsigned int tag;

                    loTag = MODBUS.buffer[3] & 0x07; //младшая часть адреса первого регистра
                    //                   if (loTag == 0) {
                    //                     loTag = 8;
                    //               }
                    //                    mask = ((~(0xFFFF << MODBUS.buffer[5])) << loTag);
                    //                    mask = mask<<loTag;
                    //                    val = MODBUS.buffer[7] << loTag;
                    //                    val = val<<loTag;

                    hiTag = busINP + (uint8_t) (MODBUS.buffer[3] >> 3); //старшая часть адреса первого регистра 

                    if (hiTag > ((busINP + busOUT) - 3)) {
                        if (loTag == 0) { //адрес кратный 8
                            if (MODBUS.buffer[5] == 8) { //  длина = 8
                                COMMON.registrTable[hiTag] = MODBUS.buffer[7];
                                if (hiTag == ((busINP + busOUT) - 1)) {
                                    DATAEE_WriteByte(COMMON.registrTable[hiTag - 1], COMMON.registrTable[hiTag]);
                                }
                                MODBUS.sendLen = 8; //?????????
                                return;
                            }
                        }
                        makeErrorFrame(0x02); //область памяти не поддерживается  
                    } else {
                        tag = COMMON.registrTable[hiTag + 1]; //старшие биты области регистров
                        tag = tag << 8;
                        tag = tag + COMMON.registrTable[hiTag]; //младшие биты области регистров

                        //                    tag = (tag & (~mask)) | val;
                        tag = (tag & (~(unsigned char) ((~(0xFFFF << MODBUS.buffer[5])) << loTag))) | (unsigned char) (MODBUS.buffer[7] << loTag);
                        COMMON.registrTable[hiTag + 1] = (unsigned char) (tag >> 8);
                        COMMON.registrTable[hiTag] = (unsigned char) tag;

                        MODBUS.sendLen = 8;
                        return;

                    }

                }
            }
        }
    }
    makeErrorFrame(0x02); //область памяти не поддерживается
    return;
}

