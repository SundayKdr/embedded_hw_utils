#pragma once

#include "embedded_hw_utils/connectivity/impl/interface_task.hpp"
#include "embedded_hw_utils/connectivity/uart/uart_driver_settings.hpp"
//#include "embedded_hw_utils/meta/meta_utils.hpp"

namespace connectivity::uart{

using InterfaceTask = InterfaceTask<tx_storage_size>;

struct Task final: InterfaceTask{

    template<typename ... Types>
    Task(Types&&... args)
        :InterfaceTask(std::forward<Types>(args)...)
    {}
//    template<typename ...Types>
//    Task(Types&&... args)
//    requires meta::utils::has_type<PinT, Types...>
//    {
//        meta::utils::remove_arg_by_type_and_invoke<PinT>(
//                [&]<typename... Args_t>(Args_t &&...args_){
//                    *this = TaskT(std::forward<Args_t>(args_)...);}
//                ,std::forward<Types>(args)...);
//        pin_ = meta::utils::magic_get<PinT>(std::forward<Types>(args)...);
//    }
};

}//namespace connectivity::uart