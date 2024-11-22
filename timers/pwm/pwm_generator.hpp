#pragma once

#include <ranges>
#include "tim.h"

namespace timers::pwm{
    struct PWMGenerator{

        PWMGenerator() = delete;
        PWMGenerator(std::pair<TIM_HandleTypeDef*, uint32_t> specs)
            : handle_(specs.first)
            , tim_channel_(specs.second)
        {
            timer_tick_Hz_ = SystemCoreClock / (handle_->Instance->PSC);
        }

        void SetPWMLimit(float percentage){
            up_pwm_limit_ = static_cast<uint16_t>(static_cast<float>(UINT16_MAX) / 100 * percentage);
        }

        void Start(){
            timer_tick_Hz_ = SystemCoreClock / (handle_->Instance->PSC);
            current_tim_arr_ = __HAL_TIM_GET_AUTORELOAD(handle_);
            __HAL_TIM_SET_COMPARE(handle_, tim_channel_, 0);
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
            assert(percentage <= 100 && percentage >= 0);
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
        uint32_t timer_tick_Hz_{};
        float current_tim_arr_{};
    };
}//namespace timers::pwm
