#pragma once

#include <ranges>
#include "tim.h"

namespace timers::pwm{
    struct PWMGenerator{

        PWMGenerator() = default;
        PWMGenerator(TIM_HandleTypeDef* handle, uint32_t channel)
            : handle_(handle)
            , tim_channel_(channel)
        {
            timer_tick_Hz_ = SystemCoreClock / (handle->Instance->PSC);
            current_tim_arr_ = __HAL_TIM_GET_AUTORELOAD(handle);
        }

        void SetPWMLimit(float percentage){
            up_pwm_limit_ = static_cast<uint16_t>(static_cast<float>(UINT16_MAX) / 100 * percentage);
        }

        void Start(){
            HAL_TIM_PWM_Start(handle_, tim_channel_);
        }

        void Stop(){
            SetPulseWidth(0);
            HAL_TIM_PWM_Stop(handle_, tim_channel_);
        }

        TIM_HandleTypeDef* GetHandle(){
            return handle_;
        }

        [[gnu::always_inline]] void SetPulseWidth(float percentage){
            auto new_ccr_v = static_cast<uint16_t>(current_tim_arr_ / 100 * percentage);
            if(new_ccr_v < up_pwm_limit_)
                __HAL_TIM_SET_COMPARE(handle_, tim_channel_, new_ccr_v);
            else
                __HAL_TIM_SET_COMPARE(handle_, tim_channel_, up_pwm_limit_);
        }

    private:
        uint16_t up_pwm_limit_{UINT16_MAX};
        TIM_HandleTypeDef* handle_{};
        uint32_t tim_channel_{};
        uint32_t timer_tick_Hz_;
        float current_tim_arr_;
    };
}//namespace timers::pwm
