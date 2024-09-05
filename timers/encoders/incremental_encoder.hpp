#pragma once

#include <array>

constexpr std::size_t center_value = 0xffff / 2;

namespace encoder{
    enum class Direction{
        BACKWARDS = 0,
        FORWARD = 1
    };

    struct Event{
        Direction dir{Direction::FORWARD};
        float mm{0};
        bool has_update{false};
    };
}

struct Encoder{
    explicit Encoder(TIM_HandleTypeDef* tim, float conv_coef = 1, bool inverted = false)
        :tim_(tim)
        ,inverted_(inverted)
        ,conv_coef_(conv_coef)
    {
        Start();
    }

    void Start(){
        HAL_TIM_Encoder_Start(tim_, TIM_CHANNEL_ALL);
        ResetBuffer();
    }

    encoder::Event GetChanges(){
        if(!enabled_)
            return {encoder::Direction::BACKWARDS, 0};
        auto ticks = GetTicksAndReset();
        auto dir = GetDirection();
        bool has_update = ticks != 0;
        return encoder::Event{dir, ConvertTicksToMm(ticks), has_update};
    }

    [[nodiscard]] encoder::Direction GetDirection() const{
        bool is_counting_down = __HAL_TIM_IS_TIM_COUNTING_DOWN(tim_);
        is_counting_down = inverted_ == !is_counting_down;
        return static_cast<encoder::Direction>(is_counting_down);
    }

    [[nodiscard]] float ConvertTicksToMm(uint32_t ticks) const{
        return ticks * conv_coef_;
    }

    uint16_t GetTicksAndReset(){
        bool counting_down = __HAL_TIM_IS_TIM_COUNTING_DOWN(tim_);
        uint32_t count = __HAL_TIM_GET_COUNTER(tim_);
        auto ticks = counting_down ? center_value - count : count - center_value;
        ResetBuffer();
        return ticks;
    }

    void ResetBuffer(){
        __HAL_TIM_SET_COUNTER(tim_, center_value);
    }

    void SetEnabled(){
        enabled_ = true;
    }

    void SetDisabled(){
        enabled_ = false;
    }

private:
    TIM_HandleTypeDef* tim_;
    bool enabled_ {true};
    bool inverted_{false};
    float conv_coef_{1};
};
