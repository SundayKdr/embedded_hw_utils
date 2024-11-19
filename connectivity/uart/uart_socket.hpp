#pragma once

#include <ranges>

#include "embedded_hw_utils/utils/socket_packet.hpp"
#include "embedded_hw_utils/utils/storages/tx_storage.hpp"
#include "embedded_hw_utils/connectivity/uart/uart_driver.hpp"

#include "async_tim_tasks/async_tim_tasks.hpp"

namespace connectivity::uart{

template<typename T>
concept DavaViewConcept = requires(T t)
{
    t.operator[](t.size()-1);
};

template<typename User,
            bool use_crc_on_tx = true,
            std::size_t rx_update_rate_hz = 300
        >
struct Socket{
    using SocketPacket = utils::SocketPacket<rx_storage_size>;
    using TxStorage = utils::TxStorage<tx_storage_size, use_crc_on_tx>;

    Socket(HandleT uart_h, User* user)
        : uart_handle_(uart_h)
        , user_(user)
    {
        Start();
    }

    template<typename ...Args>
    void PlaceAndSend(Args&&... args){
        tx_storage_.PlaceToStorage(std::forward<Args>(args)...);
        SendToUART();
    }

    template<typename ...Args>
    void Place(Args&&... args){
        tx_storage_.PlaceToStorage(std::forward<Args>(args)...);
    }

    void Send(){
        SendToUART();
    }
protected:
    User* user_;
    HandleT uart_handle_;
    TxStorage tx_storage_;
    Port::RxStorage rx_packet_;
    SocketPacket socket_packet_;

    void HandleMsg(){
        socket_packet_.Reset();
        socket_packet_.PlaceData(rx_packet_.dataView());
        if(socket_packet_.CheckCRC())
            user_->ProcessPacket(socket_packet_.GetPayloadView());
    }

    void SendToUART(){
        PlaceTask(uart_handle_, tx_storage_.MakeTxData());
        tx_storage_.Reset();
    }

    void Start(){
        Port(uart_handle_)->StartReading();
        $RunAsync({
            if(Port(self->uart_handle_)->GrabPacket(self->rx_packet_))
                self->HandleMsg();
        },rx_update_rate_hz);
    }
};

}//namespace connectivity::uart