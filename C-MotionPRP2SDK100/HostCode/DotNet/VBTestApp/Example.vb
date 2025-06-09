Imports PMDLibrary


Public Class Examples
    Public Sub Example1()


        ' Allocate and open a peripheral connection to a Prodigy/CME card using TCP/IP.
        ' Note that the arguments for the PMDPeripheralTCP object are the same as for the
        ' C language call PMDPeriphOpenTCP, except that the first argument for the peripheral
        ' struct pointer and the second argument for the device are not used.
        ' The standard .NET class for IP addresses is used instead of a numeric IP address.
        ' DEFAULT_ETHERNET_PORT is a constant defined in PMDLibrary.vb for the default TCP port
        ' used for commands by the Prodigy/CME card.
        ' 1000 is a timeout value in milliseconds.
        Dim periph As New PMDPeripheralTCP(System.Net.IPAddress.Parse("192.168.2.2"),
                                           DEFAULT_ETHERNET_PORT,
                                           1000)

        ' Now allocate and connect a device object using the newly opened peripheral.
        ' Instead of using two different names the second argument specifies whether a
        ' Prodigy/CME or attached Magellan device is expected.
        Dim DevCME As New PMDDevice(periph, PMDDeviceType.ResourceProtocol)

        ' Once the Prodigy/CME device is open we can obtain an axis object, which may be used
        ' for any C-Motion commands.  Notice that the enumerated value used to specify the axis is
        ' called "Axis1" instead of "PMDAxis1" because the enumeration namespace need not be imported.
        Dim axis1 As New PMDAxis(DevCME, PMDAxisNumber.Axis1)

        ' Open an object representing the dual-ported RAM on the Prodigy/CME device.
        ' Currently only 32 bit memory devices are supported, but other devices may be supported
        ' in the future.
        Dim mem As New PMDMemory(DevCME, PMDDataSize.Size32Bit, PMDMemoryType.DPRAM)

        ' Start the user program on the C-Motion Engine.
        DevCME.TaskStart()

        Dim pos As Int32
        ' C-Motion procedures returning a single value become class properties, and may be
        ' retrieved or set by using an assignment.  The "Get" or "Set" part of the name is dropped.
        pos = axis1.ActualPosition

        ' The following line sets the actual position of the axis to zero.
        axis1.ActualPosition = 0

        ' Properties may accept parameters, for example the CurrentLoop parameter is used to set
        ' control gains for the current loops, and takes two parameters.  This example sets
        ' the proportional gain for phaseA to 1000
        axis1.SetCurrentLoop(PMDCurrentLoop.PhaseA, PMDCurrentLoopParameter.Kp, 1000)

        ' C-Motion procedures returning multiple values become Sub methods, and return their
        ' values using ByRef parameters.  The "Get" and "Set" parts of the names are the same as
        ' in the C language binding.
        Dim MPmajor, MPminor, NumberAxes, special, custom, family As UInt16
        Dim MotorType As PMDMotorTypeVersion
        axis1.GetVersion(family, MotorType, NumberAxes, special, custom, MPmajor, MPminor)

        ' C library procedures that accept pointers become methods operating on VB arrays.
        ' This example reads 100 values from dual-ported RAM. 
        Dim memvals(0 To 100) As UInt32
        mem.read(memvals, 100, memvals.Length())                

        ' If the objects opened here are not explicitly closed they will be closed by the
        ' garbage collector.
    End Sub

    Public Sub Example2()
        Dim periph As PMDPeripheral
        Dim periphPRP As PMDPeripheral
        Dim Dev As PMDDevice
        Dim PRPDev As PMDDevice
        Dim axis1 As PMDAxis
        Dim MagellanAttached As Boolean


        ' Open the connection on COM1, using default serial port parameters
        periphPRP = New PMDPeripheralSerial(1, 57600, PMDSerialParity.None, PMDSerialStopBits.SerialStopBits1)

        PRPDev = New PMDDevice(periphPRP, PMDDeviceType.ResourceProtocol)

        If (MagellanAttached) Then
            ' Connect to a Magellan via CAN NodeId 0 that is attached to the device we connected to above.
            periph = New PMDPeripheralCAN(PRPDev, &H600, &H580, 0)

            ' Obtain a device object using the peripheral.  Device type MotionProcessor means Magellan.
            Dev = New PMDDevice(periph, PMDDeviceType.MotionProcessor)
        Else
            Dev = PRPDev
        End If

        ' Finally get the axis object, specifying the desired axis number 1.
        axis1 = New PMDAxis(Dev, PMDAxisNumber.Axis1)

        ' Example VB-Motion operation: Get the event status
        Dim status As UInt16
        status = axis1.EventStatus
    End Sub

End Class





