# Changelog
## 0.9.4
+ Support was added for 9.0.0.
  + **Please note**: 9.0.0 made a number of changes that may cause some issues with homebrew. Details:
  + 9.0.0 changed HID in a way that causes libnx to be unable to detect button input.
    + Homebrew should be recompiled with newest libnx to fix this.
    + Atmosphere now provides a temporary hid-mitm that will cause homebrew to continue to work as expected.
      + This mitm will be removed in a future Atmosphere revision once homebrew has been updated, to allow users to use a custom hid mitm again if they desire.
  + 9.0.0 introduced an dependency in FS on the USB system module in order to launch the SD card.
    + This means the USB system module must now be launched before the SD card is initialized.
    + Correspondingly, the USB system module can no longer be IPS patched, and its settings cannot be reliably mitm'd.
    + We know this is frustrating, so we'll be looking into whether there is some way of addressing this in the future.
+ An off-by-one error was fixed in `boot` system module's pinmux initialization.
  + This could theoretically have caused issues with HdmiCec communication.
  + No users reported issues, so it's unclear if this was a problem in practice.
+ A bug was fixed that could cause webapplet launching homebrew to improperly set the accessible url whitelist.
+ BIS key generation has been fixed for newer hardware.
  + Newer hardware uses new, per-firmware device key to generate BIS keys instead of the first device key, so previously the wrong keys were generated as backup.
  + This only affects units manufactured after ~5.0.0.
+ General system stability improvements to enhance the user's experience.
## 0.9.3
+ Thanks to hexkyz, fusee's boot sequence has been greatly optimized.
  + Memory training is now managed by a separate binary (`fusee-mtc`, loaded by fusee-primary before fusee-secondary).
  + Unnecessarily long splash screen display times were reduced.
  + The end result is that Atmosphere now boots *significantly* faster. :)
  + **Note:** This means fusee-primary must be updated for Atmosphere to boot successfully.
+ The version string was adjusted, and now informs users whether or not they are using emummc.
+ Atmosphere now automatically backs up the user's BIS keys on boot.
  + This should prevent a user from corrupting nand without access to a copy of the keys needed to fix it.
    + This is especially relevant on ipatched units, where the RCM vulnerability is not an option for addressing bricks.
+ The `pm` system module was rewritten as part of Stratosphere's ongoing refactor.
  + Support was added for forward-declaring a mitm'd service before a custom user sysmodule is launched.
    + This should help resolve dependency issues with service registration times.
  + SM is now informed of every process's title id, including built-in system modules.
+ The `creport` system module was rewritten as part of Stratosphere's ongoing refactor.
  + creport now dumps up to 0x100 of stack from each thread in the target process.
  + A few bugs were fixed, including one that caused creport to incorrectly dump process dying messages.
+ Defaults were added to `system_settings.ini` for controlling hbloader's memory usage in applet mode.
  + These defaults reserve enough memory so that homebrew can launch swkbd while in applet mode.
+ The `fatal` system module was rewritten as part of Stratosphere's ongoing refactor.
  + Incorrect display output ("2000-0000") has been fixed. Fatal will now correctly show 2162-0002 when this occurs.
  + A longstanding bug in how fatal manages the displays has been fixed, and official display init behavior is now matched precisely.
