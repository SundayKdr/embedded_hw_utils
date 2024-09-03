#pragma once

#include <array>
#include <cstring>
#include <ranges>

namespace utils{
template<std::size_t storage_size>
struct RxStorage {
    auto dataView(auto size){
        return std::ranges::views::counted( data_.begin(), size );
    }
    auto& data(){
        return data_;
    }
    std::size_t size(){
       return data_.size();
    }
    bool isReady(){
       return ready_;
    }
    void setReady(auto size){
        ready_ = true;
        rx_size_ = size;
    }
    void setPending(){
        ready_ = false;
    }
    void setRxSize(auto size){
        rx_size_ = size;
    }
    auto getRxSize(){
        return rx_size_;
    }
private:
    bool ready_{false};
    uint16_t rx_size_{0};
    std::array<uint8_t, storage_size> data_;
};

}//namespace utils
