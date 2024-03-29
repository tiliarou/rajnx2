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

#include <atmosphere/version.h>

#include "fatal_task_screen.hpp"
#include "fatal_config.hpp"
#include "fatal_font.hpp"

namespace sts::fatal::srv {

    /* Include Atmosphere logo into its own anonymous namespace. */

    namespace {

        #include "fatal_ams_logo.inc"

    }

    namespace {

        /* Screen definitions. */
        constexpr u32 FatalScreenWidth = 1280;
        constexpr u32 FatalScreenHeight = 720;
        constexpr u32 FatalScreenBpp = 2;
        constexpr u32 FatalLayerZ = 100;

        constexpr u32 FatalScreenWidthAlignedBytes = (FatalScreenWidth * FatalScreenBpp + 63) & ~63;
        constexpr u32 FatalScreenWidthAligned = FatalScreenWidthAlignedBytes / FatalScreenBpp;

        /* Pixel calculation helper. */
        constexpr u32 GetPixelOffset(u32 x, u32 y) {
            u32 tmp_pos = ((y & 127) / 16) + (x/32*8) + ((y/16/8)*(((FatalScreenWidthAligned/2)/16*8)));
            tmp_pos *= 16*16 * 4;

            tmp_pos += ((y%16)/8)*512 + ((x%32)/16)*256 + ((y%8)/2)*64 + ((x%16)/8)*32 + (y%2)*16 + (x%8)*2;//This line is a modified version of code from the Tegra X1 datasheet.

            return tmp_pos / 2;
        }

        /* Task definitions. */
        class ShowFatalTask : public ITask {
            private:
                ViDisplay display;
                ViLayer layer;
                NWindow win;
                Framebuffer fb;
            private:
                Result SetupDisplayInternal();
                Result SetupDisplayExternal();
                Result PrepareScreenForDrawing();
                Result ShowFatal();
            public:
                virtual Result Run() override;
                virtual const char *GetName() const override {
                    return "ShowFatal";
                }
                virtual size_t GetStackSize() const override {
                    return 0x8000;
                }
        };

        class BacklightControlTask : public ITask {
            private:
                void TurnOnBacklight();
            public:
                virtual Result Run() override;
                virtual const char *GetName() const override {
                    return "BacklightControlTask";
                }
        };

        /* Task globals. */
        ShowFatalTask g_show_fatal_task;
        BacklightControlTask g_backlight_control_task;

        /* Task implementations. */
        Result ShowFatalTask::SetupDisplayInternal() {
            ViDisplay temp_display;
            /* Try to open the display. */
            R_TRY_CATCH(viOpenDisplay("Internal", &temp_display)) {
                R_CATCH(ResultViNotFound) {
                    return ResultSuccess;
                }
            } R_END_TRY_CATCH;

            /* Guarantee we close the display. */
            ON_SCOPE_EXIT { viCloseDisplay(&temp_display); };

            /* Turn on the screen. */
            if (GetRuntimeFirmwareVersion() >= FirmwareVersion_300) {
                R_TRY(viSetDisplayPowerState(&temp_display, ViPowerState_On));
            } else {
                /* Prior to 3.0.0, the ViPowerState enum was different (0 = Off, 1 = On). */
                R_TRY(viSetDisplayPowerState(&temp_display, ViPowerState_On_Deprecated));
            }

            /* Set alpha to 1.0f. */
            R_TRY(viSetDisplayAlpha(&temp_display, 1.0f));

            return ResultSuccess;
        }

        Result ShowFatalTask::SetupDisplayExternal() {
            ViDisplay temp_display;
            /* Try to open the display. */
            R_TRY_CATCH(viOpenDisplay("External", &temp_display)) {
                R_CATCH(ResultViNotFound) {
                    return ResultSuccess;
                }
            } R_END_TRY_CATCH;

            /* Guarantee we close the display. */
            ON_SCOPE_EXIT { viCloseDisplay(&temp_display); };

            /* Set alpha to 1.0f. */
            R_TRY(viSetDisplayAlpha(&temp_display, 1.0f));

            return ResultSuccess;
        }

