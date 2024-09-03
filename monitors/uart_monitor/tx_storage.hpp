#pragma once

#include <array>
#include <cstdint>
#include <ranges>
#include <cstring>
#include <complex>

#include "embedded_hw_utils/utils/storages/tx_storage.hpp"

namespace connectivity::uart::monitor{

template<std::size_t storage_size, bool use_crc>
struct TxStorage: utils::TxStorage<storage_size, use_crc> {
    using BaseStorage = utils::TxStorage<storage_size, use_crc>;

    template<typename ...Args>
    void PlaceToStorage(Args&&... args){
        (CheckSpecialT(std::forward<Args>(args)), ...);
    }

    template<typename T>
    void CheckSpecialT(T&& t){
        if constexpr (std::is_integral_v<std::remove_cvref_t<T>> || std::is_floating_point_v<std::remove_cvref_t<T>>)
            PlaceValue(std::forward<T>(t));
        else if constexpr (std::is_enum_v<std::remove_cvref_t<T>>)
            PlaceValue(static_cast<int>(t));
        else
            BaseStorage::PlaceValue(std::forward<T>(t));
    }
//    template<typename T, typename Trr=std::remove_cvref_t<T>>
//    void CheckSpecialT(T&& t){
//        if constexpr (std::is_integral_v<Trr> || std::is_floating_point_v<Trr>)
//            PlaceValue(std::forward<T>(t));
//        else if constexpr (std::is_enum_v<Trr>)
//            PlaceValue(static_cast<int>(t));
//        else
//            BaseStorage::PlaceValue(std::forward<T>(t));
//    }
protected:
    template<typename T>
    void PlaceValue(T&& num)
        requires(std::is_floating_point_v<std::remove_cvref_t<T>>)
    {
        assert(BaseStorage::FitsInRange(sizeof(num)));
        if(num < 0){
            BaseStorage::StoreByte('-');
            num = -num;
        }
        static constexpr float kPrecision = 0.1;
        int cursor = std::log10(num);
        int digit;
        while (num > kPrecision)
        {
            auto weight = std::pow(10.0f, cursor);
            digit = std::floor(num / weight);
            num -= digit * weight;
            BaseStorage::StoreByte('0' + digit);
            if(cursor == 0)
                BaseStorage::StoreByte('.');
            cursor--;
        }
        BaseStorage::StoreByte('0');
    }

    template<typename T>
    void PlaceValue(T&& num)
        requires(std::is_integral_v<std::remove_cvref_t<T>>)
    {
        assert(BaseStorage::FitsInRange(sizeof(T)));
        if constexpr(std::is_same_v<std::remove_reference_t<T>, char>){
            BaseStorage::StoreByte(num);
            return;
        }
        auto ptr = std::bit_cast<char*>(std::ranges::next(BaseStorage::data_.begin(), BaseStorage::cursor()));
        auto bytes_written = std::sprintf(ptr, "%d", num);
        if(bytes_written > 0)
            BaseStorage::MoveCursor(bytes_written);
    }
};

}//namespace connectivity::uart::monitor
