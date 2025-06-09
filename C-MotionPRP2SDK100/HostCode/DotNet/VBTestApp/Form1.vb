Imports System.Text
Imports PMDLibrary


Public Class Form1
    Dim periph1 As PMDPeripheral
    Dim device1 As PMDDevice
    Dim axis1 As PMDAxis
    Dim memory1 As PMDMemory
    Dim periph2 As PMDPeripheral
    Dim device2 As PMDDevice
    Dim axis2 As PMDAxis
    Dim major, minor As UInt32
    Dim MPmajor, MPminor, NumberAxes, special, custom, family As UInt16
    Dim MotorType As PMDMotorTypeVersion
    Dim DeviceType As PMDDeviceType
    Const TCPTimeout = 1000

    Public Sub CloseAll()
        If (memory1 IsNot Nothing) Then
            memory1.Close()
        End If
        If (axis1 IsNot Nothing) Then
            axis1.Close()
        End If
        If (device1 IsNot Nothing) Then
            device1.Close()
        End If
        If (periph1 IsNot Nothing) Then
            periph1.Close()
        End If
        If (axis2 IsNot Nothing) Then
            axis2.Close()
        End If
        If (device2 IsNot Nothing) Then
            device2.Close()
        End If
        If (periph2 IsNot Nothing) Then
            periph2.Close()
        End If
    End Sub

    Public Sub New()

        DeviceType = PMDDeviceType.ResourceProtocol
        '        DeviceType = PMDDeviceType.MotionProcessor 'set PMDDeviceType to MotionProcessor if connecting to a Motion IC without a C-Motion Engine.

        ' This call is required by the Windows Form Designer.
        InitializeComponent()

        ' Add any initialization after the InitializeComponent() call.
    End Sub

    Private Sub ClearControls()
        LabelVersionChip.Text = ""
        LabelVersionCME.Text = ""
        LabelDefaultsCOM.Text = ""
        ListBox1.Items.Clear()
    End Sub

    Private Sub PRP2Functions()
        Dim value As UInt32
        Dim time As SYSTEMTIME
        Dim currentTime As Date
        Dim nodeID, DOsignal, DIsignal, DIsense As Byte
        Dim filename As String
        Dim filedate As String
        Dim data(0 To 260) As Byte


        value = device1.GetFaultCode(PMDFaultCode.Initialization)
        value = device1.GetFaultCode(PMDFaultCode.Exception)
        value = device1.GetFaultCode(PMDFaultCode.ResetCause)

        value = device1.GetInfo(PMDDeviceInfo.CMEVersion, 0)
        value = device1.GetInfo(PMDDeviceInfo.HostInterface, 0)
        value = device1.GetInfo(PMDDeviceInfo.LogicVersion, 0)
        value = device1.GetInfo(PMDDeviceInfo.MemorySize, PMDMemoryType.NVRAM)
        value = device1.GetInfo(PMDDeviceInfo.MemorySize, PMDMemoryType.RAM)

        value = device1.GetTaskInfo(0, PMDTaskInfo.State) ' task #0 is main task
        value = device1.GetTaskInfo(1, PMDTaskInfo.State) ' task #1 is first task created by the main task if any.
        value = device1.GetTaskInfo(0, PMDTaskInfo.AbortCode)
        value = device1.GetTaskInfo(0, PMDTaskInfo.StackSize)
        value = device1.GetTaskInfo(0, PMDTaskInfo.Priority)
        value = device1.GetTaskInfo(0, PMDTaskInfo.Name)
        'value = device1.GetTaskInfo(0, PMDTaskInfo.StackRemaining) 'Will error if task is not running

        device1.GetSystemTime(time)
        currentTime = Now
        time.wYear = currentTime.Year
        time.wMonth = currentTime.Month
        time.wDay = currentTime.DayOfWeek
        time.wHour = currentTime.Hour
        time.wMinute = currentTime.Minute
        time.wSecond = currentTime.Second
        time.wMilliseconds = currentTime.Millisecond
        device1.SetSystemTime(time)
        device1.GetSystemTime(time)

        nodeID = 1
        DOsignal = 7 ' DIO7/SynchOut
        DIsignal = 0 ' a value of 0 forces the nodeID setting rather than detecting a DI state. DIO4/SynchIn
        DIsense = 0  ' The input level that triggers the device to set its node id when it receives this command. SynchOut is high on power up.
        'device1.SetNodeID(nodeID, DOsignal, DIsignal, DIsense)         'should only be used on CAN or RS485 buses.

        value = device1.UserCodeVersion()
        value = device1.UserCodeChecksum()
        filename = device1.GetUserCodeName()
        filedate = device1.GetUserCodeDate()

    End Sub

    Private Sub ButtonCOM_click(ByVal Sender As System.Object, ByVal e As System.EventArgs) Handles ButtonCOM.Click
        Dim ra1(0 To 10) As UInt32
        Dim i As UInt32
        Dim IPAddr As UInt32

        ClearControls()
        'Dim periph As PMDPeripheralCOM
        Dim position As UInt32


        Try
            periph1 = New PMDPeripheralSerial(PMDSerialPort.COM4, 57600, PMDSerialParity.None, PMDSerialStopBits.SerialStopBits1)
            device1 = New PMDDevice(periph1, DeviceType)
            axis1 = New PMDAxis(device1, PMDAxisNumber.Axis1)
            ' periph.Sync()
            position = axis1.ActualPosition
            axis1.GetVersion(family, MotorType, NumberAxes, special, custom, MPmajor, MPminor)
            LabelVersionChip.Text = "Motion IC version: " & family & MotorType & NumberAxes & special & "." & custom.ToString("00") & "." & MPmajor & "." & MPminor
            If (DeviceType = PMDDeviceType.ResourceProtocol) Then
                device1.Version(major, minor)
                LabelVersionCME.Text = "CME version: " & major & "." & minor.ToString("00")
                IPAddr = device1.GetDefault(PMDDefault.IPAddress)
                LabelDefaultsCOM.Text = "IPAddr: " & (IPAddr >> 24).ToString() & "." &
                                                 ((IPAddr >> 16) And &HFF).ToString() & "." &
                                                 ((IPAddr >> 8) And &HFF).ToString() & "." &
                                                 (IPAddr And &HFF).ToString()
                PRP2Functions()

                memory1 = New PMDMemory(device1, PMDDataSize.Size32Bit, PMDMemoryType.RAM)
                ListBox1.Items.Clear()
                For i = 0 To ra1.Length() - 1
                    ra1(i) = i
                    ListBox1.Items.Insert(i, ra1(i))
                Next
                memory1.Write(ra1, 0, ra1.Length())
                memory1.Read(ra1, 100, ra1.Length())
                memory1.Close()


            End If

        Catch ex As Exception
            MsgBox(ex.Message)
        End Try
        CloseAll()


    End Sub

    Private Sub ButtonTCP_click(ByVal Sender As System.Object, ByVal e As System.EventArgs) Handles ButtonTCP.Click
        Dim ra1(0 To 10) As UInt32
        Dim major, minor As UInt32
        Dim MPmajor, MPminor, NumberAxes, special, custom, family As UInt16
        Dim MotorType As PMDMotorTypeVersion
        Dim i As UInt32
        ClearControls()

        Try
            periph1 = New PMDPeripheralTCP(System.Net.IPAddress.Parse("192.168.2.2"), DEFAULT_ETHERNET_PORT, 1000)
            device1 = New PMDDevice(periph1, PMDDeviceType.ResourceProtocol)
            axis1 = New PMDAxis(device1, PMDAxisNumber.Axis1)
            axis1.GetVersion(family, MotorType, NumberAxes, special, custom, MPmajor, MPminor)
            LabelVersionChip.Text = "Motion IC version: " & family & MotorType & NumberAxes & special & "." & custom.ToString("00") & "." & MPmajor & "." & MPminor
            device1.Version(major, minor)
            LabelVersionCME.Text = "CME version: " & major & "." & minor.ToString("00")
            memory1 = New PMDMemory(device1, PMDDataSize.Size32Bit, PMDMemoryType.RAM)
            memory1.Read(ra1, 100, ra1.Length())
            ListBox1.Items.Clear()
            For i = 0 To ra1.Length() - 1
                ListBox1.Items.Insert(i, ra1(i))
            Next

            PRP2Functions()

        Catch ex As Exception
            MsgBox(ex.Message)
        End Try
        CloseAll()
    End Sub

    ' This example sends a packet to a downloaded app running on the CME that is listening to port 40104
    Private Sub ButtonTCPSend_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles ButtonTCPSend.Click
        Dim pUser As PMDPeripheral
        ' pUser is used to exchange data with the UserTCP CME application.
        Try
            pUser = New PMDPeripheralTCP(System.Net.IPAddress.Parse("192.168.2.2"), 40104, 1000)
            Dim BytesOut As Byte()
            Dim BytesIn(0 To 100) As Byte
            Dim nReceived As UInteger
            BytesOut = Encoding.ASCII.GetBytes(TCPMessage.Text)
            pUser.Send(BytesOut, BytesOut.Length, TCPTimeout)
            pUser.Receive(BytesIn, nReceived, BytesIn.Length, TCPTimeout)
            TCPReply.Text = Encoding.ASCII.GetString(BytesIn, 0, nReceived)
            pUser.Close()
        Catch ex As Exception
            MsgBox(ex.Message)
        End Try
    End Sub

    Private Sub CAN_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles CAN.Click
        Dim MPmajor, MPminor, NumberAxes, special, custom, family As UInt16
        Dim MotorType As PMDMotorTypeVersion
        Dim value As UInt32

        ' NODE 2 = out: Hex602 in: Hex582 
        Dim transmitID1 As UInteger = &H600
        Dim recieveID1 As UInteger = &H580
        Dim eventID1 As UInteger = &H150
        ClearControls()

        Try
            periph1 = New PMDPeripheralCAN(transmitID1, recieveID1, eventID1)
            periph2 = New PMDPeripheralMultiDrop(periph1, 2) 'nodeID=2

            ' -----Device pointer--------
            device1 = New PMDDevice(periph1, DeviceType) 'Starts as Resource Protocol, ours needs to be MotionProcessor
            device2 = New PMDDevice(periph2, DeviceType) 'Starts as Resource Protocol, ours needs to be MotionProcessor

            ' -----Axis pointer----------
            axis1 = New PMDAxis(device1, PMDAxisNumber.Axis1)
            axis2 = New PMDAxis(device2, PMDAxisNumber.Axis1)

            axis1.GetVersion(family, MotorType, NumberAxes, special, custom, MPmajor, MPminor)
            LabelVersionChip.Text = "Motion IC version: " & family & MotorType & NumberAxes & special & "." & custom.ToString("00") & "." & MPmajor & "." & MPminor

            value = axis1.Time
            ListBox1.Items.Add(value)
            value = axis2.Time
            ListBox1.Items.Add(value)

        Catch ex As Exception
            MsgBox(ex.Message)
        End Try
        CloseAll()

    End Sub

    Public Enum MachineIO
        DIRead = &H200
        DOMask = &H210
        DOWrite = &H212
        DORead = &H214
        DODirMask = &H220
        DODir = &H222
        DODirRead = &H224
        AmpEnaMask = &H230
        AmpEna = &H232
        AmpEnaRead = &H234
        IntPending = &H240
        IntMask = &H250
        IntPosEdgeAs = &H260
        IntNegEdgeAs = &H270
        AOCh1 = &H300
        AOCh2 = &H302
        AOCh3 = &H304
        AOCh4 = &H306
        AOCh5 = &H308
        AOCh6 = &H30A
        AOCh7 = &H30C
        AOCh8 = &H30E
        AOCh1Ena = &H310
        AOCh2Ena = &H312
        AOCh3Ena = &H314
        AOCh4Ena = &H316
        AOCh5Ena = &H318
        AOCh6Ena = &H31A
        AOCh7Ena = &H31C
        AOCh8Ena = &H31E
        AOEna = &H320
        AICh1 = &H340
        AICh2 = &H342
        AICh3 = &H344
        AICh4 = &H346
        AICh5 = &H348
        AICh6 = &H34A
        AICh7 = &H34C
        AICh8 = &H34E
    End Enum

    Private Sub IO_Example(ByRef device As PMDDevice)
        ' set direction of DIO pins

        Dim perIO As PMDPeripheral
        Dim DIODir, dataout, datain As UInt16

        perIO = New PMD.PMDPeripheralPIO(device, 0, 0, PMD.PMDDataSize.Size16Bit)

        'Some of the Digital Input and Output are dedicated and the direction must be defined
        'on the one which are not dedicated.
        'Write a "1" to the bit to set the direction as OUT.
        'DIO1  Bit 8
        'DIO2  Bit 9
        'DIO3  Bit 10
        'DIO4  Bit 11
        'DIO5  Bit 12
        'DIO6  Bit 13
        'DIO7  Bit 14
        'DIO8  Bit 15

        perIO.Write(&HF00, MachineIO.DODir)   ' set all DIO8 to be outputs

        DIODir = perIO.Read(MachineIO.DODirRead)  ' read the IO direction

        datain = perIO.Read(MachineIO.DIRead)

        'Bits for PMDMachineIO_DO
        'DO1   Bit 0
        'DO2   Bit 1
        'DO3   Bit 2
        'DO4   Bit 3
        'DIO1  Bit 8
        'DIO2  Bit 9
        'DIO3  Bit 10
        'DIO4  Bit 11
        'DIO5  Bit 12
        'DIO6  Bit 13
        'DIO7  Bit 14
        'DIO8  Bit 15

        'For DO1 to DO4 a 1 bit means 0V output
        'For DIO1 to DIO8 a 1 bit means 5V output

        'set DO1,DO3, DIO2, and DIO7 to 5V 
        dataout = &H4206
        perIO.Write(dataout, MachineIO.DOWrite)

        perIO.Close()

    End Sub
End Class
