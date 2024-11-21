#pragma once

#include <ranges>
#include "adc_results.hpp"
#include "adc.h"

namespace analog::adc{

struct AdcDma{
    AdcDma() = default;

    void Start(ADC_HandleTypeDef* handle, uint8_t channel_count){
        handle_ = handle;
        result_.SetChannelCnt(channel_count);
        HAL_ADCEx_Calibration_Start(handle);
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

    auto GetHandle(){
        return handle_;
    }

    auto GetResult(uint8_t sequencer_pos){
        return result_.GetValue(sequencer_pos);
    }
private:
    AdcResult result_{};
    ADC_HandleTypeDef* handle_{};
};

struct Dispatcher{
    static Dispatcher& global(){
        static auto instance = Dispatcher();
        return instance;
    }

    void ProcessADCCallBack(ADC_HandleTypeDef* handle){
        if(auto it = std::ranges::find(adc_storage_, handle, &AdcDma::GetHandle); it != adc_storage_.end())
            it->ProcessADCCallBack();
    }

    void PlaceADC(std::pair<ADC_HandleTypeDef*,uint8_t> adc_config_pair){
        assert(cnt_ < adc_cnt);
        adc_storage_[cnt_++].Start(adc_config_pair.first, adc_config_pair.second);
    }

    std::optional<float> GetValue(ADC_HandleTypeDef* handle, uint8_t sequencer_pos){
        if(auto it = std::ranges::find(adc_storage_, handle, &AdcDma::GetHandle); it != adc_storage_.end())
            return it->GetResult(sequencer_pos);
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