        Result ShowFatalTask::PrepareScreenForDrawing() {
            /* Connect to vi. */
            R_TRY(viInitialize(ViServiceType_Manager));

            /* Close other content. */
            viSetContentVisibility(false);

            /* Setup the two displays. */
            R_TRY(SetupDisplayInternal());
            R_TRY(SetupDisplayExternal());

            /* Open the default display. */
            R_TRY(viOpenDefaultDisplay(&this->display));

            /* Reset the display magnification to its default value. */
            u32 display_width, display_height;
            R_TRY(viGetDisplayLogicalResolution(&this->display, &display_width, &display_height));

            /* viSetDisplayMagnification was added in 3.0.0. */
            if (GetRuntimeFirmwareVersion() >= FirmwareVersion_300) {
                R_TRY(viSetDisplayMagnification(&this->display, 0, 0, display_width, display_height));
            }

            /* Create layer to draw to. */
            R_TRY(viCreateLayer(&this->display, &this->layer));

            /* Setup the layer. */
            {
                /* Display a layer of 1280 x 720 at 1.5x magnification */
                /* NOTE: N uses 2 (770x400) RGBA4444 buffers (tiled buffer + linear). */
                /* We use a single 1280x720 tiled RGB565 buffer. */
                constexpr u32 raw_width = FatalScreenWidth;
                constexpr u32 raw_height = FatalScreenHeight;
                constexpr u32 layer_width = ((raw_width) * 3) / 2;
                constexpr u32 layer_height = ((raw_height) * 3) / 2;

                const float layer_x = static_cast<float>((display_width - layer_width) / 2);
                const float layer_y = static_cast<float>((display_height - layer_height) / 2);

                R_TRY(viSetLayerSize(&this->layer, layer_width, layer_height));

                /* Set the layer's Z at display maximum, to be above everything else .*/
                R_TRY(viSetLayerZ(&this->layer, FatalLayerZ));

                /* Center the layer in the screen. */
                R_TRY(viSetLayerPosition(&this->layer, layer_x, layer_y));

                /* Create framebuffer. */
                R_TRY(nwindowCreateFromLayer(&this->win, &this->layer));
                R_TRY(framebufferCreate(&this->fb, &this->win, raw_width, raw_height, PIXEL_FORMAT_RGB_565, 1));
            }

            return ResultSuccess;
        }

