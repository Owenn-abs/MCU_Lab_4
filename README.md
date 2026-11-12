# Current firmware: five LEDs and UART (2026-11-05)

This section supersedes the archived display design below. Target: STM32F103C6U.
PA3 red, PA4 yellow, PA5 green, PA6 blue, PA7 pink toggle every 500, 1000,
1500, 2000 and 2500 ms respectively. LEDs are active LOW, initially HIGH/OFF,
matching the supplied +3.3 V anode wiring. Add a series resistor per LED.
The seven-segment driver is retired and compiled out.

USART1 uses PA9 TX and PA10 RX at **115200 baud, 8 data bits, no parity,
1 stop bit, no flow control**. Connect PA9 to Virtual Terminal RXD and PA10
to its TXD. Load `build/Lab_4.hex` or `build/Lab_4.elf` and use an 8 MHz MCU clock.
Refresh CubeIDE with F5 after external changes.

If the terminal shows repeated symbols or unreadable characters, open the
Proteus Virtual Terminal properties and set its baud rate to **115200**, data
bits to **8**, parity to **None**, and stop bits to **1**. Check that its RXD is
connected to MCU PA9 (TX), with a common ground. Verify the simulated MCU
clock is **8 MHz**. The firmware uses HSI at 8 MHz with PLL disabled; USART1
derives its baud clock from the 8 MHz APB2 clock. A terminal left at 9600 baud
cannot decode this firmware's output. Keep 115200 for the 10 ms timestamp
stream: 9600 baud cannot carry one complete line every 10 ms.

Example output:

```text
[10ms] 10ms callback at 1 tick
[350ms] One-shot 350ms at 35 tick
[500ms] Task 1 RED toggle at 50 tick
[600ms] Delete test PASS at 60 tick
[750ms] One-shot 750ms at 75 tick
[1000ms] Task 2 YELLOW toggle at 100 tick
```

Every diagnostic callback queues its actual `get_time()` value. A 2048-byte
FIFO transmits from the main loop without blocking or UART interrupts.
`g_serial_dropped` must remain zero; a full queue drops a whole line and counts
the failure. Transmission latency does not change the timestamp captured by
the callback. PA10 is configured for reception but no receive commands are used.

The 16-slot scheduler supports concurrent timeouts, index IDs, deletion,
one-shots, earliest-deadline ordering, and phase-preserving skipped periods.
Its `sTask` has the PDF's five fields: `pTask`, `Delay`, `Period`, `RunMe`,
and `TaskID`. `SCH_Update()` only increments the 10 ms tick in the TIM2
interrupt. The main-loop dispatcher computes due tasks using a side array of
absolute deadlines and updates `Delay`/`RunMe`. Add/Delete/Dispatch use short
critical sections. Every `SCH_Add_Task()` call for
the live demo is visible in `main.c`
after `SCH_Init()` and `App_Init()` and before TIM2 is started. The five
LEDs use `DELAY=PERIOD` so their first toggle happens after a full period.
The 750 ms one-shot is registered before 350 ms to test deadline ordering.
The cancellation test deletes a 1200 ms callback at 600 ms and verifies that
a second deletion fails. `SCH_Update()` is O(1); task searching runs in the
main-loop dispatcher. This follows the PDF's O(1) demonstration criterion.
All tasks register before the timer starts; five LEDs plus the 10 ms diagnostic
callback run independently. Host tests exercise Init, Update, Add, Delete,
Dispatch and get_time, including capacity, invalid IDs, wraparound and ordering.

`SCH_Add_Task()` follows the PDF example: it returns the assigned array index
(`0..15`), and returns `SCH_MAX_TASKS` (`16`, named `SCH_INVALID_TASK_ID` here)
when registration fails. `SCH_Delete_Task()` accepts an active index. A freed
index can be reused, so callers must discard saved IDs after deletion; an old
ID could otherwise refer to a new task in that slot.

**Unresolvable handout contradiction:** a regular 10 ms interrupt is 100 Hz.
It cannot simultaneously satisfy a maximum of 10 Hz. This build prioritizes
the explicit 10 ms tick/callback demonstration. HAL SysTick also runs at 1 kHz.
Instructor clarification is required for the conflicting frequency criterion;
this firmware does not claim to satisfy that criterion.

Build: `python build.py`. Test: `python tests/run_tests.py`.
Proteus execution and physical serial timing remain to be checked by the user.

---

# Archived design notes (superseded; not current wiring or build sizes)

This project targets **STM32F103C6Ux, UFQFPN48**, with 32 KiB Flash and
10 KiB SRAM. The user's C6U correction and explicit GPIO assignments override
the attached plan's C6T package and old GPIO table. The source plan is retained
unchanged outside this project.

## Wiring contract

Segments are in **a, b, c, d, e, f, g order**, with two independent,
continuously driven common-anode displays. No multiplexing is used.

