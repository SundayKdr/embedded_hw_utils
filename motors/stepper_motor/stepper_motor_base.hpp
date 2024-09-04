#pragma once

#include <cstdlib>
#include <algorithm>

#include "embedded_hw_utils/IO/pin.hpp"
#include "stm32g4xx_hal_tim.h"

namespace Motor{
    enum class Direction{
        BACKWARDS = 0,
        FORWARD = 1
    };

    enum Mode
    {
        IDLE,
        ACCEL,
        CONST,
        DECCEL,
        in_ERROR
    };

    enum MOTOR_EVENT {
        EVENT_NULL = 0,
        EVENT_STOP,
        EVENT_CSS,  //	constant speed reached
        EVENT_CSE   //  constant speed end
    };
}

namespace StepperMotor{
    struct StepperCfg
    {
        pin_board::PIN<pin_board::Writeable> step_pin;
        pin_board::PIN<pin_board::Writeable> direction_pin;
        pin_board::PIN<pin_board::Writeable> enable_pin;
        TIM_HandleTypeDef* htim {};
        uint32_t timChannel {0x00000001U};
        uint32_t criticalNofSteps {};
        bool directionInverted {false};
    };

    using namespace Motor;
    class StepperMotorBase{
    public:
        using MOTOR_IOS = pin_board::PIN<pin_board::Writeable>;
        using Dir = Motor::Direction;

        StepperMotorBase() = delete;
        const StepperMotorBase& operator=(const StepperMotorBase &) = delete;
        StepperMotorBase& operator=(StepperMotorBase &) = delete;
        StepperMotorBase(StepperMotorBase&) = delete;
        StepperMotorBase(StepperMotorBase&&)= delete;

        void StopMotor(){
            HAL_TIM_PWM_Stop_IT(htim_, timChannel_);
//            enable_pin_.setValue(pin_board::HIGH);
            motorMoving_ = false;
            mode_ = Mode::IDLE;
            event_ = EVENT_STOP;
        }

        void StandByModeOn(){
            enable_pin_.setValue(pin_board::HIGH);
        }

        [[gnu::always_inline]] void MotorRefresh(){
            if(mode_ == IDLE || mode_ == Mode::in_ERROR)
                return;
            CalcSpeed();
            CalcRegValue();
            AppCorrection();
        }

    protected:
        virtual void AppCorrection() = 0;
        virtual void AccelerationImpl() = 0;

        void MakeMotorTask(uint32_t start_speed,
                           uint32_t max_speed,
                           Direction dir,
                           uint32_t steps)
        {
            if(motorMoving_)
                StopMotor();
            Vmin_ = start_speed;
            Vmax_ = max_speed;
            V_ = start_speed;
            SetDirection(dir);
            StartMotor(steps);
        }

        [[gnu::always_inline]] void ChangeDirectionAndGo(uint32_t steps){
            ChangeDirection();
            steps_to_go_ = steps;
            if(!motorMoving_)
                StartMotor(steps);
        }

        void SetStepsToGo(uint32_t steps) {
            steps_to_go_ = steps;
            mode_ = Mode::ACCEL;
        }

        void CorrectStepsToGo(int correction){
            if(correction < 0)
                assert(-correction < steps_to_go_);
            steps_to_go_ += correction;
            mode_ = Mode::ACCEL;
        }

        void CorrectCurrentStep(int correction){
            task_step_ += correction;
        }

        void SetMode(Mode mode){
            mode_ = mode;
        }

        explicit StepperMotorBase(StepperMotor::StepperCfg& cfg)
            :step_pin_(cfg.step_pin)
            ,direction_pin_(cfg.direction_pin)
            ,enable_pin_(cfg.enable_pin)
            ,htim_(cfg.htim)
            ,timChannel_(cfg.timChannel)
            ,kCriticalNofSteps_(cfg.criticalNofSteps)
            ,directionInverted_(cfg.directionInverted)
        {
            timer_tick_Hz_ = SystemCoreClock / (cfg.htim->Instance->PSC);
        };

        uint32_t V_{0};

        MOTOR_IOS step_pin_;
        MOTOR_IOS direction_pin_;
        MOTOR_IOS enable_pin_;

        void StartMotor(uint32_t steps){
            ResetMotorData();
            steps_to_go_ = steps;
            enable_pin_.setValue(pin_board::LOW);
            motorMoving_ = true;
            HAL_TIM_PWM_Start_IT(htim_, timChannel_);
        }

