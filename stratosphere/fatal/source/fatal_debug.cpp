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
#include <unordered_map>
#include "fatal_debug.hpp"
#include "fatal_config.hpp"

namespace sts::fatal::srv {

    namespace {

        constexpr u32 SvcSendSyncRequestInstruction = 0xD4000421;

        struct StackFrame {
            u64 fp;
            u64 lr;
        };

        bool IsThreadFatalCaller(u32 error_code, u32 debug_handle, u64 thread_id, u64 thread_tls_addr, ThreadContext *thread_ctx) {
            /* Verify that the thread is running or waiting. */
            {
                u64 _;
                u32 _thread_state;
                if (R_FAILED(svcGetDebugThreadParam(&_, &_thread_state, debug_handle, thread_id, DebugThreadParam_State))) {
                    return false;
                }

                const svc::ThreadState thread_state = static_cast<svc::ThreadState>(_thread_state);
                if (thread_state != svc::ThreadState::Waiting && thread_state != svc::ThreadState::Running) {
                    return false;
                }
            }

            /* Get the thread context. */
            if (R_FAILED(svcGetDebugThreadContext(thread_ctx, debug_handle, thread_id, svc::ThreadContextFlag_All))) {
                return false;
            }

            /* Try to read the current instruction. */
            u32 insn;
            if (R_FAILED(svcReadDebugProcessMemory(&insn, debug_handle, thread_ctx->pc.x, sizeof(insn)))) {
                return false;
            }

            /* If the instruction isn't svcSendSyncRequest, it's not the fatal caller. */
            if (insn != SvcSendSyncRequestInstruction) {
                return false;
            }

            /* Read in the fatal caller's TLS. */
            u8 thread_tls[0x100];
            if (R_FAILED(svcReadDebugProcessMemory(thread_tls, debug_handle, thread_tls_addr, sizeof(thread_tls)))) {
                return false;
            }

            /* HACK: We want to parse the command the fatal caller sent. */
            /* The easiest way to do this is to copy their TLS over ours, and parse ours. */
            std::memcpy(armGetTls(), thread_tls, sizeof(thread_tls));
            {
                IpcParsedCommand r;
                if (R_FAILED(ipcParse(&r))) {
                    return false;
                }

                /* Fatal command takes in a PID, only one buffer max. */
                if (!r.HasPid || r.NumStatics || r.NumStaticsOut || r.NumHandles) {
                    return false;
                }

                struct {
                    u32 magic;
                    u32 version;
                    u64 cmd_id;
                    u32 err_code;
                } *raw = (decltype(raw))(r.Raw);

                if (raw->magic != SFCI_MAGIC) {
                    return false;
                }

                if (raw->cmd_id > 2) {
                    return false;
                }

                if (raw->cmd_id != 2 && r.NumBuffers) {
                    return false;
                }

                if (raw->err_code != error_code) {
                    return false;
                }
            }

            /* We found our caller. */
            return true;
        }

        bool TryGuessBaseAddress(u64 *out_base_address, u32 debug_handle, u64 guess) {
            MemoryInfo mi;
            u32 pi;
            if (R_FAILED(svcQueryDebugProcessMemory(&mi, &pi, debug_handle, guess)) || mi.perm != Perm_Rx) {
                return false;
            }

            /* Iterate backwards until we find the memory before the code region. */
            while (mi.addr > 0) {
                if (R_FAILED(svcQueryDebugProcessMemory(&mi, &pi, debug_handle, guess))) {
                    return false;
                }

                if (mi.type == MemType_Unmapped) {
                    /* Code region will be at the end of the unmapped region preceding it. */
                    *out_base_address = mi.addr + mi.size;
                    return true;
                }

                guess = mi.addr - 4;
            }

            return false;
        }

        u64 GetBaseAddress(const ThrowContext *throw_ctx, const ThreadContext *thread_ctx, u32 debug_handle) {
            u64 base_address = 0;

            if (TryGuessBaseAddress(&base_address, debug_handle, thread_ctx->pc.x)) {
                return base_address;
            }

            if (TryGuessBaseAddress(&base_address, debug_handle, thread_ctx->lr)) {
                return base_address;
            }

            for (size_t i = 0; i < throw_ctx->cpu_ctx.aarch64_ctx.stack_trace_size; i++) {
                if (TryGuessBaseAddress(&base_address, debug_handle, throw_ctx->cpu_ctx.aarch64_ctx.stack_trace[i])) {
                    return base_address;
                }
            }

            return base_address;
        }

    }