| Function | GPIO(s) | UFQFPN48 pin(s) | Action |
|---|---|---|---|
| RED_LED, task 1 | PA3 | 13 | Toggle every 500 ms |
| YELLOW_LED, task 2 | PA4 | 14 | Toggle every 1000 ms |
| GREEN_LED, task 3 | PA5 | 15 | Toggle every 1500 ms |
| First display, task 4, segments a–g | PB0, PB1, PB2, PB3, PB4, PB5, PB6 | 18, 19, 20, 39, 40, 41, 42 | Count up every 2000 ms |
| Second display, task 5, segments a–g | PA8, PA9, PA10, PA11, PA12, PA13, PA14 | 29, 30, 31, 32, 33, 34, 37 | Count down every 2500 ms |
| First display anode driver enable | PA7 | 17 | LOW enables |
| Second display anode driver enable | PA6 | 16 | LOW enables |
| Optional timer scope marker | PB14 | 27 | Toggle each TIM2 tick |
| Optional callback scope marker | PB15 | 28 | Toggle each 10 ms callback |

PA6 replaces the plan's second-anode PA8 assignment because PA8 now drives
segment a of the second display. PA7 remains the first-anode enable. These
enable outputs control external high-side drivers, such as PNP transistors
or P-channel MOSFETs; they must not source the entire display current directly.
Use a resistor for each segment and LED (330 ohms is a simulation starting
point; check actual current). Use 3.3 V GPIO levels and a common ground.

Standalone LEDs are active HIGH. Segments are active LOW. Anode polarity is
set once in `SEVENSEG_ANODE_ACTIVE_LOW` in `Core/Inc/sevenseg.h`; the default is
active LOW. Both anodes start disabled, segment outputs start HIGH, then the
initial patterns are written and the anodes enabled. Displays show **0 and 9**
at startup, and all three LEDs start OFF. Decimal points are unused.