        void SetDirInversion(bool val){
            directionInverted_ = val;
        }

        [[gnu::always_inline]] void ChangeDirection(){
            ResetMotorData();
            SetDirection(currentDirection_ == Motor::Direction::FORWARD ? Direction::BACKWARDS : Direction::FORWARD);
        }

    private:
        int task_step_ {0};
        int steps_to_go_ {0};
        int accel_step_ {0};

        long long uSec_accel_ {0};

        uint32_t Vmin_{0};
        uint32_t Vmax_{0};

        TIM_HandleTypeDef* htim_;
        uint32_t timChannel_;
        uint32_t timer_tick_Hz_;

        uint32_t kCriticalNofSteps_;
        Direction currentDirection_ {Direction::FORWARD};
        Mode mode_ {Mode::IDLE};
        MOTOR_EVENT event_ {EVENT_STOP};

        bool directionInverted_ {false};
        bool motorMoving_ {false};

        void ResetMotorData(){
            V_ = Vmin_;
            CalcRegValue();
            task_step_ = 0;
            accel_step_ = 0;
            uSec_accel_ = 0;
            mode_ = Mode::ACCEL;
        }

        [[gnu::always_inline]] void CalcRegValue(){
            auto new_arr_v = timer_tick_Hz_ / V_;
            auto new_ccr_v = new_arr_v / 2;
            if(new_arr_v < UINT16_MAX){
                __HAL_TIM_SET_AUTORELOAD(htim_, new_arr_v);
                __HAL_TIM_SET_COMPARE(htim_, timChannel_, new_ccr_v);
            }else{
                __HAL_TIM_SET_AUTORELOAD(htim_, UINT16_MAX);
                __HAL_TIM_SET_COMPARE(htim_, timChannel_, UINT16_MAX / 2);
            }
        }

        void SetDirection(Direction newDirection){
            currentDirection_ = newDirection;
            if(directionInverted_)
                direction_pin_.setValue(currentDirection_ == Motor::Direction::FORWARD ?
                        pin_board::logic_level(Direction::BACKWARDS) : pin_board::logic_level(Direction::FORWARD));
            else
                direction_pin_.setValue(pin_board::logic_level(newDirection));
        }

        [[gnu::always_inline]] void CalcSpeed(){
            if(mode_ == Mode::IDLE)
                return;

            task_step_++;

            switch (mode_)
            {
                case Mode::ACCEL:
                {
                    AccelerationImpl();
                    if(V_ >= Vmax_){
                        V_ = Vmax_;
                        event_ = EVENT_CSS;
                        mode_ = Mode::CONST;
                    }
                    accel_step_++;
                    uSec_accel_ += timer_tick_Hz_ / V_;

                    if(accel_step_ >= steps_to_go_ / 2)
                        mode_ = Mode::DECCEL;
                }
                break;

                case Mode::CONST:
                {
                    if(task_step_ + accel_step_ >= steps_to_go_){
                        event_ = EVENT_CSE;
                        mode_ = Mode::DECCEL;
                    }
                }
                break;

                case Mode::DECCEL:
                {
                    if(V_ > Vmin_){
                        AccelerationImpl();
                        accel_step_--;
                        uSec_accel_ -= timer_tick_Hz_ / V_;
                    }
                    else if(V_ < Vmin_ || V_ == 0){
                        V_ = Vmin_;
                        mode_ = Mode::CONST;
                    }
                }
                break;

                default:
                    break;
            }
        }

public:
        [[nodiscard]] bool IsMotorMoving() const { return motorMoving_;}
        [[nodiscard]] MOTOR_EVENT GetEvent() const { return event_;}
        [[nodiscard]] auto StepsToGo() const { return steps_to_go_; }
        [[nodiscard]] auto CurrentStep() const { return task_step_; }
        [[nodiscard]] auto CurrentDirection() const { return currentDirection_; }
        [[nodiscard]] auto GetTimHandler() const { return  htim_;}
        [[nodiscard]] auto GetTotalRangeSteps() const {return kCriticalNofSteps_;}
        [[nodiscard]] auto TimeOfAccelPhase() const {return uSec_accel_;}
        [[nodiscard]] auto CurrentMoveMode() const { return mode_; };
        [[nodiscard]] auto CurrentMinSpeed() const { return Vmin_; };
        [[nodiscard]] auto CurrentMaxSpeed() const { return Vmax_; };
    };
} //namespace stepper_motor