#pragma once

#include "embedded_hw_utils/connectivity/uart/impl/uart_port.hpp"
#include "embedded_hw_utils/connectivity/impl/interface_driver.hpp"

#define $UARTPlaceTask_cb(context_ptr, args...)    \
    DRIVER_place_task_cb(uart, args)

namespace connectivity::uart{

struct Driver final: InterfaceDriver<Port, interface_cnt>{
     static Driver& global(){
        static auto instance = Driver();
        return instance;
    }
    void RxHandlerSize(HandleT handle, uint16_t size){
        if(auto it = std::ranges::find(ports_, handle, &Port::GetHandle); it != ports_.end())
            it->RxHandlerSize(size);
    }
};

template <typename ... Types>
static void PlaceTask(HandleT handle, Types&&... args){
    Driver::global().GetPort(handle)->PlaceTask(std::forward<Types>(args)...);
}

void PlacePort(HandleT handle){
    Driver::global().PlacePort(handle);
}

auto Port(HandleT handle){
    return Driver::global().GetPort(handle);
}

}//namespace connectivity::uart

extern "C"
{
    void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size) {
        connectivity::uart::Driver::global().RxHandlerSize(huart, size);
    }
    void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart) {
        connectivity::uart::Driver::global().TxHandler(huart);
    }
    void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart) {
        connectivity::uart::Driver::global().RxHandler(huart);
    }
    void HAL_UART_ErrorCallback(UART_HandleTypeDef* huart) {
        connectivity::uart::Driver::global().ErrorHandler(huart);
    }
}