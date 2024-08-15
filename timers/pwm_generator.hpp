#pragma once

#include <ranges>
#include "tim.h"

namespace timers::pwm{
    struct PWMGenerator{
        PWMGenerator() = default;
        PWMGenerator(TIM_HandleTypeDef* handle, uint32_t channel)
                : handle_(handle)
                , channel_(channel)
        {}

        void Start(){
            HAL_TIM_PWM_Start(handle_, channel_);
        }

        void Stop(){
            HAL_TIM_PWM_Stop(handle_, channel_);
        }

        TIM_HandleTypeDef* GetHandle(){
            return handle_;
        }

        void SetPulseWidth(float percentage){

        }

    private:
        TIM_HandleTypeDef* handle_{};
        uint32_t channel_{};
    };
}//namespace timers::pwm
