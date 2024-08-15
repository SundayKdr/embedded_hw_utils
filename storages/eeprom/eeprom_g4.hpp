#pragma once

#include <array>
#include <bit>
#include <span>

#include "stm32g4xx_hal.h"
#include "eeprom_maps/g4_map.hpp"

struct EEPROM{
    struct Specs{
        using T = uint32_t;
        static constexpr T ow_data_size_                {320}; //OneWire::ParamTable::MAX_ROWS_COUNT * OneWire::ParamTable::ROW_SIZE
        static constexpr T board_data_start_addr_       {ow_data_size_};
        static constexpr T eeprom_board_data_size_      {sizeof(uint8_t) * 8}; //UID(4byte) ADDR(1byte) + HWver(1byte) + SWver(1byte) + 1byte
        static constexpr T eeprom_params_start_addr_    {board_data_start_addr_ + eeprom_board_data_size_};
        static constexpr T param_n_of_calib_fields_     {2};
        static constexpr T eeprom_calib_data_size_      {param_n_of_calib_fields_ * sizeof(float)};

        static constexpr T first_page_addr              {ADDR_FLASH_PAGE_0};
        static constexpr T start_addr_                  {ADDR_FLASH_PAGE_63};
        static constexpr T memory_size_bytes            {2048};
        static constexpr T base_offset_                 {eeprom_params_start_addr_ + eeprom_calib_data_size_};
    };

    uint32_t CalcOffset(uint32_t size){
        static uint32_t offset {Specs::base_offset_};
        auto ret_val = offset;
        offset += size;
        return ret_val;
    }

protected:
    using buffer_data_t = uint64_t;
    static constexpr std::size_t bytes_in_buffer_elem = sizeof(buffer_data_t);

    template<typename T, std::size_t Size>
    bool WriteBlock(uint32_t offset, std::span<T, Size> data)
    {
        if (Size > EEPROM::Specs::memory_size_bytes)
            return false;

        //find page by offset
        uint32_t pageNum = (EEPROM::Specs::start_addr_ + offset - EEPROM::Specs::first_page_addr) / EEPROM::Specs::memory_size_bytes;
        uint32_t offsetOnPage = offset % EEPROM::Specs::memory_size_bytes;

        //read page to buffer
        auto page_addr = pageNum * EEPROM::Specs::memory_size_bytes + EEPROM::Specs::first_page_addr;
        auto page_ptr = std::bit_cast<uint64_t*>(page_addr);
        for(auto& cell : page_buffer_)
            cell = *(page_ptr++);

        //modify buffer
        auto* buf_ptr = std::bit_cast<uint8_t*>(page_buffer_.data());
        for (std::size_t i = 0; i < Size; i++)
            buf_ptr[offsetOnPage + i] = data[i];

        //erase page
        HAL_FLASH_Unlock();
        __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

        erase_init_struct_.Page = pageNum;
        uint32_t PAGEError;
        if(HAL_FLASHEx_Erase(&erase_init_struct_, &PAGEError) != HAL_OK)
            return false;

        //write page
        uint32_t flashWriteCounter = 0;
        for (std::size_t i = 0; i < page_buffer_.size(); i++)
            if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, page_addr + i * 8, page_buffer_[i]) == HAL_OK)
                flashWriteCounter++;

        HAL_FLASH_Lock();
        if(flashWriteCounter == (EEPROM::Specs::memory_size_bytes / sizeof(buffer_data_t)))
            return true;

        return false;
    }

    template<typename T, std::size_t Size>
    bool ReadBlock(uint32_t offset, std::span<T, Size> storage){
        if(Size > EEPROM::Specs::memory_size_bytes)
            return false;
        auto* eeprom_data_ptr = std::bit_cast<uint8_t*>(EEPROM::Specs::start_addr_ + offset);
        for(auto& item: storage)
            item = *(eeprom_data_ptr++);
        return true;
    }

    template<typename T>
    void ReadIn(T& var, uint32_t offset){
        constexpr auto var_size = sizeof(T);
        std::array<uint8_t, var_size> buffer_;
        ReadBlock(offset, std::span{buffer_});
        std::memcpy(&var, buffer_.data(), buffer_.size());
    }

    template<typename T>
    bool WriteIn(T& var, uint32_t offset){
        constexpr auto var_size = sizeof(T);
        std::array<uint8_t, var_size> buffer_;
        std::memcpy(buffer_.data(), &var, buffer_.size());
        return WriteBlock(offset, std::span{buffer_});
    }

private:
    FLASH_EraseInitTypeDef erase_init_struct_{
        .TypeErase = FLASH_TYPEERASE_PAGES,
        .Banks = FLASH_BANK_1,
        .Page = 0,
        .NbPages = 1
    };
    std::array<buffer_data_t, (EEPROM::Specs::memory_size_bytes / bytes_in_buffer_elem)> page_buffer_{};
};