# FreeRTOS SMP SMP on target test for XMOS XCORE.AI Explorer Board

> **FreeRTOS-SMP Kernel is still being tested.**

This page documents a SMP on target test that uses the FreeRTOS symmetric multiprocessing (SMP) version.
The test targets the [XCORE.AI](https://www.xmos.ai/xcore-ai/), which has 16
cores. The project uses [XMOS XTC Tools](https://www.xmos.ai/software-tools/) to
build the FreeRTOS XCOREAI port. This test shows that [XCORE.AI](https://www.xmos.ai/xcore-ai/)
supports [FreeRTOS Kernel Symmetric Multiprocessing (SMP)](https://github.com/FreeRTOS/FreeRTOS-Kernel/tree/smp).

---

## IMPORTANT! Notes on using the FreeRTOS XCORE.AI SMP on target test

Please read all the following points before using this RTOS port.

1. [Source Code Organization](#source-code-organization)
1. [The SMP On Target Test Application](#the-smp-on-target-test-application)
1. [Building and Running the FreeRTOS SMP On Target Test](#building-and-running-the-smp-on-target-test-application)
1. [FreeRTOS Configuration and Usage Details](#freertos-configuration-and-usage-details)
1. [Trouble Shooting](#trouble-shooting)

Also see the FAQ [My application does not run, what could be wrong](https://www.freertos.org/FAQHelp.html)?

---

## Source Code Organization

The project files for this test are located in the [FreeRTOS/Test/Target/boards/xmos](./)
directory of the [FreeRTOS repository](https://github.com/FreeRTOS/FreeRTOS).
FreeRTOS Port files compiled in the project are in the
[FreeRTOS/Source/portable/ThirdParty/xClang/XCOREAI](../../../../Source/portable/ThirdParty/xClang/XCOREAI) directory.

Test cases are listed in [FreeRTOS/Test/Target/tests/smp](../../tests/smp) directory.
And [XCORE.AI](https://www.xmos.ai/xcore-ai/) can pass all test cases with corresponding [test_runners](./tests/smp/).

Test cases includes:

-   disable_multiple_priorities
-   disable_preemption
-   interrupt_wait_critical
-   multiple_tasks_running
-   only_one_task_enter_critical
-   only_one_task_enter_suspendall
-   schedule_affinity
-   schedule_equal_priority
-   schedule_highest_priority
-   suspend_scheduler
-   task_delete

---

## The SMP On Target Test Application

The SMP On Target Test is used to verify kernel behavior with SMP(FreeRTOS symmetric multiprocessing)
enabled. Each test case verifies different scenarios on SMP. And the configurations for FreeRTOS are changed
based on test cases, which means it needs several images to test.
To simplify the framework and to debug easily, the test framework uses individual images for each test case.
Each test case has its own [test_runners](./tests/smp/). The SMP On Target Test uses only one tile to run the
test and keep the other tile in idle.

### The SMP On Target Test Flow

The test starts from [main() in main.xc](./main.xc), goes to [c_main in main.c](./main.c), then runs the test
runner corresponding to each test case. Take test case `Multiple Tasks Running` as example:

1. Start from [main() in main.xc](./main.xc).
1. Call c_main in [main.c](./main.c) for tile 0.
1. Call vRunTest() in [multiple_tasks_running_test_runner.c](./tests/smp/multiple_tasks_running/multiple_tasks_running_test_runner.c).
1. Create task to run prvTestRunnerTask() in [multiple_tasks_running_test_runner.c](./tests/smp/multiple_tasks_running/multiple_tasks_running_test_runner.c).
1. Execute test case vRunMultipleTasksRunningTest() in [multiple_tasks_running.c](../../tests/smp/multiple_tasks_running/multiple_tasks_running.c).

---

## Building and Running the SMP On Target Test Application

### Hardware setup

Plug the xTAG programmer into the evaluation board. Ensure both the xTAG and
evaluation board are connected to the computer via USB.

### Toolchain installation

The development tools require a Linux host or a Linux style environment.

1. Download the [XMOS XTC Tools](https://www.xmos.ai/software-tools/).
2. Uncompress the archive to your chosen installation directory. The example
   below will install to your home directory:

```sh
$ tar -xf archive.tgz -C ~
```

3. Configure the default set of environment variables:

```sh
$ cd ~/XMOS/XTC/15.1.0
$ source SetEnv
```

4. Check that your tools environment has been setup correctly:

```sh
$ xcc --help
```

5. Make the XTAG drivers accessible to all users. This step is only required
   to be done once on a given development machine.

```sh
$ cd ~/XMOS/XTC/15.1.0/scripts
$ sudo ./setup_xmos_devices.sh
```

6. Check that the XTAG devices are available and accessible:

```sh
$ cd ~/XMOS/XTC/15.1.0/scripts
$ ./check_xmos_devices.sh
Searching for xtag3 devices...
0 found
Searching for xtag4 devices...
1 found
Success: User <username> is able to access all xtag4 devices
```

7. Check that the device is available for debugging:

```sh
$ xrun -l
Available XMOS Devices
----------------------

  ID  Name            Adapter ID    Devices
  --  ----            ----------    -------
  0   XMOS XTAG-4     2W3T8RAG      P[0]
```

### Build and Run the application

1. Go to the test case directory:

```sh
$ cd FreeRTOS/Test/Target/boards/xmos/tests/smp/multiple_tasks_running
```

2. Build the test:

```sh
$ make
```

3. Run the test:

```sh
$ make run
```

3. Clean the binaries:

```sh
$ make clean
```

---

## FreeRTOS Configuration and Usage Details

-   Configuration items specific to this test are contained in
    [FreeRTOSConfig.h](./FreeRTOSConfig.h). The
    [constants defined in that file](https://www.freertos.org/a00110.html) can be
    edited to suit your application. The following configuration options are set in test_config.h
    for every test case, which is specific to the SMP support in the FreeRTOS Kernel:
    -   `configNUMBER_OF_CORES` - Set the number of cores.
    -   `configRUN_MULTIPLE_PRIORITIES` - Enable/Disable simultaneously running tasks with multiple priorities.
    -   `configUSE_CORE_AFFINITY` - Enable/Disable setting a task's affinity to certain cores.
    -   `configUSE_TASK_PREEMPTION_DISABLE` - Enable/Disable functions (vTaskPreemptionDisable/vTaskPreemptionEnable)
        to stop preempting MCU resource from specific task.
    -   `configUSE_PREEMPTION` - Enable/Disable preemption as general rule.
-   `Source/Portable/MemMang/heap_4.c` is included in the project to provide the
    memory allocation required by the RTOS kernel. Please refer to the
    [Memory Management](https://www.freertos.org/a00111.html) section of the API
    documentation for complete information.
-   vPortEndScheduler() has not been implemented.

---

## Trouble Shooting

[XMOS XTC Tools](https://www.xmos.ai/software-tools/) supports xgdb as on target debug tool.
We can debug with gdb commands by following steps:

1. Follow [Hardware Setup](#hardware-setup) and [Toolchain installation](#toolchain-installation) to set the environment.
1. Follow [Build and Run the application](#build-and-run-the-application) to build the image.
1. Call below command to start debugging with gdb.
    - `xgdb bin/$(test_case_name).xe -ex connect -ex "connect --xscope"`
1. Enjoy the convience provided by gdb.

---
