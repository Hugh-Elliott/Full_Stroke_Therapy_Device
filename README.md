# Intro
# Hardware
* nIONCME (Microcontroller)
* Host PC (Laptop, desktop, etc)
* DB9 to USB-A cable
* 3-Pin programming cable
# Software Needed
* Python (Used IDLE 3.11+)
* Visual Studio (Used VS 2022)
* Pro-Motion (Install file: Pro-Motion600)
# File Structure
This GitHub repo is split into two sub-folders based on the PMD Corp's software development kit for the nIONCME microcontroller. Details about the development kit example code can be found in the PMDReadMe file.
## Host PC Code
Located in ~\C-MotionPRP2SDK100\HostCode\Examples

The Examples.sln solution compiles the four example programs designed to be executed from a host PC with commands transmitted via serial communication. The host PC solution is compatible with C/C++ languages. Executing this code can be done from Visual Studio.

During initial development, the nIONCME.c example was chosen as the basis for this project, and all additional files were created using this solution.
## CME Code
Located in ~\C-MotionPRP2SDK100\CMECode\Examples

The CMECodeExamples.sln solution compiles the four example programs designed to be executed from the nIONCME itself. It shares the same nIONCME.c file as the Host PC code. 

> [!CAUTION]
> **The nIONCME is only compatible with C language code**

It is not possible to execute the code using the Visual Studio solution. Any files to be included in the code **must** be added to the Makefile in ~\nIONCME before building in Visual Studio. The bin file then must be uploaded to the nIONCME microcontroller. 

In addition to the example code, two folders were added for all MATLAB code and the Python GUI script. Those being ~\MATLAB and ~\PythonGUI, respectively.
# Contributions
* Originally created and maintained by Hugh Elliott from September 2024 to August 2025
