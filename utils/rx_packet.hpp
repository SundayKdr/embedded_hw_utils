#pragma once

#include <array>
#include <cstdint>
#include <ranges>

#include "embedded_hw_utils/utils/crc/crc16.hpp"

namespace utils{

template< std::size_t payload_size >
struct RxPacket{
    static constexpr std::size_t dlc_width = 1;
    static constexpr std::size_t pack_size = payload_size + dlc_width + computation::crc::crc16_width;

    void PlaceData(auto data_view){
        for(auto& byte: data_view)
            PlaceByteToStorage(byte);
    }

    void PlaceData(uint8_t byte){
        PlaceByteToStorage(byte);
    }

    void Reset(){
        cursor_ = 0;
        crc_pass_ = false;
    }

    auto GetPayloadView() { return std::ranges::views::counted(std::ranges::next(storage_.begin(), dlc_width),
                                                       pack_size ); }
    [[nodiscard]] const auto& data() const{ return storage_; }
    [[nodiscard]] bool isReady() const{ return crc_pass_;}

private:
    bool crc_pass_{false};
    std::size_t dlc_{0};
    std::size_t cursor_{0};
    std::array<uint8_t, pack_size> storage_{};

    void PlaceByteToStorage(uint8_t byte){
        if(!isFull())
            storage_[cursor_++] = byte;
        if(isFull())
            CheckCRC();
    }

    void CheckCRC(){
        dlc_ = storage_.front();
        auto expected_crc = GetCRCFromPack();
        auto payload_section_it = std::ranges::next(storage_.begin(), dlc_width);
        auto calc_crc = computation::crc::CalcCRC(payload_section_it, payload_size);

        if(expected_crc == calc_crc)
            crc_pass_ = true;
        else{
            crc_pass_ = false;
            Reset();
        }
    }

    uint16_t GetCRCFromPack(){
        auto packet_end_it = std::ranges::next(storage_.begin(), pack_size);
        auto start_byte = std::prev(packet_end_it, 1);
        auto end_byte = std::prev(packet_end_it, 2);
        return (*start_byte << 8) | *end_byte;
    }

    [[nodiscard]] bool isFull() const{
        return cursor_ == pack_size;
    }
};
}// namespace monitor