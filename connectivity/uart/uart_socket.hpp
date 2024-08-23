#pragma once

#include <ranges>

#include "embedded_hw_utils/utils/rx_packet.hpp"
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
            std::size_t rx_packet_size,
            std::size_t tx_storage_size_,
            std::size_t rx_update_rate_hz,
            bool use_crc_on_tx
        >
struct Socket{
    using RxPacket = utils::RxPacket<rx_packet_size>;
    using TxStorage = utils::TxStorage<tx_storage_size_, use_crc_on_tx>;

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
    Port::RxStorage rx_storage_;
    RxPacket assembled_packet_;

    void HandleMsg(){
        auto size = rx_storage_.getRxSize();
        if(size == RxPacket::pack_size)
            assembled_packet_.Reset();

        assembled_packet_.PlaceData(rx_storage_.dataView(size));

        if(assembled_packet_.isReady()){
            user_->ProcessPacket(assembled_packet_.GetPayloadView());
            assembled_packet_.Reset();
        }
    }

    void SendToUART(){
        PlaceTask(uart_handle_, tx_storage_.MakeTxData());
        tx_storage_.Reset();
    }

    void Start(){
        Port(uart_handle_)->StartReading();
        $RunAsync({
            if(Port(self->uart_handle_)->GetPack(self->rx_storage_))
                self->HandleMsg();
        }, rx_update_rate_hz);
    }
};

}//namespace connectivity::uart