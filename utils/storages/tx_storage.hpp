#pragma once

#include <array>
#include <cstdint>
#include <ranges>
#include <cstring>

#include "embedded_hw_utils/utils/rx_packet.hpp"
#include "embedded_hw_utils/utils/tx_data_pair.hpp"

namespace utils{

template<std::size_t storage_size, bool crc_packet>
struct TxStorage{
    static constexpr std::size_t dlc_width = 1;
    auto& at(auto idx){
        return data_.at(idx);
    }
    auto dataView(){
        return std::ranges::views::counted( data_.begin(), storage_size );
    }
    void Reset(){
        cursor_ = dlc_width;
    }
    bool FitsInRange(std::size_t amount){
        return cursor() + amount <= data_.size();
    }
    auto currentDataIt(){
        return std::ranges::next(data_.begin(), cursor());
    }

    auto cursor(){
        if constexpr (crc_packet)
            return cursor_;
        else
            return cursor_ - dlc_width;
    }

    auto& data(){
        return data_;
    }

    auto dataPtr(){
        auto ptr = data_.data();
        if constexpr(crc_packet)
            return ptr;
        else
            return ptr++; //dlc_width
    }

    auto MakeTxData(){
        if constexpr(crc_packet){
            PlaceCRC();
            PlaceDlc();
        }
        return TxData{dataPtr(), cursor()};
    }

    std::size_t size(){
        return data_.size();
    }
    template<typename ...Args>
    void PlaceToStorage(Args&&... args){
        (PlaceValue(std::forward<Args>(args)), ...);
    }
    template<typename ...Args>
    void StoreBytes(Args&&... args){
        (StoreByte(std::forward<Args>(args)), ...);
    }

protected:
    std::size_t cursor_{dlc_width};
    std::array<uint8_t, storage_size> data_{};

    void MoveCursor(int shift){
        cursor_ += shift;
    }

    void StoreByte(uint8_t byte){
        assert(FitsInRange(1));
        data_[cursor()] = byte;
        MoveCursor(1);
    }

    template<typename T, std::size_t N>
    void PlaceValue(T(&array)[N])
    {
        auto size = sizeof(T) * N - 1;
        assert(FitsInRange(size));
        std::memcpy(currentDataIt(), array, size);
        MoveCursor(size);
    }

    template<typename T>
    void PlaceValue(T&& array)
        requires(std::is_bounded_array<T>::value)
    {
        auto size = sizeof(T) * array.size() - 1;
        assert(FitsInRange(size));
        std::memcpy(currentDataIt(), array.begin(), size);
        MoveCursor(size);
    }

    template<typename T>
    void PlaceValue(T&& num)
    {
        assert(FitsInRange(sizeof(T)));
        auto size = sizeof(T);
        std::memcpy(currentDataIt(), &num, size);
        MoveCursor(size);
    }

    template<typename ...Arrays>
    void PlaceArraysToStorage(Arrays&&... arrays){
        auto add_to_storage = [&]<typename T, std::size_t N>(T(&array)[N]){
            PlaceValue(std::forward<decltype(array)>(array));
        };
        (add_to_storage(arrays), ...);
    }

    void PlaceDlc(){
        data_[0] = cursor() - dlc_width;
    }

    void PlaceCRC(){
        auto payload_section_it = std::ranges::next(data_.begin(), dlc_width);
        auto payload_size = cursor() - dlc_width;
        auto crc = computation::crc::CalcCRC(payload_section_it, payload_size);

        auto crc_f_b = crc >> 8 & 0xff;
        auto crc_s_b = crc & 0xff;
        StoreBytes(crc_s_b, crc_f_b);
    }
};

}//namespace utils
