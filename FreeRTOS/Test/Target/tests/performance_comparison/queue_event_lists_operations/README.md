# Queue Event Lists Operations: listLIST_IS_EMPTY() in queue.c

Measures the elapsed time of the queue/semaphore operations in
[`Source/queue.c`](../../../../../Source/queue.c) that check
`listLIST_IS_EMPTY()` on an event list (`xTasksWaitingToSend` /
`xTasksWaitingToReceive`) before calling `xTaskRemoveFromEventList()`. The
goal is to evaluate the performance impact of moving that emptiness check
into `xTaskRemoveFromEventList()` itself, by comparing timings taken before
and after such a change.

See the header comment in
[`performance_comparison.c`](./performance_comparison.c) for how scenarios
are structured (`ScenarioConfig_t`, the generic timed task, etc.).

## How the test works

Each scenario runs on its own, one at a time, and involves up to three tasks:

- A **timed task**, which is the one actually being measured. In a tight
  loop, it calls `testGET_TIME_FUNCTION()` (a cycle counter), makes one
  call to the queue/semaphore function under test, calls
  `testGET_TIME_FUNCTION()` again, and adds the difference to a running
  total. It repeats this for many samples (usually 16384), then prints the
  accumulated and average elapsed time.
- An optional **helper task**, used only by the "waiting" scenarios. It
  sits permanently blocked on the same queue (e.g. stuck in
  `xQueueReceive()`), so that whenever the timed task's call runs, it finds
  that helper task on the event list and has to remove and wake it - this
  is what makes `listLIST_IS_EMPTY()` return `pdFALSE`. "No waiting"
  scenarios skip this task entirely, so the same event list is always
  empty and `listLIST_IS_EMPTY()` always returns `pdTRUE`.
- The **main task**, which just creates the above, waits for the timed
  task to finish all its samples, prints the result, then deletes
  everything and moves on to the next scenario.

Whichever state a scenario needs (empty list, or a task waiting on it) has
to be re-established after every sample, since the timed call itself
changes it (e.g. sending consumes the "queue has room" precondition). This
happens automatically: the helper task is given a higher priority than the
timed task, so the moment it gets woken it immediately runs to completion
and blocks again on its own, restoring the precondition before the timed
task's next iteration - no separate cleanup step needed. A couple of
functions (`xQueuePeek`, `prvNotifyQueueSetContainer`) can't settle that
way for structural reasons and use small explicit reset/delay steps
between samples instead; see the file header comment for why.

## Scenarios and the functions they cover

The only thing that changes which branch of the check under test runs is
whether the target event list is empty or not, so every function gets
exactly two scenarios: **"no waiting sender/receiver"** (list empty,
`listLIST_IS_EMPTY()` returns `pdTRUE`, the guarded block is skipped
entirely) and **"waiting sender/receiver"** (list non-empty,
`listLIST_IS_EMPTY()` returns `pdFALSE`, `xTaskRemoveFromEventList()` runs).
Which list a scenario is named after (`Sender` or `Receiver`) matches
whichever of `xTasksWaitingToSend` / `xTasksWaitingToReceive` that function
actually checks - not whether the function itself sends or receives (e.g.
`xQueueReceive()` checks `xTasksWaitingToSend`, so its scenarios are named
`...Sender`).

Priority of any helper task involved plays no part in *what* is measured -
it is only chosen (where a helper task is needed at all) to make the queue
settle back to the required state between samples automatic, by giving the
helper a higher priority than the timed task so the RTOS's own synchronous
yield does the settling. See the file header comment for the one exception
(`xQueuePeek`) and why.

Each scenario times its operation over many samples and prints the
accumulated and average elapsed time (in whatever unit
`testGET_TIME_FUNCTION()` returns for the board - see below).

| queue.c function | Which list | Test cases |
| --- | --- | --- |
| `xQueueGenericSend` (`xQueueSend`) | `xTasksWaitingToReceive` | `Test_PerformanceComparison_xQueueSend_NoWaitingReceiver`, `..._WaitingReceiver` |
| `xQueueGenericSendFromISR` (`xQueueSendFromISR`) | `xTasksWaitingToReceive` | `Test_PerformanceComparison_xQueueSendFromISR_NoWaitingReceiver`, `..._WaitingReceiver` |
| `xQueueReceive` | `xTasksWaitingToSend` | `Test_PerformanceComparison_xQueueReceive_NoWaitingSender`, `..._WaitingSender` |
| `xQueueReceiveFromISR` | `xTasksWaitingToSend` | `Test_PerformanceComparison_xQueueReceiveFromISR_NoWaitingSender`, `..._WaitingSender` |
| `xQueueSemaphoreTake` | `xTasksWaitingToSend` | `Test_PerformanceComparison_xQueueSemaphoreTake_NoWaitingSender`, `..._WaitingSender` |
| `xQueueGiveFromISR` | `xTasksWaitingToReceive` | `Test_PerformanceComparison_xQueueGiveFromISR_NoWaitingReceiver`, `..._WaitingReceiver` |
| `xQueuePeek` | `xTasksWaitingToReceive` | `Test_PerformanceComparison_xQueuePeek_NoWaitingReceiver`, `..._WaitingReceiver` |
| `xQueueGenericReset` (`xQueueReset`) | `xTasksWaitingToSend` | `Test_PerformanceComparison_xQueueReset_NoWaitingSender`, `..._WaitingSender` |
| `prvNotifyQueueSetContainer` | container's `xTasksWaitingToReceive` | `Test_PerformanceComparison_prvNotifyQueueSetContainer_NoWaitingReceiver`, `..._WaitingReceiver` (only built when `configUSE_QUEUE_SETS == 1`) |
| `prvUnlockQueue` | - | **Not covered** - see below. |

