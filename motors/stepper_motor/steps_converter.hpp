#pragma once

#define $mStep_                     $DriverMicroStep
#define $timPcs_                    $MotorTimPsc

#define $mSTEPS(v)                  int((v) * $mStep_)
#define $uSec_(htim_psc)            (SystemCoreClock / (htim_psc))
#define $rampT_(val)                uint32_t( (val)/$mStep_ * $uSec_($timPcs_) )