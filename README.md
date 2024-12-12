## Lib for easy use and re-use all kinds of peripheral on stm32 in your cpp aps
### 1. UART sockets <br />
Set-up uart specs for lib:
```cpp
    namespace connectivity::uart{
        constexpr std::size_t interface_cnt {2};
        constexpr std::size_t tasks_queue_size {24};
        constexpr std::size_t tx_storage_size {64};
        constexpr std::size_t rx_storage_size {64};
        using HandleT = UART_HandleTypeDef*;
    }
```
#### Register all uart handles in init code of your app <br />
```cpp
    connectivity::uart::PlacePort(&huart1);
    connectivity::uart::PlacePort(&huart2);
    ...
```
Init socket in ctor and implement public function ProcessPacket(auto data_view) which manages a span  <br />
for message (wo dlc and crc only payload)
```cpp
    struct MyClass{
        MyClass(connectivity::uart::HandleT handle)
        : uart_socket_(handle, this)
        {}
        
        void ProcessPacket(auto data_view){}

        private:
            connectivity::uart::Socket<MyClass> uart_socket_;
    }
```