        Result ShowFatalTask::ShowFatal() {
            const FatalConfig &config = GetFatalConfig();

            /* Prepare screen for drawing. */
            DoWithSmSession([&]() {
                R_ASSERT(PrepareScreenForDrawing());
            });

            /* Dequeue a buffer. */
            u16 *tiled_buf = reinterpret_cast<u16 *>(framebufferBegin(&this->fb, NULL));
            if (tiled_buf == nullptr) {
                return ResultFatalNullGraphicsBuffer;
            }

            /* Let the font manager know about our framebuffer. */
            font::ConfigureFontFramebuffer(tiled_buf, GetPixelOffset);
            font::SetFontColor(0xFFFF);

            /* Draw a background. */
            for (size_t i = 0; i < this->fb.fb_size / sizeof(*tiled_buf); i++) {
                tiled_buf[i] = 0x39C9;
            }

            /* Draw the atmosphere logo in the bottom right corner. */
            for (size_t y = 0; y < AtmosphereLogoHeight; y++) {
                for (size_t x = 0; x < AtmosphereLogoWidth; x++) {
                    tiled_buf[GetPixelOffset(FatalScreenWidth - AtmosphereLogoWidth - 32 + x, 32 + y)] = AtmosphereLogoData[y * AtmosphereLogoWidth + x];
                }
            }

            /* TODO: Actually draw meaningful shit here. */
            font::SetPosition(32, 64);
            font::SetFontSize(16.0f);
            font::PrintFormat(config.GetErrorMessage(), R_MODULE(this->context->error_code), R_DESCRIPTION(this->context->error_code), this->context->error_code);
            font::AddSpacingLines(0.5f);
            font::PrintFormatLine("Title: %016lX", static_cast<u64>(this->context->title_id));
            font::AddSpacingLines(0.5f);
            font::PrintFormatLine(u8"Firmware: %s (Atmosphère %u.%u.%u-%s)", config.GetFirmwareVersion().display_version, CURRENT_ATMOSPHERE_VERSION, GetAtmosphereGitRevision());
            font::AddSpacingLines(1.5f);
            if (this->context->error_code != ResultAtmosphereVersionMismatch) {
                font::Print(config.GetErrorDescription());
            } else {
                /* Print a special message for atmosphere version mismatch. */
                font::Print(u8"Atmosphère version mismatch detected.\n\n"
                                   u8"Please press the POWER Button to restart the console normally, or a VOL button\n"
                                   u8"to reboot to a payload (or RCM, if none is present). If you are unable to\n"
                                   u8"restart the console, hold the POWER Button for 12 seconds to turn the console off.\n\n"
                                   u8"Please ensure that all Atmosphère components are updated.\n"
                                   u8"github.com/Atmosphere-NX/Atmosphere/releases\n");
            }

            /* Add a line. */
            for (size_t x = 32; x < FatalScreenWidth - 32; x++) {
                tiled_buf[GetPixelOffset(x, font::GetY())] = 0xFFFF;
            }

            font::AddSpacingLines(1.5f);

            u32 backtrace_y = font::GetY();
            u32 backtrace_x = 0;

            /* Note architecutre. */
            const bool is_aarch32 = this->context->cpu_ctx.architecture == CpuContext::Architecture_Aarch32;

            /* Print GPRs. */
            font::SetFontSize(14.0f);
            font::Print("General Purpose Registers      ");
            {
                font::SetPosition(font::GetX() + 2, font::GetY());
                u32 x = font::GetX();
                font::Print("PC: ");
                font::SetPosition(x + 47, font::GetY());
            }
            if (is_aarch32) {
                font::PrintMonospaceU32(this->context->cpu_ctx.aarch32_ctx.pc);
            } else {
                font::PrintMonospaceU64(this->context->cpu_ctx.aarch64_ctx.pc);
            }
            font::PrintLine("");
            font::SetPosition(32, font::GetY());
            font::AddSpacingLines(0.5f);
            if (is_aarch32) {
                for (size_t i = 0; i < (aarch32::RegisterName_GeneralPurposeCount / 2); i++) {
                    u32 x = font::GetX();
                    font::PrintFormat("%s:", aarch32::CpuContext::RegisterNameStrings[i]);
                    font::SetPosition(x + 47, font::GetY());
                    if (this->context->cpu_ctx.aarch32_ctx.HasRegisterValue(static_cast<aarch32::RegisterName>(i))) {
                        font::PrintMonospaceU32(this->context->cpu_ctx.aarch32_ctx.r[i]);
                    } else {
                        font::PrintMonospaceBlank(8);
                    }
                    font::Print("  ");
                    x = font::GetX();
                    font::PrintFormat("%s:", aarch32::CpuContext::RegisterNameStrings[i + (aarch32::RegisterName_GeneralPurposeCount / 2)]);
                    font::SetPosition(x + 47, font::GetY());
                    if (this->context->cpu_ctx.aarch32_ctx.HasRegisterValue(static_cast<aarch32::RegisterName>(i + (aarch32::RegisterName_GeneralPurposeCount / 2)))) {
                        font::PrintMonospaceU32(this->context->cpu_ctx.aarch32_ctx.r[i + (aarch32::RegisterName_GeneralPurposeCount / 2)]);
                    } else {
                        font::PrintMonospaceBlank(8);
                    }

                    if (i == (aarch32::RegisterName_GeneralPurposeCount / 2) - 1) {
                        font::Print("    ");
                        backtrace_x = font::GetX();
                    }

                    font::PrintLine("");
                    font::SetPosition(32, font::GetY());
                }
            } else {
                for (size_t i = 0; i < aarch64::RegisterName_GeneralPurposeCount / 2; i++) {
                    u32 x = font::GetX();
                    font::PrintFormat("%s:", aarch64::CpuContext::RegisterNameStrings[i]);
                    font::SetPosition(x + 47, font::GetY());
                    if (this->context->cpu_ctx.aarch64_ctx.HasRegisterValue(static_cast<aarch64::RegisterName>(i))) {
                        font::PrintMonospaceU64(this->context->cpu_ctx.aarch64_ctx.x[i]);
                    } else {
                        font::PrintMonospaceBlank(16);
                    }
                    font::Print("  ");
                    x = font::GetX();
                    font::PrintFormat("%s:", aarch64::CpuContext::RegisterNameStrings[i + (aarch64::RegisterName_GeneralPurposeCount / 2)]);
                    font::SetPosition(x + 47, font::GetY());
                    if (this->context->cpu_ctx.aarch64_ctx.HasRegisterValue(static_cast<aarch64::RegisterName>(i + (aarch64::RegisterName_GeneralPurposeCount / 2)))) {
                        font::PrintMonospaceU64(this->context->cpu_ctx.aarch64_ctx.x[i + (aarch64::RegisterName_GeneralPurposeCount / 2)]);
                    } else {
                        font::PrintMonospaceBlank(16);
                    }

                    if (i == (aarch64::RegisterName_GeneralPurposeCount / 2) - 1) {
                        font::Print("    ");
                        backtrace_x = font::GetX();
                    }

                    font::PrintLine("");
                    font::SetPosition(32, font::GetY());
                }
            }

            /* Print Backtrace. */
            u32 bt_size;
            if (is_aarch32) {
                bt_size = this->context->cpu_ctx.aarch32_ctx.stack_trace_size;
            } else {
                bt_size = this->context->cpu_ctx.aarch64_ctx.stack_trace_size;
            }


            font::SetPosition(backtrace_x, backtrace_y);
            if (bt_size == 0) {
                if (is_aarch32) {
                    font::Print("Start Address: ");
                    font::PrintMonospaceU32(this->context->cpu_ctx.aarch32_ctx.base_address);
                    font::PrintLine("");
                } else {
                    font::Print("Start Address: ");
                    font::PrintMonospaceU64(this->context->cpu_ctx.aarch64_ctx.base_address);
                    font::PrintLine("");
                }
            } else {
                if (is_aarch32) {
                    font::Print("Backtrace - Start Address: ");
                    font::PrintMonospaceU32(this->context->cpu_ctx.aarch32_ctx.base_address);
                    font::PrintLine("");
                    font::AddSpacingLines(0.5f);
                    for (u32 i = 0; i < aarch32::CpuContext::MaxStackTraceDepth / 2; i++) {
                        u32 bt_cur = 0, bt_next = 0;
                        if (i < this->context->cpu_ctx.aarch32_ctx.stack_trace_size) {
                            bt_cur = this->context->cpu_ctx.aarch32_ctx.stack_trace[i];
                        }
                        if (i + aarch32::CpuContext::MaxStackTraceDepth / 2 < this->context->cpu_ctx.aarch32_ctx.stack_trace_size) {
                            bt_next = this->context->cpu_ctx.aarch32_ctx.stack_trace[i + aarch32::CpuContext::MaxStackTraceDepth / 2];
                        }

                        if (i < this->context->cpu_ctx.aarch32_ctx.stack_trace_size) {
                            u32 x = font::GetX();
                            font::PrintFormat("BT[%02d]: ", i);
                            font::SetPosition(x + 72, font::GetY());
                            font::PrintMonospaceU32(bt_cur);
                            font::Print("  ");
                        }

                        if (i + aarch32::CpuContext::MaxStackTraceDepth / 2 < this->context->cpu_ctx.aarch32_ctx.stack_trace_size) {
                            u32 x = font::GetX();
                            font::PrintFormat("BT[%02d]: ", i + aarch32::CpuContext::MaxStackTraceDepth / 2);
                            font::SetPosition(x + 72, font::GetY());
                            font::PrintMonospaceU32(bt_next);
                        }

                        font::PrintLine("");
                        font::SetPosition(backtrace_x, font::GetY());
                    }
                } else {
                    font::Print("Backtrace - Start Address: ");
                    font::PrintMonospaceU64(this->context->cpu_ctx.aarch64_ctx.base_address);
                    font::PrintLine("");
                    font::AddSpacingLines(0.5f);
                    for (u32 i = 0; i < aarch64::CpuContext::MaxStackTraceDepth / 2; i++) {
                        u64 bt_cur = 0, bt_next = 0;
                        if (i < this->context->cpu_ctx.aarch64_ctx.stack_trace_size) {
                            bt_cur = this->context->cpu_ctx.aarch64_ctx.stack_trace[i];
                        }
                        if (i + aarch64::CpuContext::MaxStackTraceDepth / 2 < this->context->cpu_ctx.aarch64_ctx.stack_trace_size) {
                            bt_next = this->context->cpu_ctx.aarch64_ctx.stack_trace[i + aarch64::CpuContext::MaxStackTraceDepth / 2];
                        }

                        if (i < this->context->cpu_ctx.aarch64_ctx.stack_trace_size) {
                            u32 x = font::GetX();
                            font::PrintFormat("BT[%02d]: ", i);
                            font::SetPosition(x + 72, font::GetY());
                            font::PrintMonospaceU64(bt_cur);
                            font::Print("  ");
                        }

                        if (i + aarch64::CpuContext::MaxStackTraceDepth / 2 < this->context->cpu_ctx.aarch64_ctx.stack_trace_size) {
                            u32 x = font::GetX();
                            font::PrintFormat("BT[%02d]: ", i + aarch64::CpuContext::MaxStackTraceDepth / 2);
                            font::SetPosition(x + 72, font::GetY());
                            font::PrintMonospaceU64(bt_next);
                        }

                        font::PrintLine("");
                        font::SetPosition(backtrace_x, font::GetY());
                    }
                }
            }


            /* Enqueue the buffer. */
            framebufferEnd(&fb);

            return ResultSuccess;
        }

        Result ShowFatalTask::Run() {
            /* Don't show the fatal error screen until we've verified the battery is okay. */
            eventWait(const_cast<Event *>(&this->context->battery_event), U64_MAX);

            return ShowFatal();
        }

        void BacklightControlTask::TurnOnBacklight() {
            lblSwitchBacklightOn(0);
        }

        Result BacklightControlTask::Run() {
            TurnOnBacklight();
            return ResultSuccess;
        }

    }

    ITask *GetShowFatalTask(const ThrowContext *ctx) {
        g_show_fatal_task.Initialize(ctx);
        return &g_show_fatal_task;
    }

    ITask *GetBacklightControlTask(const ThrowContext *ctx) {
        g_backlight_control_task.Initialize(ctx);
        return &g_backlight_control_task;
    }

}
