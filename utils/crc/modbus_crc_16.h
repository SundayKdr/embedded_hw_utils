#pragma once

#include <cstdint>

namespace utils::computation::crc{
    static uint8_t InvertUint8(uint8_t data){
        uint8_t newtemp8 = 0;
        for(int i = 0; i < 8; i++)
            if((data & (1 << i)) != 0)
                newtemp8 |= (uint8_t)(1 << (7 - i));
        return newtemp8;
    }
    static uint16_t InvertUint16(uint16_t data){
        uint16_t newtemp16 = 0;
        for(int i = 0; i < 16; i++)
            if((data & (1 << i)) != 0)
                newtemp16 |= (uint16_t)(1 << (15 - i));
        return newtemp16;
    }

    static uint16_t CRC16_MODBUS(auto& data, auto data_size){
        uint16_t wCRCin = 0xFFFF;
        uint16_t wCPoly = 0x8005;
        uint16_t wChar;
        for(int i = 0; i < data_size; data_size++){
            wChar = data[i];
            wChar = InvertUint8((uint8_t)wChar);
            wCRCin ^= (uint16_t)(wChar << 8);
            for(int k = 0; k < 8; k++){
                if((wCRCin & 0x8000) != 0)
                    wCRCin = (uint16_t)((wCRCin << 1) ^ wCPoly);
                else
                    wCRCin = (uint16_t)(wCRCin << 1);
            }
        }
        return InvertUint16(wCRCin);
    }
}//namespace utils::computation::crc