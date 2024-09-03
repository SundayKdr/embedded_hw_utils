#pragma once

#include "pin.hpp"

struct InputSignal{
    enum PinConnectionState{
        on,
        connected,
        disconnected,
        off
    };

    explicit InputSignal(pin_board::PIN<pin_board::Readable> pin, uint32_t debounce_time = 1) noexcept
        :pin_(pin)
        ,debounce_time_(debounce_time)
    {
        UpdatePin();
        GetPinConnectionState();
    }

    PinConnectionState GetPinConnectionState(){
        auto retV = off;
        if(signal_state_ && last_pin_connection_state_)
            retV = on;
        else if(signal_state_ && !last_pin_connection_state_)
            retV = connected;
        else if(!signal_state_ && last_pin_connection_state_)
            retV = disconnected;
        last_pin_connection_state_ = signal_state_;
        return retV;
    }

    constexpr void UpdatePin() {
        if(pin_.getState()){
            if(active_time_ < debounce_time_)
                active_time_++;
        }else
            active_time_ = 0;

        if(active_time_ >= debounce_time_)
            SetState(pin_board::HIGH);
        else
            SetState(pin_board::LOW);
    }

    constexpr void SetDebounceTime(uint32_t time){
        debounce_time_ = time;
    }

    constexpr void InvertSignalPin(){
        pin_.setInverted();
    }

    [[nodiscard]] constexpr pin_board::logic_level getState() const {
        return signal_state_;
    }

    bool getRawState(){
        if(pin_.IsInverted())
            return !pin_.getState();
        else
            return pin_.getState();
    }
private:
    pin_board::PIN<pin_board::Readable> pin_;
    pin_board::logic_level signal_state_ {pin_board::LOW};
    uint32_t debounce_time_;
    uint32_t active_time_{0};
    pin_board::logic_level last_pin_connection_state_ {pin_board::LOW};

    constexpr void SetState(pin_board::logic_level level){
        signal_state_ = level;
    }
};