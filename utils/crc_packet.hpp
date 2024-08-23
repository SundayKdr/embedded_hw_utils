#pragma once

#include <array>
#include <cstdint>
#include <ranges>

namespace utils{
template<
        std::size_t payload_size,
        std::size_t service_width_at_begin
        >
struct Packet{
    static constexpr std::size_t crc_width = 2;
    static constexpr std::size_t pack_size = payload_size + service_width_at_begin + crc_width;

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

    auto GetPayloadView() { return std::ranges::views::counted(std::ranges::next(storage_.begin(), service_width_at_begin),
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
        auto calc_crc = CalcCRC();

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

    [[nodiscard]]uint16_t CalcCRC() const{
        uint16_t crc = 0;
        auto payload_section_it = std::ranges::next(storage_.begin(), service_width_at_begin);
        for(std::size_t i = 0; i < payload_size; ++i, payload_section_it++)
            crc += *payload_section_it;
        crc = ((~crc + 1) & 0xffff);
        return crc;
    }
};
}// namespace monitor