#pragma once

#include "stm32g4xx_hal_spi.h"
#include "cstdint"

namespace connectivity::spi{
    constexpr std::size_t buffer_size = 4;
    constexpr std::size_t interface_cnt = 1;
    constexpr std::size_t queue_size = 10;
    using HandleT = SPI_HandleTypeDef*;
}// namespace connectivity::spi