Both **JTAG and SWD are disabled** in `HAL_MspInit`, and CubeMX SYS is set to
No Debug. This frees PB3/PB4 and PA13/PA14 for the specified segment buses.
SWD debugging is therefore unavailable after initialization on physical
hardware. Use connect-under-reset for recovery/programming as supported by
your probe. PB2 is also BOOT1: keep BOOT0 LOW for normal Flash boot; do not
hard-wire PB2 to ground when using it as a segment output. Pin functions and
package numbering: [ST datasheet, Figure 6 and Table 5](https://www.st.com/resource/en/datasheet/stm32f103c4.pdf).

## Timing and scheduler

HSI supplies SYSCLK/HCLK/APB1/APB2 at 8 MHz, with PLL OFF. TIM2 uses its internal
clock, prescaler 7999 and ARR 9:

`(7999 + 1) * (9 + 1) / 8000000 = 0.010 seconds`.

After GPIO/timer initialization, `SCH_Init()` clears the scheduler and
`App_Init()` initializes displays, traces and tasks. All tasks register before
TIM2 starts. The initialization update flag and counter are cleared before
`HAL_TIM_Base_Start_IT`, avoiding a spurious tick at startup.

`TIM2_IRQHandler -> HAL_TIM_IRQHandler -> HAL_TIM_PeriodElapsedCallback`
calls `SCH_Update()` once. That function only increments a volatile 32-bit
tick: O(1), no task-table walk or callback. The normal main loop calls only
`SCH_Dispatch_Tasks()`.

The six lab APIs are declared in `scheduler.h`. Add-task delay and period
arguments are **10 ms ticks**, not milliseconds. Delay zero is eligible at
the next dispatch; period zero is a one-shot. The five application tasks use
delay = period = 50, 100, 150, 200 and 250 ticks. A full LED ON/OFF cycle is
twice its toggle interval.

A conventional super loop does not provide independent task periods, and
blocking delays make every other task wait. Here function pointers and
absolute deadlines provide independent scheduling, while callbacks execute
sequentially in main context. The dispatcher scans a static 16-slot table for
the oldest due deadline, breaking ties by registration ID. The ISR may
interrupt a callback; application callbacks never interrupt one another.

Periodic deadlines retain their original phase and skip missed periods,
counted in `g_missed_deadline_count`. Dispatch captures time once and executes
at most 16 callbacks per call to bound work, including callbacks that add new
delay-zero tasks. Pending work continues on the next main-loop pass. Records
are rescheduled/retired before callback invocation, allowing callbacks to
add/delete tasks safely. Recursive dispatch is ignored.

IDs are nonzero and never recycled within an initialized scheduler; exhaustion
fails instead of aliasing a stale handle. `SCH_Init` invalidates all handles.
Deletion returns 1 on success and 0 otherwise; add failure returns UINT32_MAX.
`SCH_GetLastError()` reports callback/range/full-table/ID errors. Table access
belongs to main context only; the timer ISR owns tick advancement. Naturally
aligned 32-bit tick reads are atomic on this target. Delays, periods and the
maximum interval without dispatch must stay below 2^31 ticks for wrap-safe
deadline comparisons. `get_time()` returns ticks * 10 in uint32 milliseconds
and wraps after approximately 49.7 days. No heap, RTOS, UART or blocking delays
are used by the application.

## Diagnostics and checkpoints

**RAM trace + scope pins replace textual printing for this no-UART design;
instructor approval is needed for strict print compliance.** This is a
documented deviation from the handout's demonstration on page 23.

The handout contains two conflicting requirements: a 10 ms regular
timer is 100 Hz, incompatible with a literal maximum interrupt rate of 10 Hz;
and the teaching O(n) tick traversal conflicts with the O(1) demonstration
criterion. This implementation uses the explicit 10 ms tick and O(1) update.
HAL SysTick still runs every 1 ms. These statements were checked directly
against pages 16 and 23 of `VXL_VDK_Lab_4_CooperativeScheduler.pdf`.

Inspect RAM using the simulator's memory/symbol facilities where supported
(physical SWD is disabled by this pin assignment):

- `g_trace[256]`: 8-byte timestamp/event/aux records, overwritten in a ring.
  `g_trace_count` is the total published record count; latest entry is
  `(g_trace_count - 1) % 256`. Pause execution when inspecting a consistent
  snapshot. The ring retains only roughly the last 2.4 seconds; capture early
  events before they are overwritten.
- `g_10ms_callback_count`: runs of the 10 ms timestamp callback.
- `g_app_task_count[5]`: total runs of tasks 1–5.
- `g_count_up`, `g_count_down`: current displayed digits.
- `g_oneshot_count[2]`: each becomes 1 at 350/750 ms. The 750 ms callback is
  deliberately registered before the 350 ms callback.
- `g_delete_test_passed`: becomes 1 at 600 ms, verifying deletion and repeated
  deletion. `g_cancelled_task_count` must stay zero (cancelled 1200 ms task).
- `g_missed_deadline_count`: zero under normal unloaded operation.

| Elapsed | Expected observation |
|---|---|
| Boot | LEDs off; first digit 0, second 9 |
| 350 / 750 ms | Corresponding one-shot once |
| 500 ms | Red first toggles |
| 1000 ms | Red second toggle, yellow first toggle |
| 1500 ms | Green first toggle |
| 2000 ms | First display becomes 1 |
| 2500 ms | Second display becomes 8 |
| 3000 ms | Red, yellow, green execute in that order |
| 10000 ms | Digits 5 and 5 |
| 20000 ms | Digits 0 and 1 |
| 22500 ms | Second digit 0 |
| 25000 ms | Second digit wraps to 9 |

## Build and run

From this project folder, with Python 3 and GNU Arm Embedded Toolchain on PATH:

```powershell
python build.py
python tests/run_tests.py
```

The build produces `build/Lab_4.elf`, `build/Lab_4.hex` and `build/Lab_4.map`
using C99 and the bundled CMSIS/HAL sources. Host tests use native GCC on PATH,
or the installed `C:/msys64/ucrt64/bin/gcc.exe`; override with `--cc` if needed.
Tests use fake GPIO registers and do not constitute a hardware simulation.

For optional PB14/PB15 timing outputs, run `python build.py --scope`; outputs
go to `build-scope/`. Each edge should be 10 ms apart, with 20 ms full waveform
periods. The optional toggle remains O(1), but actual jitter/overhead must be
measured in the simulator or on hardware.

Alternatively, open/import the existing `.project` in STM32CubeIDE, refresh
the project, then build Debug or Release. The `.ioc` keeps C6U/UFQFPN48, the
named pins, safe startup levels, No Debug and TIM2 settings. Custom application
code is in separate modules and `USER CODE` blocks to survive regeneration.
The direct GNU build was validated; CubeMX regeneration and the IDE UI build
have not been run in this session.

In Proteus, load the ELF (for symbols where supported) or HEX into the matching
STM32F103C6 model, use 8 MHz clock settings, wire the table above, and run the
checkpoints. Schematic creation/wiring and simulation remain manual work.
No Proteus simulation has been performed or claimed.

## Validation recorded 2026-11-04

- GNU Arm Embedded GCC 10.3.1 compiled/linked the C6U firmware successfully.
  Default image: 5,624 bytes Flash; linker reports 4,152 bytes RAM including
  reserved heap/stack (within 32 KiB/10 KiB). Generated `syscalls.c` emits
  pre-existing unused-parameter warnings under `-Wextra`; new modules compile
  without warnings.
- Optional scope-pin build also passed: 5,692 bytes Flash and 4,152 bytes RAM.
- Host GCC 14.2.0, C99, `-Wall -Wextra -Werror`: scheduler, GPIO and application
  tests pass, including every digit/polarity/pin, preservation of unrelated
  pins, reverse deadline order, ties, stale IDs, capacity, callback mutations,
  wraparound, ID exhaustion, missed periods and a 25-second task timeline.
- Physical electrical behavior, oscillator accuracy, Proteus timing and
  debugger visibility still require simulator/hardware validation.
