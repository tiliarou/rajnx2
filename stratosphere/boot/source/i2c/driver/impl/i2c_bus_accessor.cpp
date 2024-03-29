/*
 * Copyright (c) 2018-2019 Atmosphère-NX
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <switch.h>
#include <stratosphere.hpp>

#include "i2c_pcv.hpp"
#include "i2c_bus_accessor.hpp"

namespace sts::i2c::driver::impl {

    void BusAccessor::Open(Bus bus, SpeedMode speed_mode) {
        std::scoped_lock<HosMutex> lk(this->open_mutex);
        /* Open new session. */
        this->open_sessions++;

        /* Ensure we're good if this isn't our first session. */
        if (this->open_sessions > 1) {
            if (this->speed_mode != speed_mode) {
                std::abort();
            }
            return;
        }

        /* Set all members for chosen bus. */
        {
            std::scoped_lock<HosMutex> lk(this->register_mutex);
            /* Set bus/registers. */
            this->SetBus(bus);
            /* Set pcv module. */
            this->pcv_module = ConvertToPcvModule(bus);
            /* Set speed mode. */
            this->speed_mode = speed_mode;
            /* Setup interrupt event. */
            this->CreateInterruptEvent(bus);
        }
    }

    void BusAccessor::Close() {
        std::scoped_lock<HosMutex> lk(this->open_mutex);
        /* Close current session. */
        this->open_sessions--;
        if (this->open_sessions > 0) {
            return;
        }

        /* Close interrupt event. */
        eventClose(&this->interrupt_event);

        /* Close PCV. */
        pcv::Finalize();

        this->suspended = false;
    }

    void BusAccessor::Suspend() {
        std::scoped_lock<HosMutex> lk(this->open_mutex);
        std::scoped_lock<HosMutex> lk_reg(this->register_mutex);

        if (!this->suspended) {
            this->suspended = true;

            if (this->pcv_module != PcvModule_I2C5) {
                this->DisableClock();
            }
        }
    }

    void BusAccessor::Resume() {
        if (this->suspended) {
            this->DoInitialConfig();
            this->suspended = false;
        }
    }

    void BusAccessor::DoInitialConfig() {
        std::scoped_lock<HosMutex> lk(this->register_mutex);

        if (this->pcv_module != PcvModule_I2C5) {
            pcv::Initialize();
        }

        this->ResetController();
        this->SetClock(this->speed_mode);
        this->SetPacketMode();
        this->FlushFifos();
    }

    size_t BusAccessor::GetOpenSessions() const {
        return this->open_sessions;
    }

    bool BusAccessor::GetBusy() const {
        /* Nintendo has a loop here that calls a member function to check if busy, retrying a few times. */
        /* This member function does "return false". */
        /* We will not bother with the loop. */
        return false;
    }

    void BusAccessor::OnStartTransaction() const {
        /* Nothing actually happens here. */
    }

    void BusAccessor::OnStopTransaction() const {
        /* Nothing actually happens here. */
    }

    Result BusAccessor::StartTransaction(Command command, AddressingMode addressing_mode, u32 slave_address) {
        /* Nothing actually happens here... */
        return ResultSuccess;
    }

    Result BusAccessor::Send(const u8 *data, size_t num_bytes, I2cTransactionOption option, AddressingMode addressing_mode, u32 slave_address) {
        std::scoped_lock<HosMutex> lk(this->register_mutex);
        const u8 *cur_src = data;
        size_t remaining = num_bytes;

        /* Set interrupt enable, clear interrupt status. */
        reg::Write(&this->i2c_registers->I2C_INTERRUPT_MASK_REGISTER_0,   0x8E);
        reg::Write(&this->i2c_registers->I2C_INTERRUPT_STATUS_REGISTER_0, 0xFC);

        ON_SCOPE_EXIT { this->ClearInterruptMask(); };

        /* Send header. */
        this->WriteTransferHeader(TransferMode::Send, option, addressing_mode, slave_address, num_bytes);

        /* Send bytes. */
        while (true) {
            const u32 fifo_status = reg::Read(&this->i2c_registers->I2C_FIFO_STATUS_0);
            const size_t fifo_cnt = (fifo_status >> 4);

            for (size_t fifo_idx = 0; remaining > 0 && fifo_idx < fifo_cnt; fifo_idx++) {
                const size_t cur_bytes = std::min(remaining, sizeof(u32));
                u32 val = 0;
                for (size_t i = 0; i < cur_bytes; i++) {
                    val |= cur_src[i] << (8 * i);
                }
                reg::Write(&this->i2c_registers->I2C_I2C_TX_PACKET_FIFO_0, val);

                cur_src += cur_bytes;
                remaining -= cur_bytes;
            }

            if (remaining == 0) {
                break;
            }

            eventClear(&this->interrupt_event);
            if (R_FAILED(eventWait(&this->interrupt_event, InterruptTimeout))) {
                this->HandleTransactionResult(ResultI2cBusBusy);
                eventClear(&this->interrupt_event);
                return ResultI2cTimedOut;
            }

            R_TRY(this->GetAndHandleTransactionResult());
        }

        reg::Write(&this->i2c_registers->I2C_INTERRUPT_MASK_REGISTER_0, 0x8C);

        /* Wait for successful completion. */
        while (true) {
            R_TRY(this->GetAndHandleTransactionResult());

            /* Check PACKET_XFER_COMPLETE */
            const u32 interrupt_status = reg::Read(&this->i2c_registers->I2C_INTERRUPT_STATUS_REGISTER_0);
            if (interrupt_status & 0x80) {
                R_TRY(this->GetAndHandleTransactionResult());
                break;
            }

            eventClear(&this->interrupt_event);
            if (R_FAILED(eventWait(&this->interrupt_event, InterruptTimeout))) {
                this->HandleTransactionResult(ResultI2cBusBusy);
                eventClear(&this->interrupt_event);
                return ResultI2cTimedOut;
            }
        }

        return ResultSuccess;
    }

    Result BusAccessor::Receive(u8 *out_data, size_t num_bytes, I2cTransactionOption option, AddressingMode addressing_mode, u32 slave_address) {
        std::scoped_lock<HosMutex> lk(this->register_mutex);
        u8 *cur_dst = out_data;
        size_t remaining = num_bytes;

        /* Set interrupt enable, clear interrupt status. */
        reg::Write(&this->i2c_registers->I2C_INTERRUPT_MASK_REGISTER_0,   0x8D);
        reg::Write(&this->i2c_registers->I2C_INTERRUPT_STATUS_REGISTER_0, 0xFC);

        /* Send header. */
        this->WriteTransferHeader(TransferMode::Receive, option, addressing_mode, slave_address, num_bytes);

        /* Receive bytes. */
        while (remaining > 0) {
            eventClear(&this->interrupt_event);
            if (R_FAILED(eventWait(&this->interrupt_event, InterruptTimeout))) {
                this->HandleTransactionResult(ResultI2cBusBusy);
                this->ClearInterruptMask();
                eventClear(&this->interrupt_event);
                return ResultI2cTimedOut;
            }

            R_TRY(this->GetAndHandleTransactionResult());

            const u32 fifo_status = reg::Read(&this->i2c_registers->I2C_FIFO_STATUS_0);
            const size_t fifo_cnt = std::min((remaining + 3) >> 2, static_cast<size_t>(fifo_status & 0xF));

            for (size_t fifo_idx = 0; remaining > 0 && fifo_idx < fifo_cnt; fifo_idx++) {
                const u32 val = reg::Read(&this->i2c_registers->I2C_I2C_RX_FIFO_0);
                const size_t cur_bytes = std::min(remaining, sizeof(u32));
                for (size_t i = 0; i < cur_bytes; i++) {
                    cur_dst[i] = static_cast<u8>((val >> (8 * i)) & 0xFF);
                }

                cur_dst += cur_bytes;
                remaining -= cur_bytes;
            }
        }

        /* N doesn't do ClearInterruptMask. */
        return ResultSuccess;
    }

    void BusAccessor::SetBus(Bus bus) {
        this->bus = bus;
        this->i2c_registers = GetRegisters(bus);
        this->clkrst_registers.SetBus(bus);
    }

    void BusAccessor::CreateInterruptEvent(Bus bus) {
        static constexpr u64 s_interrupts[] = {
            0x46, 0x74, 0x7C, 0x98, 0x55, 0x5F
        };
        if (ConvertToIndex(bus) >= util::size(s_interrupts)) {
            std::abort();
        }

        Handle evt_h;
        if (R_FAILED(svcCreateInterruptEvent(&evt_h, s_interrupts[ConvertToIndex(bus)], 1))) {
            std::abort();
        }

        eventLoadRemote(&this->interrupt_event, evt_h, false);
    }

    void BusAccessor::SetClock(SpeedMode speed_mode) {
        u32 t_high, t_low;
        u32 clk_div, src_div;
        u32 debounce;

        switch (speed_mode) {
            case SpeedMode::Normal:
                t_high = 2;
                t_low = 4;
                clk_div = 0x19;
                src_div = 0x13;
                debounce = 2;
                break;
            case SpeedMode::Fast:
                t_high = 2;
                t_low = 4;
                clk_div = 0x19;
                src_div = 0x04;
                debounce = 2;
                break;
            case SpeedMode::FastPlus:
                t_high = 2;
                t_low = 4;
                clk_div = 0x10;
                src_div = 0x02;
                debounce = 0;
                break;
            case SpeedMode::HighSpeed:
                t_high = 3;
                t_low = 8;
                clk_div = 0x02;
                src_div = 0x02;
                debounce = 0;
                break;
            default:
                std::abort();
        }

        if (speed_mode == SpeedMode::HighSpeed) {
            reg::Write(&this->i2c_registers->I2C_I2C_HS_INTERFACE_TIMING_0_0, (t_high << 8) | (t_low));
            reg::Write(&this->i2c_registers->I2C_I2C_CLK_DIVISOR_REGISTER_0, clk_div);
        } else {
            reg::Write(&this->i2c_registers->I2C_I2C_INTERFACE_TIMING_0_0, (t_high << 8) | (t_low));
            reg::Write(&this->i2c_registers->I2C_I2C_CLK_DIVISOR_REGISTER_0, (clk_div << 16));
        }

        reg::Write(&this->i2c_registers->I2C_I2C_CNFG_0, debounce);
        reg::Read(&this->i2c_registers->I2C_I2C_CNFG_0);

        if (this->pcv_module != PcvModule_I2C5) {
            if (R_FAILED(pcv::SetReset(this->pcv_module, true))) {
                std::abort();
            }
            if (R_FAILED(pcv::SetClockRate(this->pcv_module, (408'000'000) / (src_div + 1)))) {
                std::abort();
            }
            if (R_FAILED(pcv::SetReset(this->pcv_module, false))) {
                std::abort();
            }
        }
    }

    void BusAccessor::ResetController() const {
        if (this->pcv_module != PcvModule_I2C5) {
            if (R_FAILED(pcv::SetReset(this->pcv_module, true))) {
                std::abort();
            }
            if (R_FAILED(pcv::SetClockRate(this->pcv_module, 81'600'000))) {
                std::abort();
            }
            if (R_FAILED(pcv::SetReset(this->pcv_module, false))) {
                std::abort();
            }
        }
    }

    void BusAccessor::ClearBus() const {
        bool success = false;
        for (size_t i = 0; i < 3 && !success; i++) {
            success = true;

            this->ResetController();

            reg::Write(&this->i2c_registers->I2C_I2C_BUS_CLEAR_CONFIG_0, 0x90000);
            reg::SetBits(&this->i2c_registers->I2C_I2C_BUS_CLEAR_CONFIG_0, 0x4);
            reg::SetBits(&this->i2c_registers->I2C_I2C_BUS_CLEAR_CONFIG_0, 0x2);

            reg::SetBits(&this->i2c_registers->I2C_I2C_CONFIG_LOAD_0, 0x1);
            {
                u64 start_tick = armGetSystemTick();
                while (reg::Read(&this->i2c_registers->I2C_I2C_CONFIG_LOAD_0) & 1) {
                    if (armTicksToNs(armGetSystemTick() - start_tick) > 1'000'000) {
                        success = false;
                        break;
                    }
                }
            }
            if (!success) {
                continue;
            }

            reg::SetBits(&this->i2c_registers->I2C_I2C_BUS_CLEAR_CONFIG_0, 0x1);
            {
                u64 start_tick = armGetSystemTick();
                while (reg::Read(&this->i2c_registers->I2C_I2C_BUS_CLEAR_CONFIG_0) & 1) {
                    if (armTicksToNs(armGetSystemTick() - start_tick) > 1'000'000) {
                        success = false;
                        break;
                    }
                }
            }
            if (!success) {
                continue;
            }

            {
                u64 start_tick = armGetSystemTick();
                while (reg::Read(&this->i2c_registers->I2C_I2C_BUS_CLEAR_STATUS_0) & 1) {
                    if (armTicksToNs(armGetSystemTick() - start_tick) > 1'000'000) {
                        success = false;
                        break;
                    }
                }
            }
            if (!success) {
                continue;
            }
        }
    }

    void BusAccessor::DisableClock() {
        if (R_FAILED(pcv::SetClockEnabled(this->pcv_module, false))) {
            std::abort();
        }
    }

    void BusAccessor::SetPacketMode() {
        /* Set PACKET_MODE_EN, MSTR_CONFIG_LOAD */
        reg::SetBits(&this->i2c_registers->I2C_I2C_CNFG_0, 0x400);
        reg::SetBits(&this->i2c_registers->I2C_I2C_CONFIG_LOAD_0, 0x1);

        /* Set TX_FIFO_TRIGGER, RX_FIFO_TRIGGER */
        reg::Write(&this->i2c_registers->I2C_FIFO_CONTROL_0, 0xFC);
    }

    Result BusAccessor::FlushFifos() {
        reg::Write(&this->i2c_registers->I2C_FIFO_CONTROL_0, 0xFF);

        /* Wait for flush to finish, check every ms for 5 ms. */
        for (size_t i = 0; i < 5; i++) {
            if (!(reg::Read(&this->i2c_registers->I2C_FIFO_CONTROL_0) & 3)) {
                return ResultSuccess;
            }
            svcSleepThread(1'000'000ul);
        }

        return ResultI2cBusBusy;
    }

    Result BusAccessor::GetTransactionResult() const {
        const u32 packet_status = reg::Read(&this->i2c_registers->I2C_PACKET_TRANSFER_STATUS_0);
        const u32 interrupt_status = reg::Read(&this->i2c_registers->I2C_INTERRUPT_STATUS_REGISTER_0);

        /* Check for no ack. */
        if ((packet_status & 0xC) || (interrupt_status & 0x8)) {
            return ResultI2cNoAck;
        }

        /* Check for arb lost. */
        if ((packet_status & 0x2) || (interrupt_status & 0x4)) {
            this->ClearBus();
            return ResultI2cBusBusy;
        }

        return ResultSuccess;
    }

    void BusAccessor::HandleTransactionResult(Result result) {
        if (R_FAILED(result)) {
            if (result == ResultI2cNoAck || result == ResultI2cBusBusy) {
                this->ResetController();
                this->SetClock(this->speed_mode);
                this->SetPacketMode();
                this->FlushFifos();
            } else {
                std::abort();
            }
        }
    }

    Result BusAccessor::GetAndHandleTransactionResult() {
        const Result transaction_res = this->GetTransactionResult();
        R_TRY_CLEANUP(transaction_res, {
            this->HandleTransactionResult(transaction_res);
            this->ClearInterruptMask();
            eventClear(&this->interrupt_event);
        });
        return ResultSuccess;
    }

    void BusAccessor::WriteTransferHeader(TransferMode transfer_mode, I2cTransactionOption option, AddressingMode addressing_mode, u32 slave_address, size_t num_bytes) {
        this->FlushFifos();

        reg::Write(&this->i2c_registers->I2C_I2C_TX_PACKET_FIFO_0, 0x10);
        reg::Write(&this->i2c_registers->I2C_I2C_TX_PACKET_FIFO_0, static_cast<u32>(num_bytes - 1) & 0xFFF);

        const u32 slave_addr_val = ((transfer_mode == TransferMode::Receive) & 1) | ((slave_address & 0x7F) << 1);
        u32 hdr_val = 0;
        hdr_val |= ((this->speed_mode == SpeedMode::HighSpeed) & 1) << 22;
        hdr_val |= ((transfer_mode == TransferMode::Receive) & 1) << 19;
        hdr_val |= ((addressing_mode != AddressingMode::SevenBit) & 1) << 18;
        hdr_val |= (1 << 17);
        hdr_val |= (((option & I2cTransactionOption_Stop) == 0) & 1) << 16;
        hdr_val |= slave_addr_val;

        reg::Write(&this->i2c_registers->I2C_I2C_TX_PACKET_FIFO_0, hdr_val);
    }

}
