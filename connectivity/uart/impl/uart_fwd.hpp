#pragma once

#include "stm32g4xx_hal_uart.h"
#include "embedded_hw_utils/connectivity/impl/interface_task.hpp"
#include "embedded_hw_utils/connectivity/impl/interface_port.hpp"
#include "embedded_hw_utils/connectivity/impl/interface_driver.hpp"

namespace connectivity::uart{
    constexpr std::size_t uart_buffer_size = 8;
    constexpr std::size_t uart_interface_cnt = 1;
    constexpr std::size_t uart_queue_size = 8;
    using UartHandleT = UART_HandleTypeDef*;
}