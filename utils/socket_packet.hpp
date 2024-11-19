#pragma once

#include <array>
#include <cstdint>
#include <ranges>

#include "embedded_hw_utils/utils/computation/crc/crc16.hpp"

namespace utils{

template< std::size_t storage_size >
struct SocketPacket{
    static constexpr std::size_t dlc_width = 1;
    static constexpr std::size_t service_data_size = dlc_width + computation::crc::crc16_width;

    void PlaceData(auto data_view){
        for(uint8_t byte: data_view)
            PlaceByteToStorage(byte);
    }

    void Reset(){
        cursor_ = 0;
        crc_pass_ = false;
    }

    auto GetPayloadView() {
        return std::ranges::views::counted( std::ranges::next(storage_.begin(), dlc_width), dlc_ );
    }

    bool CheckCRC(){
        auto expected_crc = GetCRCFromPack();
        auto payload_section_it = std::ranges::next(storage_.begin(), dlc_width);
        auto calc_crc = computation::crc::CalcCRC(payload_section_it, dlc_);
        return expected_crc == calc_crc;
    }

    [[nodiscard]] const auto& data() const{ return storage_; }
    [[nodiscard]] bool isReady() const{ return crc_pass_;}
    [[nodiscard]] bool isFull() const{
        return cursor_ == dlc_ + service_data_size;
    }
private:
    bool crc_pass_{false};
    std::size_t dlc_{0};
    std::size_t cursor_{0};
    std::array<uint8_t, storage_size> storage_{};

    void PlaceByteToStorage(uint8_t byte){
        if(cursor_ == 0)
            dlc_ = byte;
        else
            storage_[cursor_] = byte;
        cursor_++;
    }

    uint16_t GetCRCFromPack(){
        auto packet_end_it = std::ranges::next(storage_.begin(), dlc_ + service_data_size);
        auto start_byte = *std::prev(packet_end_it, 1);
        auto end_byte = *std::prev(packet_end_it, 2);
        return (start_byte << 8) | end_byte;
    }

};
}// namespace monitor