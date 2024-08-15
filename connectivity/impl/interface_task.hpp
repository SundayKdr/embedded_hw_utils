#pragma once

#include <array>
#include <optional>
#include <cstring>

#include "embedded_hw_utils/utils/task_callback.hpp"
#include "embedded_hw_utils/utils/tx_data_pair.hpp"

namespace connectivity{
using CB = utils::task::CallBack<uint8_t*>;

enum TaskType{
    transmit,
    receive,
    transmit_receive
};

enum class TaskState{
    free,
    in_process,
    pending
};

template<std::size_t tx_buffer_size, std::size_t rx_buffer_size = 8, typename CB = CB>
struct InterfaceTask{
    void PlaceData(utils::TxData data){
        if(data.size <= sizeof tx_data_)
            std::memcpy(tx_data_.data(), data.data_ptr, data.size);
    }

    void CallBack(){
        if(call_back_)
            (*call_back_)(rx_data_.data());
    }
    auto& TxStorage(){ return tx_data_; }
    uint8_t* TxData(){ return tx_data_.data(); }
    uint8_t* RxData(){ return rx_data_.data(); }
    TaskType Type(){ return type_; }
    [[nodiscard]] std::size_t TxSize() const { return tx_size_; }
    [[nodiscard]] std::size_t RxSize() const { return rx_size_; }
    void setState(TaskState state){ state_ = state; }
    auto isFree(){ return state_ == TaskState::free; }
    auto isInProcess(){ return state_ == TaskState::in_process; }
    auto isPending(){ return state_ == TaskState::pending; }

    InterfaceTask() = default;

    InterfaceTask(utils::TxData data)
        :tx_size_(data.size)
        ,type_(transmit)
    {
        PlaceData(data);
    }

    InterfaceTask(utils::TxData data, CB call_back)
        :call_back_(call_back)
        ,tx_size_(data.size)
        ,type_(transmit)
    {
        PlaceData(data);
    }

    InterfaceTask(std::size_t rx_size, CB call_back)
        :call_back_(call_back)
        ,rx_size_(rx_size)
        ,type_(receive)
    {}

    InterfaceTask(std::size_t rx_size)
        :rx_size_(rx_size)
        ,type_(receive)
    {}

    InterfaceTask(utils::TxData data, std::size_t rx_size, CB call_back)
        :call_back_(call_back)
        ,rx_size_(rx_size)
        ,tx_size_(data.size)
        ,type_(transmit_receive)
    {
        PlaceData(data);
    }
protected:
    std::array<uint8_t, rx_buffer_size> rx_data_{0,};
    std::array<uint8_t, tx_buffer_size> tx_data_{0,};
    std::size_t rx_size_;
    std::size_t tx_size_;
private:
    TaskType type_;
    TaskState state_{TaskState::free};
    std::optional<CB> call_back_;
};
} //namespace connectivity