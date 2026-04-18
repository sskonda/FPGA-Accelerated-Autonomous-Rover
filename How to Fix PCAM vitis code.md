How to Fix PCAM vitis code

## Camera passthrough bring-up checklist

1. Create the app component on the **correct platform/domain**
   Use:

   * platform: `camera_motor_v2`
   * domain: `standalone_ps7_cortexa9_0`

2. Copy in the Digilent passthrough source tree
   The app needed the same source structure as the working passthrough app:

   * `main.cc`
   * `platform/`
   * `ov5640/`
   * `hdmi/`
   * `MIPI_CSI_2_RX_v1_0/`
   * `MIPI_D_PHY_RX_v1_0/`
   * `xparams_compat.h`
   * `assert_filename_fix.h`

3. Fix the compile source list in `UserConfig.cmake`
   Keep only the files actually needed to build the app.
   The passthrough app does **not** need the CSI/DPHY self-test files. Your earlier builds were compiling those extra self-test sources while also compiling the main video sources. 

   Use this source list:

   * `main.cc`
   * `MIPI_CSI_2_RX_v1_0/src/MIPI_CSI_2_RX.c`
   * `MIPI_D_PHY_RX_v1_0/src/MIPI_D_PHY_RX.c`
   * `ov5640/OV5640.cpp`
   * `platform/platform.c`

4. Fix the `__FILENAME__` build break
   The big compile failure came from the bad compiler define
   `-D__FILENAME__='__FILE__'`, which showed up directly in your build logs and caused the `Xil_Assert` type mismatch inside `OV5640.cpp` / `VideoOutput.h`. 
   `Xil_Assert` expects a filename pointer, not an integer-like character constant. ([AMD Documentation][1])

5. Keep `assert_filename_fix.h` and use it the right way
   The header itself was fine.
   The fix was to force-include it at **compile time** so `__FILENAME__` becomes `__FILE__`.

6. Put the forced include in `USER_COMPILE_OTHER_FLAGS`, not in linker flags
   One of the failed attempts had the `-include ${CMAKE_SOURCE_DIR}/assert_filename_fix.h` line in `USER_LINK_OTHER_FLAGS`, which does nothing for compiling `OV5640.cpp`. Your pasted `UserConfig.cmake` showed exactly that. 

   The working place is:

   * `USER_COMPILE_OTHER_FLAGS`

7. Clean hard after changing `UserConfig.cmake`
   After changing compile flags:

   * delete the app `build` folder
   * rebuild the app

   Otherwise stale generated files can keep reusing old bad settings.

8. Make sure the launch config points to the **actual ELF that was built**
   At one point Vitis tried to download `camera_passthrough.elf`, but the build logs showed the target being built as `pcam_passthrough.elf`. The launch config has to point to the real generated ELF, not a guessed filename. 
   AMD’s run flow is driven by the launch configuration for the selected application component. ([AMD Documentation][2])

9. Use `ps497view1

10. Use `ps7_init.tcl` for board initialization
    For Zynq run/debug launch configurations, `ps7_init.tcl` is the PS init script used by([AMD Customer Community][3])teturn319835search5

11. In the launch config, keep the normal Zynq bring-up boxes enabled
    For your working setup, the important run settings were:

* correct bitstream
* correct ELF
* correct `ps7_init.tcl`
* `Program Device` enabled
* `Reset Entire System` enabled

11. If the Flow Navigator **Run** button is gray, use the **Debug view play button**
    That is still a valid way to launch the app. AMD documents that running is controlled through the application’s launch configuration, and after a run you should terminate the se([AMD Documentation][2])rch0turn950497view1

12. If Vitis gets weird, restarting Vitis can clear stale run/debug state
    That is what got your `camera_passthrough` launch moving again.

## Minimal “known good” final state

For future test runs, your known good state is:

* `camera_passthrough` app builds successfully
* `UserConfig.cmake` compile sources only include the 5 real source files
* `assert_filename_fix.h` is compile-included
* no stale bad `__FILENAME__` define in the compile path
* launch config points at the real built ELF
* launch config uses `ps7_init.tcl`
* run from Debug-view play button if Flow Run is gray

## Fast retry procedure

When you want to rerun later, do this in order:

1. Open the correct workspace.
2. Select `camera_passthrough`.
3. Rebuild the app.
4. Open launch config.
5. Verify:

   * bitstream path
   * ELF path
   * `ps7_init.tcl`
6. Plug in board, Pcam, HDMI, UART.
7. Click play in Debug view.
8. After stopping, terminat([AMD Documentation][4])citeturn950497view1

set(USER_COMPILE_DEFINITIONS
)

set(USER_UNDEFINED_SYMBOLS
"__clang__"
)

set(USER_INCLUDE_DIRECTORIES
)

set(USER_COMPILE_SOURCES
"main.cc"
"MIPI_CSI_2_RX_v1_0/src/MIPI_CSI_2_RX.c"
"MIPI_D_PHY_RX_v1_0/src/MIPI_D_PHY_RX.c"
"ov5640/OV5640.cpp"
"platform/platform.c"
)

set(USER_COMPILE_OTHER_FLAGS
"-include ${CMAKE_SOURCE_DIR}/assert_filename_fix.h"
)

set(USER_LINK_OTHER_FLAGS
)

[1]: https://docs.amd.com/r/en-US/oslib_rm/Xil_Assert "Xil_Assert - Xil_Assert - 2025.2 English - UG643"
[2]: https://docs.amd.com/r/2024.2-English/ug1400-vitis-embedded/Launch-Configurations?contentId=WLCD6GM01pAngYS8PQCSXg&utm_source=chatgpt.com "Launch Configurations - 2024.2 English - UG1400"
[3]: https://adaptivesupport.amd.com/s/article/65956?utm_source=chatgpt.com "65956 - Zynq - How to make modifications to the ps7_init.tcl ..."
[4]: https://docs.amd.com/r/2024.2-English/ug1400-vitis-embedded/Running-the-Application-Component "Running the Application Component - Running the Application Component - 2024.2 English - UG1400"