+ General system stability improvements to enhance the user's experience.
## 0.9.2
+ A number of emummc bugfixes were added (all thanks to @m4xw's hard work). The following is a summary of emummc changes:
  + Support for file-based emummc instances was fixed.
    + Please note: file-based emummc is still unoptimized, and so may be much slower than partition-based.
    + This speed differential should hopefully be made better in a future emummc update.
  + The way emummc handles power management was completely overhauled.
    + Emummc now properly handles init/de-init, and now supports low voltage mode.
    + Much better support for shutdown was added, which should assuage corruption/synchronization problems.
    + This should also improve support for more types of SD cards.
  + A bug was fixed that caused emummc to not work on lower system versions due to missing SVC access.
  + **Please note**: The configuration entries used for emummc have been changed.
    + `emummc_` prefixes have been removed, since they are superfluous given the `emummc` category they are under.
    + As an example, `emummc!emummc_enabled` is now `emummc!enabled`.
    + INI configurations made by @CTCaer's [tool](https://github.com/ctcaer/hekate/releases/latest) (which is the recommended way to manage emummc) should automatically work as expected/be corrected.
      + **If you do not wish to use the above, you will need to manually correct your configuration file**.
  + General system stability improvements to enhance the user's experience.
+ Stratosphere is currently in the process of being re-written/refactored.
  + Stratosphere was my (SciresM's) first C++ project, ever -- the code written for it a year ago when I was learning C++ is/was of much lower quality than code written more recently.
  + Code is thus being re-rwitten for clarity/stlye/to de-duplicate functionality, with much being moved into libstratosphere.
  + Stratosphere will, after the rewrite, globally use the `sts::` namespace -- this should greatly enhancing libstratosphere's ability to provide functionality for system modules.
  + The rewritten modules consistently have lower memory footprints, and should be easier to maintain going forwards.
  + The `sm`, `boot`, `spl`, `ro`, and `loader` modules have been tackled so far.
+ General system stability improvements to enhance the user's experience.
## 0.9.1
+ Support was added for 8.1.0.
+ Please note, emummc is still considered **beta/experimental** -- this is not the inevitable bugfix update for it, although some number of bugs have been fixed. :)
+ General system stability improvements to enhance the user's experience.
## 0.9.0
+ Creport output was improved significantly.
  + Thread names are now dumped on crash in addition to 0x100 of TLS from each thread.
    + This significantly aids debugging efforts for crashes.
  + Support was added for 32-bit stackframes, so reports can now be generated for 32-bit games.
+ `dmnt`'s Cheat VM was extended to add a new debug opcode.
+ With thanks to/collaboration with @m4xw and @CTCaer, support was added for redirecting NAND to the SD card (emummc).
  + Please note, this support is very much **beta/experimental**.
    + It is quite likely we have not identified all bugs -- those will be fixed as they are reported over the next few days/weeks.
    + In addition, some niceties (e.g. having a separate Atmosphere folder per emummc instance) still need some thought put in before they can be implemented in a way that makes everyone happy.
    + If you are not an advanced user, you may wish to think about waiting for the inevitable 0.9.1 bugfix update before using emummc as your default boot option.
      + You may especially wish to consider waiting if you are using Atmosphere on a unit with the RCM bug patched.
  + Emummc is managed by editing the emummc section of "emummc/emummc.ini".
    + To enable emummc, set `emummc!emummc_enabled` = 1.
  + Support is included for redirecting NAND to a partition on the SD card.
    + This can be done by setting `emummc!emummc_sector` to the start sector of your partition (e.g., `emummc_sector = 0x1A010000`).
  + Support is also included for redirecting NAND to a collection of loose files on the SD card.
    + This can be done by setting `emummc!emummc_path` to the folder (with archive bit set) containing the NAND boot partitions' files "boot0" and "boot1", and the raw NAND image files "00", "01", "02", etc. (single "00" file with the whole NAND image requires exFAT mode while multipart NAND can be used in both exFAT and FAT32 modes).
  + The `Nintendo` contents directory can be redirected arbitrarily.
    + By default, it will be redirected to `emummc/Nintendo_XXXX`, where `XXXX` is the hexadecimal representation of the emummc's ID.
      + The current emummc ID may be selected by changing `emummc!emummc_id` in emummc.ini.
    + This can be set to any arbitrary directory by setting `emummc!emummc_nintendo_path`.
  + To create a backup usable for emummc, users may use tools provided by the [hekate](https://github.com/CTCaer/hekate) project.
  + If, when using emummc, you encounter a bug, *please be sure to report it* -- that's the only way we can fix it. :)
## 0.8.10
+ A bug was fixed that could cause incorrect system memory allocation on 5.0.0.
  + 5.0.0 should now correctly have an additional 12 MiB allocated for sysmodules.
+ Atmosphère features which check button presses now consider all controllers, isntead of just P1.
+ Support was added for configuring language/region on a per-game basis.
  + This is managed by editing `atmosphere/titles/<title id>/config.ini` for the game.
  + To edit title language, edit `override_config!override_language`.
    + The languages supported are `ja`, `en-US`, `fr`, `de`, `it`, `es`, `zh-CN`, `ko`, `nl`, `pt`, `ru`, `zh-TW`, `en-GB`, `fr-CA`, `es-419`, `zh-Hans`, `zh-Hant`.
  + To edit title region, edit `override_config!override_region`.
    + The regions supported are `jpn`, `usa`, `eur`, `aus`, `chn`, `kor`, `twn`.
+ Atmosphère now provides a reimplementation of the `boot` system module.
  + `boot` is responsible for performing hardware initialization, showing the Nintendo logo, and repairing NAND on system update failure.
  + Atmosphère's `boot` implementation preserves AutoRCM during NAND repair.
    + NAND repair occurs when an unexpected shutdown or error happens during a system update.
    + This fixes a final edge case where AutoRCM might be removed by HOS, which could cause a user to burn fuses.
+ General system stability improvements to enhance the user's experience.
## 0.8.9
+ A number of bugs were fixed, including:
  + A data abort was fixed when mounting certain partitions on NAND.
  + All Stratosphère system modules now only maintain a connection to `sm` when actively using it.
    + This helps mitigate the scenario where sm hits the limit of 64 active connections and crashes.
    + This sometimes caused crashes when custom non-Atmosphère sysmodules were active and the user played certain games (ex: Smash's Stage Builder).
  + fatal now uses the 8.0.0 clkrst API, instead of silently failing to adjust clock rates on that firmware version.
  + A wait loop is now performed when trying to get a session to `sm`, in the case where `sm:` is not yet registered.
    + This fixes a race condition that could cause a failure to boot under certain circumstances.
  + libstratosphere's handling of domain object closing has been improved.
    + Previously, this code could cause crashes/extremely odd behavior (misinterpreting what object a service is) under certain circumstances.
+ An optional automatic reboot timer was added to fatal.
  + By setting the system setting `atmosphere!fatal_auto_reboot_interval` to a non-zero u64 value, fatal can be made to automatically reboot after a certain number of milliseconds.
  + If the setting is zero or not present, fatal will wait for user input as usual.
+ Atmosphère now provides a reimplementation of the `ro` system module.
  + `ro` is responsible for loading dynamic libraries (NROs) on 3.0.0+.
    + On 1.0.0-2.3.0, this is handled by `loader`.
  + Atmosphere's `ro` provides this functionality (`ldr:ro`, `ro:dmnt`) on all firmware versions.
  + An extension was implemented to provide support for applying IPS patches to NROs.
    + All patches at paths like /atmosphere/nro_patches/<user-defined patch name>/<Hex Build-ID for NRO to patch>.ips will be applied, allowing for easy distribution of patches.
    + Both the IPS and IPS32 formats are supported.
+ Atmosphère now provides a reimplementation of the `spl` system module.
  + `spl` (Secure Platform Services) is responsible for cryptographic operations, including all communications with the secure monitor (exosphère).
  + In the future, this may be used to provide extensions to the API for interacting with exosphère from userland.
+ General system stability improvements to enhance the user's experience.
## 0.8.8
+ Support was added for firmware version 8.0.0.
+ Custom exception handlers were added to stratosphere modules.
  + If a crash happens in a core atmosphere module now, instead of silently failing a reboot will occur to log the information to the SD card.
+ A bug was fixed in creport that caused games to hang when crashing under certain circumstances.
+ A bug was fixed that prevented maintenance mode from booting on 7.0.0+.
+ General system stability improvements to enhance the user's experience.
## 0.8.7
+ A few bugs were fixed that could cause fatal to fail to show an error under certain circumstances.
+ A bug was fixed that caused an error when launching certain games (e.g. Hellblade: Senua's Sacrifice).
  + Loader had support added in ams-0.8.4 for a new (7.0.0+) flag bit in NPDMs during process creation, but forgot to allow this bit to be set when validating the NPDM.
+ dmnt's cheat virtual machine received new instructions.
  + These allow for saving, restoring, or clearing registers to a secondary bank, effectively doubling the number of values that can be stored.
+ SHA256 code has been swapped from linux code to libnx's new hw-accelerated cryptography API.
+ Extensions were added to smcGetInfo:
  + A ConfigItem was added to detect whether the current unit has the RCM bug patched.
  + A ConfigItem was added to retrieve the current Atmosphère build hash.
+ Exosphère now tells the kernel to enable user-mode exception handlers, which should allow for better crash reporting/detection from Atmosphère's modules in the future..
+ Opt-in support was added for redirecting game save files to directories on the SD card.
  + Please note, this feature is **experimental**, and may cause problems. Please use at your own risk (and back up your saves before enabling it), as it still needs testing.
  + This can be enabled by setting `atmosphere!fsmitm_redirect_saves_to_sd` to 1 in `system_settings.ini`.
+ General system stability improvements to enhance the user's experience.
## 0.8.6
+ A number of bugs were fixed, including:
  + A case of inverted logic was fixed in fs.mitm which prevented the flags system from working correctly.
  + Time service access was corrected in both creport/fatal.
    + This fixes the timestamps used in fatal/crash report filenames.
  + A coherency issue was fixed in exosphère's Security Engine driver.
    + This fixes some instability issues encountered when overclocking the CPU.
  + Loader now unmaps NROs correctly, when ldr:ro is used.
    + This fixes a crash when repeatedly launching the web applet on < 3.0.0.
  + Usage of hidKeysDown was corrected to hidKeysHeld in several modules.
    + This fixes a rare issue where keypresses may have been incorrectly detected.
  + An issue with code filesystem unmounting was fixed in loader.
    + This issue could occasionally cause a fatal error 0x1015 to be thrown on boot.
  + Two bugs were fixed in the implementations of dmnt's cheat virtual machine.
    + These could cause cheats to work incorrectly under certain circumstances.
  + PM now uses a static buffer instead of a dynamically allocated one during process launch.
    + This fixes a memory exhaustion problem when building with gcc 8.3.0.
  + A workaround for a deadlock bug in Horizon's kernel on >= 6.0.0 was added in dmnt.
    + This prevents a system hang when booting certain titles with cheats enabled (ex: Mario Kart 8 Deluxe).
  + set.mitm now reads the system firmware version directly from the system version archive, instead of calling into set:sys.
    + This fixes compatibility with 1.0.0, which now successfully boots again.
+ dmnt's cheat virtual machine had some instruction set changes.
  + A new opcode was added for beginning conditional blocks based on register contents.
  + More addressing modes were added to the StoreRegisterToAddress opcode.
  + These should allow for more complex cheats to be implemented.
+ A new system for saving the state of cheat toggles between game boots was added.
  + Toggles are now saved to `atmosphere/titles/<title id>/cheats/toggles.txt` when either toggles were successfully loaded from that file or the system setting `atmosphere!dmnt_always_save_cheat_toggles` is non-zero.
  + This removes the need for manually setting cheats from all-on or all-off to the desired state on each game boot.
+ The default behavior for loader's HBL support was changed.
  + Instead of launching HBL when album is launched without R held, loader now launches HBL when album or any game is launched with R held.
  + Loader will now override any app in addition to a specific title id when `hbl_config!override_any_app` is true in `loader.ini`.
    + Accordingly, the `hbl_config!title_id=app` setting was deprecated. Support will be removed in Atmosphère 0.9.0.
+ First-class support was added to loader and fs.mitm for enabling homebrew to launch web applets.
  + Loader will now cause the "HtmlDocument" NCA path to resolve for whatever title HBL is taking over, even if it would not normally do so.
  + fs.mitm will also now cause requests to mount the HtmlDocument content for HBL's title to open the `sdmc:/atmosphere/hbl_html` folder.
    + By default, this just contains a URL whitelist.
+ General system stability improvements to enhance the user's experience.
## 0.8.5
+ Support was added for overriding content on a per-title basis, separate from HBL override.
  + This allows for using mods on the same title that one uses to launch HBL.
  + By default, `!L` is used for title content override (this is configurable by editing `default_config!override_key` in `loader.ini`)
  + This key combination can be set on a per-title basis by creating a `atmosphere/titles/<title id>/config.ini`, and editing `override_config!override_key`.
+ Content headers were added for the embedded files inside of fusee-secondary.
  + This will allow non-fusee bootloaders (like `hekate`) to extract the components bundled inside release binaries.
  + This should greatly simplify the update process in the future, for users who do not launch Atmosphère using fusee.
+ Support for cheat codes was added.
  + These are handled by a new `dmnt` sysmodule, which will also reimplement Nintendo's Debug Monitor in the future.
  + Cheat codes can be enabled/disabled at application launch via a per-title key combination.
    + For details, please see the [cheat loading documentation](https://github.com/Atmosphere-NX/Atmosphere/blob/master/docs/cheats.md#cheat-loating-process).
  + Cheat codes are fully backwards compatible with the pre-existing format, although a number of bugs have been fixed and some new features have been added.
    + For details, please see [the compatibility documentation](https://github.com/Atmosphere-NX/Atmosphere/blob/master/docs/cheats.md#cheat-code-compatibility).
  + An HIPC service API was added (`dmnt:cht`), that will allow user homebrew to interface with and control Atmosphère's cheat manager.
    + Please see [the relevant documentation](https://github.com/Atmosphere-NX/Atmosphere/blob/master/docs/modules/dmnt.md).
    + Full client code can be found in [libstratosphere](https://github.com/Atmosphere-NX/libstratosphere/blob/master/include/stratosphere/services/dmntcht.h).
    + Users interested in interfacing should see [EdiZon](https://github.com/WerWolv/EdiZon), which should have support for interfacing with Atmosphère's API shortly after 0.8.5 releases.
+ A bug was fixed that would cause Atmosphère's fatal screen to not show on 1.0.0-2.3.0.
+ A bug was fixed that caused Atmosphère's automatic ProdInfo backups to be corrupt.
+ General system stability improvements to enhance the user's experience.
## 0.8.4
+ Support for 7.0.0/7.0.1 was added.
  + This is facilitated through a new payload, `sept`, which can be signed, encrypted, and then loaded by Nintendo's TSEC firmware.
  + `sept` will derive the keys needed to boot new firmware, and then load `sept/payload.bin` off the SD card and jump to it.
+ Recognition of applications for override/mitm has been improved.
  + Nintendo's official Title ID range (`0x0100000000000000`-`0x01FFFFFFFFFFFFFF`) is now enforced.
+ A deadlock condition was fixed involving libstratosphere mitm sysmodules.
+ Kernel patches for JIT support were added (Thanks, @m4xw!).
  + These loosen restrictions on caller process in svcControlCodeMemory.
+ `set.mitm` and `fs.mitm` were merged into a single `ams_mitm` sysmodule.
  + This saves a process ID, allowing users to run one additional process up to the 0x40 process limit.
+ A `bpc.mitm` component was added, performing custom behavior on shutdown/reboot requests from `am` or applications.
  + Performing a reboot from the reboot menu now reboots to atmosphere. This can be configured via `system_settings.ini`.
  + Performing a shutdown from the reboot menu now works properly with AutoRCM, and does a real shutdown.
+ General system stability improvements to enhance the user's experience.
## 0.8.3
+ A custom warmboot firmware was implemented, which does not perform anti-downgrade fuse checks.
  + This fixes sleep mode when using a downgraded NAND.
  + This also removes Atmosphère's final dependency on Nintendo's encrypted PK11 binary; all components are now re-implemented.
+ The ExternalContentSource API was changed to not clear on failure.
+ Content override now supports an "app" setting, that causes all applications to be overridden with HBL instead of a specific title.
  + Note: because override keys are system-wide, using this setting will prevent using mods in games (as every game will be HBL).
+ A bug was fixed causing incorrect fatal-error output when svcBreak was called on 5.0.0+.
+ An extension was added to set.mitm to support customization of system settings.
  + These are controlled by `atmosphere/system_settings.ini`, see [here](https://github.com/Atmosphere-NX/Atmosphere/blob/master/docs/modules/set_mitm.md) for documentation.
+ An extension was added to sm, adding a new `sm:dmnt` service.
  + This can be used by a debug monitor in order to debug the registration state of various other services.
+ A bug was fixed in the MitM API that could sometimes cause a system hang during boot.
+ A change was made to the MitM API: in cases where sm would have returned 0xE15 when installing a mitm service, it now defers the result (following GetService semantics).
+ Support for booting into maintenance mode by holding +/- was added to PM.
+ An extension was added to exosphere, adding a custom SMC that allows for DMA to IRAM.
+ In addition, smcGetConfig was extended to reboot to a payload in IRAM at 0x40010000 when ConfigItem 65001 is set to 2.
  + Fatal will now use this to reboot to sdmc:/atmosphere/reboot_payload.bin if present, when a vol button is pressed.
  + An example homebrew ("reboot_to_payload") was also written and is now included with Atmosphère.
+ General system stability improvements to enhance the user's experience.
## 0.8.2
+ A number of bugs were fixed causing users to sometimes see `Key Derivation Failed!`.
  + KFUSE clock enable timings have been adjusted to allow time to stabilize before TSEC is granted access.
  + A race condition was fixed that could cause wrong key data to be used on 6.2.0
  + The TSEC firmware is now retried on failure, fixing a failure affecting ~1/50 boots on 6.2.0.
+ A bug was fixed causing some modules to not work on firmware 1.0.0.
+ A bug was fixed causing sleep mode to not work with debugmode enabled.
  + As a result, debugmode is now enabled in the default BCT.ini.
+ General system stability improvements to enhance the user's experience.
## 0.8.1
+ A bug was fixed causing users to see `Failed to enable SMMU!` if fusee had previously rebooted.
  + This message will still occur sporadically if fusee is not launched from coldboot, but it can never happen twice in a row.
+ A race condition was fixed in Atmosphere `bis_protect` functionality that could cause NS to be able to overwrite BCT public keys.
  + This sometimes broke AutoRCM protection, the current fix has been tested on hardware and verified to work.
+ Support was added for enabling `debugmode` based on the `exosphere` section of `BCT.ini`:
  + Setting `debugmode = 1` will cause exosphere to tell the kernel that debugmode is active.
  + Setting `debugmode_user = 1` will cause exosphere to tell userland that debugmode is active.
  + These are completely independent of one another, allowing fine control of system behavior.
+ Support was added for `nogc` functionality; thanks to @rajkosto for the patches.
  + By default, `nogc` patches will automatically apply if the user is booting into 4.0.0+ with fuses from <= 3.0.2.
  + Users can override this functionality via the `nogc` entry in the `stratosphere` section of `BCT.ini`:
    + Setting `nogc = 1` will force enable `nogc` patches.
    + Setting `nogc = 0` will force disable `nogc` patches.
  + If patches are enabled but not found for the booting system, a fatal error will be thrown.
    + This should prevent running FS without `nogc` patches after updating to an unsupported system version.
+ An extension was added to `exosphere` allowing userland applications to cause the system to reboot into RCM:
  + This is done by calling smcSetConfig(id=65001, value=<nonzero>); user homebrew can use splSetConfig for this.
+ On fatal error, the user can now choose to perform a standard reboot via the power button, or a reboot into RCM via either volume button.
+ A custom message was added to `fatal` for when an Atmosphère API version mismatch is detected (2495-1623).
+ General system stability improvements to enhance the user's experience.
## 0.8.0
+ A custom `fatal` system module was added.
  + This re-implements and extends Nintendo's fatal module, with the following features:
    + Atmosphère's `fatal` does not create error reports.
    + Atmosphère's `fatal` draws a custom error screen, showing registers and a backtrace.
    + Atmosphère's `fatal` attempts to gather debugging info for all crashes, and not just ones that include info.
    + Atmosphère's `fatal` will attempt saving reports to the SD, if a crash report was not generated by `creport`.
+ Title flag handling was changed to prevent folder clutter.
  + Instead of living in `atmosphere/titles/<tid>/%s.flag`, flags are now located in `atmosphere/titles/<tid>/flags/%s.flag`
    + The old format will continue to be supported for some time, but is deprecated.
  + Flags can now be applied to HBL by placing them at `atmosphere/flags/hbl_%s.flag`.
+ Changes were made to the mitm API, greatly improving caller semantics.
  + `sm` now informs mitm services of a new session's process id, enabling custom handling based on title id/process id.
+ smhax is no longer enabled, because it is no longer needed and breaks significant functionality.
  + Users with updated HBL/homebrew should see no observable differences due to this change.
+ Functionality was added implementing basic protections for NAND from userland homebrew:
  + BOOT0 now has write protection for the BCT public key and keyblob regions.
    + The `ns` sysmodule is no longer allowed to write the BCT public keys; all other processes can.
      + This should prevent system updates from removing AutoRCM.
    + No processes should be allowed to write to the keyblob region.
  + By default, BIS partitions other than BOOT0 are now read-only, and CAL0 is neither readable nor writable.
    + Adding a `bis_write` flag for a title will allow it to write to BIS.
    + Adding a `cal_read` flag for a title will allow it to read CAL0.
  + An automatic backup is now made of CAL0 on boot.
    + `fs.mitm` maintains a file handle to this backup, so userland software cannot read it.
  + To facilitate this, `fs.mitm` now mitms all sessions for non-system modules; content overriding has been made separate from service interception.
  + Please note: these protections are basic, and sufficiently malicious homebrew ++can defeat them++.
    + Please be careful to only run homebrew software from sources that you trust.
+ A bug involving HDCP titles crashing on newer firmwares was fixed.
+ Support was added for system version 6.2.0; our thanks to @motezazer for his invaluable help.
  + By default, new keys will automatically be derived without user input.
  + Support is also present for loading new keys from `atmosphere/prod.keys` or `atmosphere/dev.keys`
+ General system stability improvements to enhance the user's experience.
## 0.7.5
+ DRAM training was added to fusee-secondary, courtesy @hexkyz.
  + This greatly improves the speed of memory accesses during boot, resulting in a boot time that is ~200-400% faster.
+ creport has had its code region detection improved.
  + Instead of only checking one of the crashing thread's PC/LR for code region presence, creport now checks both + every address in the stacktrace. This is also now done for every thread.
    + This matches the improvement Nintendo added to official creport in 6.1.0.
  + The code region detection heuristic was further improved by checking whether an address points to .rodata or .rwdata, instead of just .text.
  + This means that a crash appears in a loaded NRO (or otherwise discontiguous) code region, creport will be able to detect all active code regions, and not just that one.
## 0.7.4
+ [libstratosphere](https://github.com/Atmosphere-NX/libstratosphere) has been completely refactored/rewritten, and split into its own, separate submodule.
  + While this is mostly "under the hood" for end-users, the refactor is faster (improving both boot-time and runtime performance), more accurate (many of the internal IPC structures are now bug-for-bug compatible with Nintendo's implementations), and significantly more stable (it fixes a large number of bugs present in the old library).
  + The refactored API is significantly cleaner and easier to write system module code for, which should improve/speed up development of stratosphere.
  + Developers looking to write their own custom system modules for the Switch can now easily include libstratosphere as a submodule in their projects.
+ Loader was extended to add a new generic way to redirect content (ExternalContentSources), courtesy @misson20000:
  + A new command was added to ldr:shel, taking in a tid to redirect and returning a session handle.
  + When the requested TID is loading, Loader will query the handle as though it were an IFileSystem.
    + This allows clients to generically define their own filesystems, and override content with them in loader.
+ fs.mitm has gotten several optimizations that should improve its performance and stability:
  + RomFS redirection now only occurs when there is content to redirect, even if the title is being mitm'd elsewhere.
  + A cache is now maintained of the active data storage, if any, for all opened title IDs. This means if two processes both try to open the same archive, fs.mitm won't duplicate any of its work.
  + RomFS metadata is now cached to the SD card on build instead of being persisted in memory -- this greatly reduces memory footprint and allows fs.mitm to redirect more titles simultaneously than before.
+ A number of bugs were fixed, including:
  + A resource leak was fixed in process creation. This fixes crashes that occur when a large number (>32) games have been launched since the last reboot.
  + fs.mitm no longer errors when receiving a zero-sized buffer. This fixes crashes in some games, including The Messenger.
  + Multi-threaded server semantics should no longer cause deadlocks in certain circumstances. This fixes crashes in some games, including NES Classics.
  + PM now only gives full FS permissions to the active KIPs. This fixes a potential crash where new processes might be unable to be registered with FS.
+ The `make dist` target now includes the branch in the generated zip name.
+ General system stability improvements to enhance the user's experience.
## 0.7.3
+ Loader and fs.mitm now try to reload loader.ini before reading it. This allows for changing the override button combination/HBL title id at runtime.
+ Added a MitM between set:sys and qlaunch, used to override the system version string displayed in system settings.
  + The displayed system version will now display `<Actual version> (AMS <x>.<y>.<z>)`.
+ General system stability improvements to enhance the user's experience.
## 0.7.2
+ Fixed a bug in fs.mitm's LayeredFS read implementation that caused some games to crash when trying to read files.
+ Fixed a bug affecting 1.0.0 that caused games to crash with fatal error 2001-0106 on boot.
+ Improved filenames output by the make dist target.
+ General system stability improvements to enhance the user's experience.

## 0.7.1
+ Fixed a bug preventing consoles on 4.0.0-4.1.0 from going to sleep and waking back up.
+ Fixed a bug preventing consoles on < 4.0.0 from booting without specific KIPs on the SD card.
+ An API was added to Atmosphère's Service Manager for deferring acquisition of all handles for specific services until after early initialization is completed.
+ General system stability improvements to enhance the user's experience.

## 0.7.0
+ First official release of Atmosphère.
+ Supports the following featureset:
  + Fusée, a custom bootloader.
    + Supports loading/customizing of arbitrary KIPs from the SD card.
    + Supports loading a custom kernel from the SD card ("/atmosphere/kernel.bin").
    + Supports compile-time defined kernel patches on a per-firmware basis.
    + All patches at paths like /atmosphere/kip_patches/<user-defined patch name>/<SHA256 of KIP>.ips will be applied to the relevant KIPs, allowing for easy distribution of patches supporting multiple versions.
      + Both the IPS and IPS32 formats are supported.
    + All patches at paths like /atmosphere/kernel_patches/<user-defined patch name>/<SHA256 of Kernel>.ips will be applied to the kernel, allowing for easy distribution of patches supporting multiple versions.
      + Both the IPS and IPS32 formats are supported.
    + Configurable by editing BCT.ini on the SD card.
    + Atmosphère should also be launchable by the alternative hekate bootloader, for those who prefer it.
  + Exosphère, a fully-featured custom secure monitor.
    + Exosphere is a re-implementation of Nintendo's TrustZone firmware, fully replicating all of its features.
    + In addition, it has been extended to provide information on current Atmosphere API version, for homebrew wishing to make use of it.
  + Stratosphère, a set of custom system modules. This includes:
    + A loader system module.
      + Reimplementation of Nintendo's loader, fully replicating all original functionality.
      + Configurable by editing /atmosphere/loader.ini
      + First class support for the Homebrew Loader.
        + An exefs NSP (default "/atmosphere/hbl.nsp") will be used in place of the victim title's exefs.
        + By default, HBL will replace the album applet, but any application should also be supported.
      + Extended to support arbitrary redirection of executable content to the SD card.
        + Files will be preferentially loaded from /atmosphere/titles/<titleid>/exefs/, if present.
        + Files present in the original exefs a user wants to mark as not present may be "stubbed" by creating a .stub file on the SD.
        + If present, a PFS0 at /atmosphere/titles/<titleid>/exefs.nsp will fully replace the original exefs.
        + Redirection is optionally toggleable by holding down certain buttons (by default, holding R disables redirection).
      + Full support for patching NSO content is implemented.
        + All patches at paths like /atmosphere/exefs_patches/<user-defined patch name>/<Hex Build-ID for NSO to patch>.ips will be applied, allowing for easy distribution of patches supporting multiple firmware versions and/or titles.
        + Both the IPS and IPS32 formats are supported.
      + Extended to support launching content from loose executable files on the SD card, without requiring any official installation.
        + This is done by specifying FsStorageId_None on launch.
    + A service manager system module.
      + Reimplementation of Nintendo's service manager, fully replicating all original functionality.
      + Compile-time support for reintroduction of "smhax", allowing clients to optionally skip service access verification by skipping initialization.
      + Extended to allow homebrew to acquire more handles to privileged services than Nintendo natively allows.
      + Extended to add a new API for installing Man-In-The-Middle listeners for arbitrary services.
        + API can additionally be used to safely detect whether a service has been registered in a non-blocking way with no side-effects.
        + Full API documentation to come.
    + A process manager system module.
      + Reimplementation of Nintendo's process manager, fully replicating all original functionality.
      + Extended to allow homebrew to acquire handles to arbitrary processes, and thus read/modify system memory without blocking execution.
      + Extended to allow homebrew to retrieve information about system resource limits.
      + Extended by embedding a full, extended implementation of Nintendo's boot2 system module.
        + Title launch order has been optimized in order to grant access to the SD card faster.
        + The error-collection system module is intentionally not launched, preventing many system telemetry error reports from being generated at all.
        + Users may place their own custom sysmodules on the SD card and flag them for automatic boot2 launch by creating a /atmosphere/titles/<title ID>/boot2.flag file on their SD card.
    + A custom fs.mitm system module.
      + Uses Atmosphère's MitM API in order to provide an easy means for users to modify game content.
      + Intercepts all FS commands sent by games, with special handling for commands used to mount RomFS/DLC content to enable easy creation and distribution of game/DLC mods.
        + fs.mitm will parse the base RomFS image for a game, a RomFS image located at /atmosphere/titles/<title ID>/romfs.bin, and all loose files in /atmosphere/titles/<title ID>/romfs/, and merge them together into a single RomFS image.
        + When merging, loose files are preferred to content in the SD card romfs.bin image, and files from the SD card image are preferred to those in the base image.
      + Can additionally be used to intercept commands sent by arbitrary system titles (excepting those launched before SD card is active), by creating a /atmosphere/titles/<title ID>/fsmitm.flag file on the SD card.
      + Can be forcibly disabled for any title, by creating a /atmosphere/titles/<title ID>/fsmitm_disable.flag file on the SD card.
      + Redirection is optionally toggleable by holding down certain buttons (by default, holding R disables redirection).
    + A custom crash report system module.
      + Serves as a drop-in replacement for Nintendo's own creport system module.
      + Generates detailed, human-readable reports on system crashes, saving to /atmosphere/crash_reports/<timestamp>_<title ID>.log.
      + Because reports are not sent to the erpt sysmodule, this disables all crash report related telemetry.
  + General system stability improvements to enhance the user's experience.
