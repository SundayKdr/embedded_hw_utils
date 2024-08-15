#pragma once

namespace connectivity::uart{
    constexpr std::size_t interface_cnt {1};
    constexpr std::size_t tasks_queue_size {8};
    constexpr std::size_t tx_storage_size {64};
    constexpr std::size_t rx_storage_size {16};

    using HandleT = UART_HandleTypeDef*;
}