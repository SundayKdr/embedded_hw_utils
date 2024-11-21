#pragma once

#include <array>
#include <bit>

namespace analog::adc{

struct AdcResult
{
    static constexpr uint8_t max_channels{16};

    AdcResult() = default;
    explicit AdcResult(uint8_t channels)
        :channels_(channels)
    {}

    void StoreResults()
    {
        for(std::size_t i = 0; i < channels_; i++)
        {
            if(count_[i] == UINT32_MAX) [[unlikely]]
            {
                count_[i] = 0;
                sum_[i] = 0;
            }
            sum_[i] += results_storage_[i];
            count_[i]++;
        }
    }

    uint32_t* GetStoragePtr(){
        return results_storage_.data();
    }

    std::optional<float> GetValue(uint8_t sequencer_pos)
    {
        std::optional<float> value_opt;
        if(count_[sequencer_pos]){
            value_opt = static_cast<float>(sum_[sequencer_pos]) / static_cast<float>(count_[sequencer_pos]);
            sum_[sequencer_pos] = 0; count_[sequencer_pos] = 0;
        }
        return value_opt;
    }
    void SetChannelCnt(uint8_t cnt){
        channels_ = cnt;
    }
private:
    uint8_t channels_{0};
	std::array<uint32_t, max_channels> results_storage_{0,};
    std::array<uint32_t, max_channels> sum_{0,};
    std::array<uint32_t, max_channels> count_{0,};
};

}//namespace analog::adc