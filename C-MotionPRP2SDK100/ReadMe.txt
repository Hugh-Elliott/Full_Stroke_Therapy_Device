============================================================

This readme file contains information pertaining to the
Performance Motion Devices C-Motion PRP II Software Development Kit.

The HostCode examples use Microsoft Visual Studio 
Microsoft Visual Studio can also be used for the CMECode examples.
Building the CMECode examples is accomplished with the build.bat file.

Most of the examples have the option to use several different
communication interfaces.  Each example contains the necessary interface 
connection functions appropriate for the particular product 
(ie. PMDPeriphOpenTCP).  It is necessary to select the desired one.

Many of the interface options may require more editing for
configuration, for example to select IP addresses, CANBus node
identifiers, serial communications ports, and so forth.  Comments in
the individual source files indicate which settings are likely to need
editing.

All streaming output functions such as printf are sent to the default console 
interface. The default console interface can be selected in Pro-Motion's 
Console Output window which is accessible from the Device Control window.

Preprocessor directives may be used to select specific modes of operation
such as: 

#define CME
Some of the provided examples can run on both the CME device and a Windows
PC, while others will only run on one or the other, but not both.  Whether or
not an example is being compiled to run on a CME device or Windows PC will 
affect what commands are supported.  Once again, a preprocessor directive is
used to assist with the compilation.  When "#define CME" is true, the example
will compile for use on a CME device using build.bat.  Otherwise if "CME" is 
not defined then the example will compile for use on a Windows PC (this would 
be the typical case when Visual Studio is in use).  Note that "CME" is a 
directive defined in the Makefile used to compile all CME code.


Some examples will have a mating example. For instance if 
HostCode\Examples\GenericUserPacket.c is to be run on a Windows PC,  
the corresponding CMECode\Examples\GenericUserPacket.c should
be running on the CME device.  


Example set:

CME Examples:
CMECode\Examples\Hello
CMECode\Examples\nIONCME
CMECode\Examples\GenericUserPacket
CMECode\Examples\Multitask

Host Examples (Windows PC):
HostCode\Examples\nIONCME
HostCode\Examples\GenericUserPacket
HostCode\Examples\MotionProcessor
HostCode\Examples\StoreUserCode


Hello.c
===============
This example runs on the CME device only.  It will print out a "Hello, world" message
to the Console window.  It will then query and print the ActualPosition of Axis1 of the 
on board Magellan controller. 


nIONCME.c
================
This example can be compiled to run on the device or from a PC host. Once the interface is 
established it will call several functions that demonstrate features of the device.

The SetupAxis1() function sets all the parameters of the processor based on the 
ProMotionExport.c file exported from Pro-Motion. (Select "File/Export as C-Motion").

The AllMagellanCommands() function send all supported commands through the interface.
This example is really only meant to be used as a command reference. 

The ProfileMove() will attempt to perform a trapezoidal profile move. 
The function will return when the profile completes or a timeout occurs.

The IOExample() function reads and writes the IO signals of the N-ION's IO connector.


GenericUserPacket.c
====================
This example contains two distinct pieces of source code.  One that runs on the
CME device (found in the CMECode/Examples/GenericUserPacket folder) and another that runs on
a Windows PC (found in HostCode/Examples/GenericUserPacket). 

On the CME device side this example will open a port and query that port in a loop to
check for received data.  If data is received it will send the data back out the peripheral.

On the Windows PC side this example will open a port and send a few bytes of data, it will
then wait to receive data.


MotionProcessor.c
====================
This example demonstrates communicating with a PMD Motion IC on any PMD product.


Multitask.c
====================
This example demonstrates multitasking features such as mailboxes and mutexes.

StoreUserCode.c
====================
This example demonstrates the downloading of CME user code (.bin) files to the N-ION via 
various interfaces.