    void TryCollectDebugInformation(ThrowContext *ctx, u64 process_id) {
        /* Try to debug the process. This may fail, if we called into ourself. */
        AutoHandle debug_handle;
        if (R_FAILED(svcDebugActiveProcess(debug_handle.GetPointer(), process_id))) {
            return;
        }

        /* First things first, check if process is 64 bits, and get list of thread infos. */
        std::unordered_map<u64, u64> thread_id_to_tls;
        {
            bool got_attach_process = false;
            svc::DebugEventInfo d;
            while (R_SUCCEEDED(svcGetDebugEvent(reinterpret_cast<u8 *>(&d), debug_handle.Get()))) {
                switch (d.type) {
                    case svc::DebugEventType::AttachProcess:
                        ctx->cpu_ctx.architecture = (d.info.attach_process.flags & 1) ? CpuContext::Architecture_Aarch64 : CpuContext::Architecture_Aarch32;
                        std::memcpy(ctx->proc_name, d.info.attach_process.name, sizeof(d.info.attach_process.name));
                        got_attach_process = true;
                        break;
                    case svc::DebugEventType::AttachThread:
                        thread_id_to_tls[d.info.attach_thread.thread_id] = d.info.attach_thread.tls_address;
                        break;
                    case svc::DebugEventType::Exception:
                    case svc::DebugEventType::ExitProcess:
                    case svc::DebugEventType::ExitThread:
                        break;
                }
            }

            if (!got_attach_process) {
                return;
            }
        }

        /* TODO: Try to collect information on 32-bit fatals. This shouldn't really matter for any real use case. */
        if (ctx->cpu_ctx.architecture == CpuContext::Architecture_Aarch32) {
            return;
        }

        /* Welcome to hell. Here, we try to identify which thread called into fatal. */
        bool found_fatal_caller = false;
        u64 thread_id = 0;
        ThreadContext thread_ctx;
        {
            /* We start by trying to get a list of threads. */
            u32 thread_count;
            u64 thread_ids[0x60];
            if (R_FAILED(svcGetThreadList(&thread_count, thread_ids, 0x60, debug_handle.Get()))) {
                return;
            }

            /* We need to locate the thread that's called fatal. */
            for (u32 i = 0; i < thread_count; i++) {
                const u64 cur_thread_id = thread_ids[i];
                if (thread_id_to_tls.find(cur_thread_id) == thread_id_to_tls.end()) {
                    continue;
                }

                if (IsThreadFatalCaller(ctx->error_code, debug_handle.Get(), cur_thread_id, thread_id_to_tls[cur_thread_id], &thread_ctx)) {
                    thread_id = cur_thread_id;
                    found_fatal_caller = true;
                    break;
                }
            }
            if (!found_fatal_caller) {
                return;
            }
        }
        if (R_FAILED(svcGetDebugThreadContext(&thread_ctx, debug_handle.Get(), thread_id, svc::ThreadContextFlag_All))) {
            return;
        }

        /* Set register states. */
        ctx->cpu_ctx.aarch64_ctx.SetRegisterValue(aarch64::RegisterName_FP, thread_ctx.fp);
        ctx->cpu_ctx.aarch64_ctx.SetRegisterValue(aarch64::RegisterName_LR, thread_ctx.lr);
        ctx->cpu_ctx.aarch64_ctx.SetRegisterValue(aarch64::RegisterName_SP, thread_ctx.sp);
        ctx->cpu_ctx.aarch64_ctx.SetRegisterValue(aarch64::RegisterName_PC, thread_ctx.pc.x);

        /* Parse a stack trace. */
        u64 cur_fp = thread_ctx.fp;
        ctx->cpu_ctx.aarch64_ctx.stack_trace_size = 0;
        for (unsigned int i = 0; i < aarch64::CpuContext::MaxStackTraceDepth; i++) {
            /* Validate the current frame. */
            if (cur_fp == 0 || (cur_fp & 0xF)) {
                break;
            }

            /* Read a new frame. */
            StackFrame cur_frame = {};
            if (R_FAILED(svcReadDebugProcessMemory(&cur_frame, debug_handle.Get(), cur_fp, sizeof(StackFrame)))) {
                break;
            }

            /* Advance to the next frame. */
            ctx->cpu_ctx.aarch64_ctx.stack_trace[ctx->cpu_ctx.aarch64_ctx.stack_trace_size++] = cur_frame.lr;
            cur_fp = cur_frame.fp;
        }

        /* Try to read up to 0x100 of stack. */
        for (size_t sz = 0x100; sz > 0; sz -= 0x10) {
            if (R_SUCCEEDED(svcReadDebugProcessMemory(ctx->stack_dump, debug_handle.Get(), thread_ctx.sp, sz))) {
                ctx->stack_dump_size = sz;
                break;
            }
        }

        /* Parse the base address. */
        ctx->cpu_ctx.aarch64_ctx.SetBaseAddress(GetBaseAddress(ctx, &thread_ctx, debug_handle.Get()));
    }

}
