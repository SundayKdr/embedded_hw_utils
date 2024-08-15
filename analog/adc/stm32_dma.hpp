#pragma once

#include <ranges>
#include "adc_results.hpp"
#include "adc.h"

namespace analog::adc{

struct AdcDma{
    AdcDma() = default;
    AdcDma(ADC_HandleTypeDef* handle, uint8_t channel_count)
        : handle_(handle)
        , result_(channel_count)
    {
        HAL_ADCEx_Calibration_Start(handle, ADC_SINGLE_ENDED);
        HAL_ADC_Start_DMA(handle, result_.GetStoragePtr(), channel_count);
    }

    void Stop(){
        HAL_ADC_Stop_DMA(handle_);
    }

    void ProcessADCCallBack(){
        result_.StoreResults();
    }

    void ProcessADCErrorCallBack(){
        result_.StoreResults();
    }

    ADC_HandleTypeDef* GetHandle(){
        return handle_;
    }

    auto GetResult(uint8_t channel){
        return result_.GetValue(channel);
    }
private:
    AdcResult result_{};
    ADC_HandleTypeDef* handle_{};
};

struct Dispatcher{
    static constexpr uint8_t adc_cnt{2};
    static Dispatcher& global(){
        static Dispatcher dispatcher{};
        return dispatcher;
    }

    void ProcessADCCallBack(ADC_HandleTypeDef* handle){
        if(auto it = std::ranges::find(adc_storage_, handle, &AdcDma::GetHandle); it != adc_storage_.end())
            it->ProcessADCCallBack();
    }

    void PlaceADC(std::pair<ADC_HandleTypeDef*,uint8_t> adc_config_pair){
        assert(cnt_ < adc_cnt);
        adc_storage_[cnt_++] = {adc_config_pair.first, adc_config_pair.second};
    }

    std::optional<float> GetValue(ADC_HandleTypeDef* handle, uint8_t channel){
        if(auto it = std::ranges::find(adc_storage_, handle, &AdcDma::GetHandle); it != adc_storage_.end())
            return it->GetResult(channel);
        return{};
    }
    void ProcessADCErrorCallBack(ADC_HandleTypeDef* handle){

    }
private:
    uint8_t cnt_{0};
    std::array<AdcDma, adc_cnt> adc_storage_;
};

}//namespace analog::adc

extern "C"{
    void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
    {
        analog::adc::Dispatcher::global().ProcessADCCallBack(hadc);
    }
    void HAL_ADC_ErrorCallback(ADC_HandleTypeDef* hadc)
    {
        analog::adc::Dispatcher::global().ProcessADCErrorCallBack(hadc);
    }
}