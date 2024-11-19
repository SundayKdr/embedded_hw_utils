#pragma once

#include <array>
#include <cstring>
#include <ranges>

namespace utils{
template<std::size_t storage_size>
struct RxStorage {
    auto dataView(){
        return std::ranges::views::counted( storage_.begin(), rx_size_ );
    }
    auto data(){
        return storage_.data();
    }
    std::size_t size(){
       return storage_.size();
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
    std::array<uint8_t, storage_size> storage_;
};

}//namespace utils
