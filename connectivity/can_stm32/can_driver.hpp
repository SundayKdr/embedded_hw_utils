#pragma once

#include "array"
#include "fdcan.h"
#include "impl/can_port.hpp"
#include "can_client.hpp"

#define $CANRegisterClient(expr)                                                                                      \
    connectivity::can::Driver().RegisterClient(connectivity::can::Client(this, [](void* context, CanPack &can_pack){  \
        auto self = static_cast<decltype(this)>(context);                                                             \
        expr;                                                                                                         \
    }))

namespace connectivity::can{

struct Driver{
    static Driver& global(){
        static Driver driver{};
        return driver;
    }

    void SetHandler(FDCAN_HandleTypeDef* handler){
        port_.SetHandler(handler);
    }

    auto RxData(){
        return rx_data_.data();
    }

    auto* Header(){
        return &header_;
    }

    void OnRx(){
        port_.OnRX(header_, rx_data_.data());
    }

    void SendMsg(Pack& pack){
        port_.Send(pack);
    }

    void SendMsg(Pack&& pack){
        port_.Send(std::forward<Pack>(pack));
    }

    void ErrorHandler(HAL_StatusTypeDef status){
        switch (status) {
            case HAL_ERROR:
            case HAL_BUSY:
            case HAL_TIMEOUT:
                break;
            case HAL_OK:
                break;
        }
    }

    void RegisterClient(Client&& client){
        if(auto it = std::ranges::find(clients_, false, &Client::HasContext); it != clients_.end())
            *it = client;
    }

private:
    Driver(){
        $RunAsync( {self->PollPort();});
    }

    void PollPort()
    {
        if(auto pack_opt = port_.Read(); pack_opt)
            SendAll(*pack_opt);
        port_.Trans();
    }

    void SendAll(Pack& pack){
        for(auto& client: clients_)
            client(pack);
    }

    std::array<Client, 10> clients_;
    FDCAN_RxHeaderTypeDef header_;
    std::array<uint8_t, 8> rx_data_;
    Port port_;
};

static void PlacePort(auto handle){
    Driver::global().SetHandler(handle);
}

static Driver& Driver(){
    return Driver::global();
}

static void SendMsg(Pack&& pack){
    Driver::global().SendMsg(std::forward<Pack>(pack));
}

}//namespace connectivity::can

extern "C"{
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
    {
        auto status = HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0,
                                             connectivity::can::Driver().Header(),
                                             connectivity::can::Driver().RxData());
        if(status)
            connectivity::can::Driver().ErrorHandler(status);
        connectivity::can::Driver().OnRx();
    }
}
}