The `...FromISR` functions are still called from task context here, with
interrupts masked around the call to emulate the non-preemptible-by-a-real-
interrupt assumption they are written under (and to keep tick-interrupt
jitter out of the timed window).

## `prvUnlockQueue`: not covered

`prvUnlockQueue`'s `listLIST_IS_EMPTY()` checks only run when a `...FromISR`
call lands on a queue while a *different* task is inside the few-instruction
window between `prvLockQueue()` and `prvUnlockQueue()` in one of its own
blocking calls (`xQueueReceive`, `xQueueSend`, `xQueueSemaphoreTake`,
`xQueuePeek`, `vQueueWaitForMessageRestricted`). That window is not enterable
by scheduling tasks against each other - it requires a genuine interrupt
preempting that task's own instructions mid-way through, which is
platform-specific and not reliably hittable for a sampled loop. Left as a
known gap.

## Building and running

### QEMU Cortex-M3 (`boards/qemu_cortex_m3`)

Requirements: `arm-none-eabi-gcc` and `qemu-system-arm` on the `PATH`.

```sh
cd FreeRTOS/Test/Target/boards/qemu_cortex_m3/tests/performance_comparison/performance_comparison
make
make run   # Ctrl-C to stop; see the board's own README.md
```

`testGET_TIME_FUNCTION` is the DWT cycle counter (`ulGetDWTCycleCount`), so
reported times are in CPU cycles at `configCPU_CLOCK_HZ`.

### XMOS xcore.ai (`boards/xmos`)

Requirements: XMOS `xcc` toolchain (`xrun`) on the `PATH`.

```sh
cd FreeRTOS/Test/Target/boards/xmos/tests/performance_comparison/performance_comparison
make
make run
```

`testGET_TIME_FUNCTION` is `__xcore_get_reference_time`, a 100 MHz reference
clock, so reported times are in units of 10 ns.

## Running with `configUSE_QUEUE_SETS` enabled and disabled

`configUSE_QUEUE_SETS` must be built **once with each value** - the two
builds are not interchangeable, and neither one alone gives the full
picture:

- **`configUSE_QUEUE_SETS == 1`** (the current default for both boards):
  `xQueueSend`, `xQueueSendFromISR` and `xQueueGiveFromISR` each take an
  extra `pxQueue->pxQueueSetContainer != NULL` branch before reaching
  `listLIST_IS_EMPTY()` - paid on *every* call, even for a queue that isn't
  a set member, which is what every scenario here except
  `prvNotifyQueueSetContainer`'s uses. The `Test_PerformanceComparison_
  prvNotifyQueueSetContainer_*` scenarios only exist in this build, since
  they need `xQueueCreateSet()` / `xQueueAddToSet()` / `xQueueSelectFromSet()`.
- **`configUSE_QUEUE_SETS == 0`**: those three functions take the leaner
  direct-check branch instead, and the queue-set scenarios are compiled out
  entirely (guarded by `#if ( configUSE_QUEUE_SETS == 1 )` in
  `performance_comparison.c`).

Comparing the `xQueueSend` / `xQueueSendFromISR` / `xQueueGiveFromISR`
timings between the two runs isolates the cost of that extra set-membership
check by itself.

To switch: edit the board's `FreeRTOSConfig.h` (`configUSE_QUEUE_SETS` is on
line 30 for `boards/xmos`, line 85 for `boards/qemu_cortex_m3`), then
`make clean && make` before rerunning. Keep the two runs' output separate
(e.g. redirect to different log files) since they are not meant to be
averaged together.

## Sample counts and run time

Almost every scenario samples `testNUM_SAMPLES` (16384) times and settles
back to its precondition synchronously between samples (no real delay
elapses). The one exception is `xQueuePeek_WaitingReceiver`: unlike the
other "waiting" scenarios it cannot settle via a synchronous yield (see the
file header comment and `prvPeekSettlePostSampleHook` for why xQueuePeek()
needs two receiver tasks and an explicit `vTaskDelay(1)` between samples
instead). It uses the smaller `testNUM_SETTLE_SAMPLES` (512) so the run
finishes in a reasonable time; expect it to take roughly
`testNUM_SETTLE_SAMPLES / configTICK_RATE_HZ` seconds (about half a second
at the boards' default 1000 Hz tick rate).
