#pragma once

#include <ranges>

#include "tx_storage.hpp"
#include "embedded_hw_utils/connectivity/uart/uart_socket.hpp"

namespace connectivity::uart{

template<typename User>
struct UartMonitor: Socket<User, 8, 0, 100, false>{
    using Socket_t = Socket<User, 8, 0, 100, false>;

    UartMonitor(HandleT uart_h, User* monitor)
        :Socket_t(uart_h, monitor)
    {}

    template<typename ...Args>
    void PlaceAndSend(Args&&... args){
        monitor_txStorage_.PlaceToStorage(std::forward<Args>(args)...);
        SendToUART();
    }

    template<typename ...Args>
    void Place(Args&&... args){
        monitor_txStorage_.PlaceToStorage(std::forward<Args>(args)...);
    }
private:
    monitor::TxStorage<tx_storage_size, false> monitor_txStorage_;

    void SendToUART(){
        PlaceTermination();
        connectivity::uart::PlaceTask(Socket_t::uart_handle_,
                                      utils::TxData{monitor_txStorage_.dataPtr(),
                                                    monitor_txStorage_.cursor()});
        monitor_txStorage_.Reset();
    }

    void PlaceTermination(){
        static constexpr unsigned char term = 0xFF;
        monitor_txStorage_.StoreBytes(term, term, term);
    }
};

}//namespace connectivity::uart