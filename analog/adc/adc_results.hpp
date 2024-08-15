#pragma once

#include <array>

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

    std::optional<float> GetValue(uint8_t channel)
    {
        std::optional<float> value_opt;
        if(count_[channel]){
            value_opt = static_cast<float>(sum_[channel])/static_cast<float>(count_[channel]);
            sum_[channel] = 0; count_[channel] = 0;
        }
        return value_opt;
    }

private:
    uint8_t channels_{0};
	std::array<uint32_t, max_channels> results_storage_{};
    std::array<uint32_t, max_channels> sum_{};
    std::array<uint32_t, max_channels> count_{};
};

}//namespace analog::adc