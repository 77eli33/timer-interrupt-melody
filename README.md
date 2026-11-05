# Timer Interrupt Melody (EK-TM4C129EXL + BOOSTXL-EDUMKII)

Lab 2: plays melodies on the BoosterPack buzzer. S1 selects the next melody,
S2 starts/stops playback. Timer5A generates the tone frequency, Timer4A the
note duration, SysTick (1 ms) drives button polling with 4-sample debouncing.

## Import into Code Composer Studio
1. File -> Import Projects... -> select this folder (or the .zip).
2. Set the TivaWare path once: Project -> Properties -> Build -> Variables ->
   `SW_ROOT` = your `TivaWare_C_Series-2.2.0.295` folder
   (preset: `C:/ti/TivaWare_C_Series-2.2.0.295`, the TI installer default).
3. Build (compiler TI v20.2.7.LTS) and debug/flash via Stellaris ICDI.

## Layout
- `src/main.c` - setup and main loop (melody control)
- `src/tone.c/.h` - buzzer PG1, Timer5A (frequency) + Timer4A (duration)
- `src/buttons.c/.h` - S1 (PH1) / S2 (PK6), polling + debouncing
- `src/melodies.h` - note table
- `src/tm4c129encpdt_startup_ccs.c` - vector table: SysTick, Timer4A, Timer5A
- `tm4c129encpdt.cmd` - linker command file
