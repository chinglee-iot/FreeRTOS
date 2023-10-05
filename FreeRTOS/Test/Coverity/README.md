# Static code analysis for FreeRTOS-Kernel
This directory is made for the purpose of statically testing the MISRA C:2012 compliance of FreeRTOS-Kernel using
[Synopsys Coverity](https://www.synopsys.com/software-integrity/security-testing/static-analysis-sast.html) static analysis tool.
To that end, this directory provides a [CMake](https://github.com/chinglee-iot/FreeRTOS/blob/add-coverity-test-application/FreeRTOS/Test/Coverity/CMakeLists.txt)
file and [configuration files](https://github.com/chinglee-iot/FreeRTOS/blob/add-coverity-test-application/FreeRTOS/Test/Coverity/coverity_misra.config)
an application for the tool to analyze.

> **Note**
For generating the report as outlined below, we have used Coverity version 2018.09.

For details regarding the suppressed violations in the report (which can be generated using the instructions described below), please
see the [MISRA.md](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/7372519cba39922ce5aeb92b8931cf8694b78bd2/MISRA.md) file.

## Getting Started
### Prerequisites
You can run this on a platform supported by Coverity. The list and other details can be found 
[here](https://sig-docs.synopsys.com/polaris/topics/c_coverity-compatible-platforms.html).
To compile and run the Coverity target successfully, you must have the following:

1. CMake version > 3.13.0 (You can check whether you have this by typing `cmake --version`)
2. GCC compiler
    - You can see the downloading and installation instructions [here](https://gcc.gnu.org/install/).

### To build and run coverity:
Go to the directory `FreeRTOS/FreeRTOS/Test/Coverity` and run the following commands in terminal:
1. Update the compiler configuration in Coverity
  ~~~
  cov-configure --force --compiler cc --comptype gcc
  ~~~
2. Create the build files using CMake in a `build` directory
  ~~~
  cmake -B build -S .
  ~~~
3. Go to the build directory and copy the coverity configuration file
  ~~~
  cd build/
  cp ../coverity_misra.config .
  ~~~
4. Build the (pseudo) application
  ~~~
  cov-build --emit-complementary-info --dir cov-out make
  ~~~
5. Go to the Coverity output directory (`cov-out`) and begin Coverity static analysis
  ~~~
  cd cov-out/
  cov-analyze --dir . --coding-standard-config ../coverity_misra.config --tu-pattern "file('.*/source/.*')"
  ~~~
6. Format the errors in HTML format so that it is more readable
  ~~~
  cov-format-errors --dir . --html-output html-output
  ~~~

You should now have the HTML formatted violations list in a directory named `html-output`.
