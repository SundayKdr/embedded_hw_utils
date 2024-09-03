#pragma once

#include <bit>
#include <utility>
#include <optional>

#include "can_pack.hpp"
#include "embedded_hw_utils/utils/queue.hpp"

namespace connectivity::can{
struct Port
{
	enum { RXQ_SIZE = 16, TXQ_SIZE = 32};

	Port(){
		header_.Identifier = 0x000;
        header_.IdType = FDCAN_EXTENDED_ID;
        header_.TxFrameType = FDCAN_DATA_FRAME;
        header_.DataLength = FDCAN_DLC_BYTES_8;
        header_.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
        header_.BitRateSwitch = FDCAN_BRS_OFF;
        header_.FDFormat = FDCAN_CLASSIC_CAN;
        header_.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
        header_.MessageMarker = 0;
	}

    void SetHandler(FDCAN_HandleTypeDef* handler){
        handler_ = handler;
    }

	std::optional<Pack> Read()
	{
		if(rx_queue_.empty())
            return {};
        std::optional<Pack> val = rx_queue_.front();
        rx_queue_.pop();
        return val;
	}

	bool Trans(){
        if(tx_queue_.empty())
            return false;
		auto& msg = tx_queue_.front();

        header_.Identifier = msg.id;
        header_.DataLength = msg.dlc << 16;

		if(HAL_FDCAN_GetTxFifoFreeLevel(handler_) > 0){
			HAL_FDCAN_AddMessageToTxFifoQ(handler_, &header_, msg.data.data());
            tx_queue_.pop();
			return true;
		}
		return false;
	}

	bool OnRX(const FDCAN_RxHeaderTypeDef& header,  uint8_t* const data){
		return rx_queue_.push(Pack{header.Identifier, static_cast<uint8_t>(header.DataLength >> 16), data});
	}

	void Send(uint32_t id, uint8_t dlc,  uint8_t* const data){
        tx_queue_.push(Pack{id, dlc, data});
	}
    void Send(Pack& pack){
        tx_queue_.push(pack);
    }
    void Send(Pack&& pack){
        tx_queue_.push(std::forward<Pack>(pack));
    }
private:
	FDCAN_TxHeaderTypeDef header_;
    FDCAN_HandleTypeDef* handler_ {nullptr};
    utils::Queue<Pack, RXQ_SIZE> rx_queue_{};
    utils::Queue<Pack, TXQ_SIZE> tx_queue_{};
};
}