# How to add a new test?

1. Create a directory in the `FreeRTOS/Test/Target/tests` directory which will
   contain the test. For example: `FreeRTOS/Test/Target/tests/smp/multiple_tasks_running`.
1. Copy the `test_name.c` and `test_config.h` files from this template
   directory to the newly created directory above.
1. Rename the `test_name.c` according to the test name.
1. Implement the test in the above test file.
1. Add any FreeRTOS specific configuration required for the test to `test_config.h`.

# How to add a new target?

1. Create a target specific directory in the `FreeRTOS/Test/Target/boards` directory.
1. Create required build files.
    - Include `test_config.h` in `FreeRTOSConfig.h` at the end.
    - Ensure that the following configurations are not defined in `FreeRTOSConfig.h` as those are defined in `test_config.h`:
        - `configRUN_MULTIPLE_PRIORITIES`
        - `configUSE_CORE_AFFINITY`
        - `configUSE_TASK_PREEMPTION_DISABLE`
        - `configUSE_TIME_SLICING`
        - `configUSE_PREEMPTION`

# How to add a test to a target

1. Create a directory in the target's directory which will contain
   the test. For example: `FreeRTOS/Test/Target/boards/pico/tests/smp/multiple_tasks_running`.
2. ( optional ) If your platform uses a test framework other than Unity, set `configTARGET_TEST_USE_CUSTOM_SETTING`
   to 1 in FreeRTOSConfig.h and create a `test_setting_config.h` header file to
   adapt the test framework to your device's requirements.
3. Create a C file and invoke the test case from a task. The invocation
   usually looks like the following:
    ```c
    void prvTestRunnerTask( void * pvParameters )
    {
        /* Invoke the test case. */
        vRunTestCaseName();
    }
    ```
4. Add the file created above and the test case file to the build system used
   for the target.

The [example target](../../../boards/examples/tests/smp/template) provides a ready-to-compile
FreeRTOS project template, designed to help newcomers create their own target test projects.