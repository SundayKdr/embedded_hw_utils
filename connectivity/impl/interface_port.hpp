#pragma once

#include <utility>
#include <optional>

#include "embedded_hw_utils/utils/queue.hpp"
#include "interface_task.hpp"

namespace connectivity{

template<typename InterfaceHandle_type, typename TaskT, std::size_t queue_size>
struct InterfacePort{
    using Task_t = TaskT;
    using Hadle_t = InterfaceHandle_type;

    void SetHandle(Hadle_t spiHandle){
        handle_ = spiHandle;
    }

    auto GetHandle(){
        return handle_;
    }

    void TxHandler(){
        if(current_task_.Type() == connectivity::transmit_receive)
            return;
        FinishTask();
    }

    void RxHandler(){
        FinishTask();
    }

    void ClearQueue(){
        tasks_.clear();
        current_task_.setState(TaskState::in_process);
    }

    void FinishTask(){
        if(current_task_.isInProcess())
            current_task_.setState(TaskState::pending);
    }

    void ProcessTask(){
        if(current_task_.isPending())
        {
            current_task_.CallBack();
            TaskPostProcedure();
            current_task_.setState(TaskState::free);
        }
        else if(!tasks_.empty() && current_task_.isFree()){
            current_task_ = tasks_.front();
            current_task_.setState(TaskState::in_process);
            tasks_.pop();
            TaskPreProcedure();
        }
    }

    template<typename ...Args>
    void PlaceTask(Args&& ...arg){
        tasks_.push(Task_t(std::forward<Args>(arg)...));
    }

    void PlaceTask(Task_t&& task){
        tasks_.push(std::forward<Task_t>(task));
    }

    auto operator()(){
        return handle_;
    }

protected:
    Task_t current_task_{};
    utils::Queue<Task_t, queue_size> tasks_{};
    Hadle_t handle_ {nullptr};

    virtual void TaskPreProcedure() = 0;
    virtual void TaskPostProcedure() = 0;
};

}//namespace connectivity