#pragma once

#include "stepper_motor_base.hpp"

#include <cmath>

namespace MotorSpecial {

    using namespace StepperMotor;

    enum AccelType{
        kLinear = 0,
        kConstantPower = 1,
        kParabolic = 2,
        kSigmoid = 3
    };

    struct AccelCfg
    {
        uint32_t ramp_time{};
        AccelType accel_type{};
        uint32_t A {};
        uint32_t Vmax {};
        uint32_t Vmin {};
        StepperCfg stepperCfg;
    };

    struct Sigmoid{
        float k1{1};
        float k2{1};
        const uint8_t k3{2};       //move sigmoid center (k3=2) from y_axis (0<k3<2) to y_axis (2<k3<8)

        float y_offset{1};
        int x_offset{1};

        float core_f(int x) const {
            return x / (k2 + std::abs(x));
        }
        void KCalc(uint32_t tTotal, float Vmax, float Vmin) {
            x_offset = tTotal / k3;
            k2 = x_offset * (1 / (1 - (Vmin / (Vmax / 2))) - 1);
            k1 = 1 / (core_f(tTotal - x_offset) + 1);
            k2 = x_offset * (1 / (1 - (Vmin / (Vmax * k1))) - 1);
            k1 = 1 / (core_f(tTotal - x_offset) + 1);
            y_offset = k1 * Vmax;
        }
        [[gnu::always_inline]] uint32_t VCalc(uint32_t uSec) {
            return y_offset * core_f(uSec - x_offset) + y_offset;
        }
    };

    struct AccelMotor: StepperMotorBase {
        explicit AccelMotor(AccelCfg& cfg)
            :StepperMotorBase(cfg.stepperCfg)
        {
            UpdateConfig(cfg);
        }

        void UpdateConfig(const AccelCfg& cfg){
            accel_type_ = cfg.accel_type;
            A_ = cfg.A;
            if (cfg.ramp_time != T_ || cfg.Vmax != config_Vmax_) {
                config_Vmax_ = cfg.Vmax;
                config_Vmin_ = cfg.Vmin;
                T_ = cfg.ramp_time;
                ReCalcKFactors();
            }
        }

        uint32_t GetAccelTimeGap(){
            return T_ - TimeOfAccelPhase();
        }

    protected:
        uint32_t T_{1};
        uint32_t config_Vmax_{1};
        uint32_t config_Vmin_{1};
    private:
        uint32_t A_{1};
        float k_{1};
        AccelType accel_type_ = kParabolic;
        Sigmoid sigmoid_;

        void ReCalcKFactors() {
            switch (accel_type_) {
                case kLinear:
                    k_ = static_cast<float>(config_Vmax_ - config_Vmin_) / T_;
                    break;
                case kConstantPower:
                    k_ = (config_Vmax_ - config_Vmin_) / sqrtf(T_);
                    break;
                case kParabolic:
                    break;
                case kSigmoid:
                    sigmoid_.KCalc(T_, static_cast<float>(config_Vmax_), static_cast<float>(config_Vmin_));
                    break;
            }
        }

        [[gnu::always_inline]] void ParabolicAcceleration() {
            switch (CurrentMoveMode()) {
                case Mode::ACCEL:
                    V_ += A_;
                    break;

                case Mode::DECCEL:
                    V_ -= A_;
                    break;
                default:
                    break;
            }
        }

        void AccelerationImpl() final{
            switch (accel_type_) {
                case kLinear:
                    V_ = static_cast<uint32_t>(k_ * TimeOfAccelPhase()) + CurrentMinSpeed();
                    break;
                case kConstantPower:
                    V_ = static_cast<uint32_t>(k_ * sqrtf(TimeOfAccelPhase())) + CurrentMinSpeed();
                    break;
                case kSigmoid:
                    V_ = sigmoid_.VCalc(TimeOfAccelPhase());
                    break;
                case kParabolic:
                    ParabolicAcceleration();
                    break;
            }
        }
    };
}