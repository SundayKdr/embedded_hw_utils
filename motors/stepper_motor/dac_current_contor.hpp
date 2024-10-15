#pragma once

#include "stm32g4xx_hal_dac.h"

template<float reference_voltage, float level_high, float level_low, uint16_t reg_max_v = 4095>
struct CurrentControl{
    CurrentControl(DAC_HandleTypeDef *h, uint32_t c)
            :handle(h)
            ,channel(c)
    {
        Start();
    }
    [[nodiscard]] uint16_t CalcValue(float voltage_to_set) const{
        auto multiplier = voltage_to_set / reference_voltage;
        return multiplier <= 1 ? multiplier * reg_max_v : reg_max_v;
    }
    void Start() const {
        HAL_DAC_Start(handle, channel);
    }
    bool SetHigh() const{
        return HAL_DAC_SetValue(handle, channel, DAC_ALIGN_12B_R, CalcValue(level_high)) == HAL_OK;
    }
    bool SetLow() const{
        return HAL_DAC_SetValue(handle, channel, DAC_ALIGN_12B_R, CalcValue(level_low)) == HAL_OK;
    }
private:
    DAC_HandleTypeDef *handle;
    uint32_t channel;
};

