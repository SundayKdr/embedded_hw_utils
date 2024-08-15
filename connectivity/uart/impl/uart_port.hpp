#pragma once

#include "uart_task.hpp"
#include "embedded_hw_utils/connectivity/impl/interface_port.hpp"
#include "embedded_hw_utils/utils/storages/rx_storage.hpp"

namespace connectivity::uart{

struct Port final: InterfacePort<HandleT, Task, tasks_queue_size>{
    using RxStorage = utils::RxStorage<rx_storage_size>;
    void ErrorHandler(){
        assert(false);
//        __HAL_UART_CLEAR_IT(handle_, UART_CLEAR_FEF);
//        __HAL_UART_CLEAR_IT(handle_, UART_CLEAR_NEF);
//        __HAL_UART_CLEAR_IT(handle_, UART_CLEAR_OREF);
//        __HAL_UART_CLEAR_IT(handle_, UART_CLEAR_RTOF);
//        __HAL_UART_CLEAR_IDLEFLAG(handle_);
//        __HAL_UART_CLEAR_OREFLAG(handle_);
//        __HAL_UART_CLEAR_FEFLAG(handle_);

//        __HAL_UART_RESET_HANDLE_STATE(handle_);
//        auto error = HAL_UART_GetError(handle_);
//        HAL_UART_Init(handle_);
    }

    void StartReading(){
        rx_pack_.setPending();
        HAL_UARTEx_ReceiveToIdle_DMA(handle_, rx_pack_.data().data(), rx_pack_.size());
    }

    void RxHandlerSize(auto size){
        rx_pack_.setReady(size);
    }

    bool GetPack(auto& pack){
        if(!rx_pack_.isReady())
            return false;
        pack = rx_pack_;
        StartReading();
        return true;
    }

protected:
    void TaskPreProcedure() final{
        switch (current_task_.Type()){
            case connectivity::transmit_receive:
                HAL_UART_Transmit_DMA(handle_, current_task_.TxData(), current_task_.TxSize());
                HAL_UART_Receive_DMA(handle_, current_task_.RxData(), current_task_.RxSize());
                break;
            case connectivity::transmit:
                HAL_UART_Transmit_DMA(handle_, current_task_.TxData(), current_task_.TxSize());
                break;
            case connectivity::receive:
                HAL_UART_Receive_DMA(handle_, current_task_.RxData(), current_task_.RxSize());
                break;
        }
    }

    void TaskPostProcedure() final{

    }
private:
    RxStorage rx_pack_;
};

}//namespace connectivity::uart