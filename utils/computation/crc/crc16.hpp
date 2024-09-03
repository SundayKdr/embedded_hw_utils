#pragma once

namespace utils::computation::crc{
    static constexpr std::size_t crc16_width = 2;

    static uint16_t CalcCRC(auto payload_section_it, std::size_t payload_s){
        uint16_t crc = 0;
        for(std::size_t i = 0; i < payload_s; ++i, payload_section_it++)
            crc += *payload_section_it;
        crc = ((~crc + 1) & 0xffff);
        return crc;
    }
}