Imports System.Runtime.InteropServices


Public Module PMD

    '*** C library constant declarations ***

    Public Const PMD_WAITFOREVER = &HFFFFFFFF
    Public Const DEFAULT_ETHERNET_PORT = 40100

    '*** C library enum declarations ***

    Public Enum PMDDeviceType
        None = 0
        ResourceProtocol = 1
        MotionProcessor = 2
    End Enum

    Public Enum PMDSerialPort
        COM1 = 0
        COM2 = 1
        COM3 = 2
        COM4 = 3
        COM5 = 4
        COM6 = 5
        COM7 = 6
    End Enum

    Public Enum PMDInterfaceType
        None = 0
        Parallel = 1
        PCI = 2
        ISA = 3
        Serial = 4
        CAN = 5
        TCP = 6
        UDP = 7
        SPI = 8
        PRP = 9
    End Enum

    ' NB this enum is not present in the C include files
    Public Enum PMDInstructionError
        NoError = 0
        ProcessorReset = 1
        InvalidInstruction = 2
        InvalidAxis = 3
        InvalidParameter = 4
        TraceRunning = 5
        BlockOutOfBounds = 7
        TraceBufferZero = 8
        BadSerialChecksum = 9
        InvalidNegativeValue = 11
        InvalidParameterChange = 12
        InvalidMoveAfterEventTriggeredStop = 13
        InvalidMoveIntoLimit = 14
        InvalidOperatingModeRestoreAfterEventTriggeredChange = 16
        InvalidOperatingModeForCommand = 17
        BadState = 18
        HardFault = 19
        AtlasNotDetected = 20
        BadSPIChecksum = 21
        InvalidSPIprotocol = 22
        InvalidTorqueCommand = 24
        BadFlashChecksum = 25
        InvalidFlashModeCommand = 26
        ReadOnlyBuffer = 27
        InitializationOnlyCommand = 28
    End Enum

    Public Enum PMDDataSize
        Size8Bit = 1
        Size16Bit = 2
        Size32Bit = 4
    End Enum

    Public Enum PMDSPIPort
        ExpSPI = 0
        HostSPI = 1
    End Enum

    Public Enum PMDSPIBitRate
        SPIBitRate_20MHz = 1
        SPIBitRate_10MHz = 2
        SPIBitRate_5MHz = 3
        SPIBitRate_2500kHz = 4
        SPIBitRate_1250kHz = 5
        SPIBitRate_625kHz = 6
        SPIBitRate_312_5kHz = 7
    End Enum

    Public Enum PMDSPIChipSelect
        SPIChipSelect_None = 0
        SPIChipSelect_1 = 1
        SPIChipSelect_2 = 2
        SPIChipSelect_3 = 3
        SPIChipSelect_4 = 4
    End Enum

    Public Enum PMDCANPort
        CANPort1 = 0
        CANPort2 = 1
    End Enum

    Public Enum PMDMemoryType
        DPRAM = 0
        NVRAM = 1
        RAM = 2
    End Enum

    Public Enum PMDTaskState
        NoCode = 1
        NotStarted = 2
        Running = 3
        Aborted = 4
    End Enum

    Public Enum PMDTaskInfo
        State = 0
        AbortCode = 1
        StackRemaining = 2
        StackSize = 3
        Priority = 4
        Name = 5
    End Enum

    Public Enum PMDDeviceInfo
        CMEVersion = 0
        LogicVersion = 1
        HostInterface = 2
        MemorySize = 3
        Heap = 5
        IPAddress = 6
    End Enum

    Public Enum PMDFaultCode
        ResetCause = 0
        Initialization = 1
        Exception = 2
    End Enum

    Public Enum PMDDefault
        IPAddress = &H303  '   
        NetMask = &H304  '   
        Gateway = &H305  '
        IPPort = &H106  '
        MACH = &H307  '
        MACL = &H308  '
        TCPKeepAliveInterval = &H10A  'TCP connection keepalive interval in seconds (default Is 30)
        TCPKeepAliveIdleTime = &H10B  'TCP connection keepalive idle time in seconds (default Is 60)
        TCPKeepAliveCount = &H10C  'TCP connection keepalive counter for KEEPALIVE probes (default Is 2)
        COM1 = &H10E  'Serial port 1 (host PRP interface port) settings when in RS232 mode. (same encoding as the Magellan SetSerialPortMode command)
        COM2 = &H10F  'Serial port 2 (debug console Or expansion port) settings when Serial port 1 Is in RS232 mode.  
        RS485Mode = &H110  'Serial port 1 RS485 mode (see PMD_RS485Mode definitions below) Factory default is 0000
        CAN = &H111  'CAN port bitrate And node ID (same encoding as the Magellan SetCANMode command)
        HostCAN = &H112  'Host CAN port bitrate And node ID (same encoding as the Magellan SetCANMode command)
        AutoStartMode = &H114  'Auto start the user task(s) after a reset.
        TaskStackSize = &H116  'Default stack size in dwords
        DebugSource = &H117  '
        ConsoleIntfType = &H118  'Console interface type. One of PMDInterface.
        ConsoleIntfAddr = &H319  'Console interface address depending on type of PMDInterface.
        ConsoleIntfPort = &H11A  'Console interface port depending on type of PMDInterface.
        LEDFlashMode = &H11C  '
        TaskParam = &H31E  'Parameter passed to all "built-in" user tasks on auto-start
        RecoverTime = &H11F  'Time to wait in ms at startup for recover string
        DHCPTries = &H120  '
        COM3 = &H121  'TTL serial port on all 'N' series ION/CME products
        COM1RS485 = &H122  'Serial port 1 settings when in RS485 mode. 
        DIOmux = &H123  'Digital IO signal selection setting (PIO register DIO_SIG_SEL 0x228)
        DIOdir = &H124  'Digital IO signal direction setting (PIO register DIO_DIR_WRITE 0x222)
        DIOout = &H125  'Digital IO signal state setting (PIO register DIO_OUT_WRITE 0x212)
        HostSPI = &H126  '(CLKPolarity << 9) | (CLKPhase << 8) | DataSize
        ConsoleBuffer = &H128  'Console buffer size (default Is 1000 bytes)
        BiSSConfig = &H129  'AXIS1_BISS_CONFIG     register (PIO register 0x100)
        BiSSResolution = &H12A  'AXIS1_BISS_RESOLUTION register (PIO register 0x102)
        BiSSFrequency = &H12B  'AXIS1_BISS_FREQUENCY  register (PIO register 0x104)
        BiSSEnable = &H12C  'AXIS1_BISS_ENABLE     register (PIO register 0x106)
        BiSSSingleTurn = &H12D  'AXIS1_BISS_STCTRL     register (PIO register 0x108)
        BiSSMultiTurn = &H12E  'AXIS1_BISS_MTCTRL     register (PIO register 0x10A)
        BiSSRightBitShift = &H12F  'AXIS1_BISS_XBIT_CTRL  register (PIO register 0x10C)
        ConsoleTimeout = &H130  'PMDPrintf console output timeout (default Is 100 ms)
        WatchdogStartMode = &H132  'Auto start the user task after a watchdog reset. User code should call GetFaultCode to handle this reset condition.
        ExpSPIStatusDIO = &H133  'ExpSPI HostSPIStatus input DIO selection (default Is DIO7) 
        Factory = &HFFFF
    End Enum

    Public Enum PMDResult

        ' Motion processor errors
        NOERROR = 0
        ERR_OK = 0
        ERR_MP_Reset = &H1
        ERR_MP_InvalidInstruction = &H2
        ERR_MP_InvalidAxis = &H3
        ERR_MP_InvalidParameter = &H4
        ERR_MP_TraceRunning = &H5
        ERR_MP_BlockOutOfBounds = &H7
        ERR_MP_TraceBufferZero = &H8
        ERR_MP_BadSerialChecksum = &H9
        ERR_MP_InvalidNegativeValue = &HB
        ERR_MP_InvalidParameterChange = &HC
        ERR_MP_LimitEventPending = &HD
        ERR_MP_InvalidMoveIntoLimit = &HE
        ERR_MP_NVRAMMode = &HF
        ERR_MP_InvalidOperatingModeRestore = &H10
        ERR_MP_InvalidOperatingModeForCommand = &H11
        ERR_MP_BadState = &H12
        ERR_MP_HardFault = &H13
        ERR_MP_AtlasNotDetected = &H14
        ERR_MP_BadSPIChecksum = &H15
        ERR_MP_InvalidSPIprotocol = &H16
        ERR_MP_SPICommandTimingViolation = &H17
        ERR_MP_InvalidTorqueCommand = &H18
        ERR_MP_BadFlashChecksum = &H19
        ERR_MP_InvalidFlashModeCommand = &H1A
        ERR_MP_ReadOnly = &H1B
        ERR_MP_InitializationOnlyCommand = &H1C
        ERR_MP_IncorrectDataCount = &H1D
        ERR_MP_MoveWhileInError = &H1E
        ERR_MP_WaitTimedOut = &H1F
        ERR_MP_InitializationRunning = &H20
        ERR_MP_InvalidClock = &H21
        ERR_MP_InitializationSkipped = &H22
        ERR_MP_InvalidInterface = &H23

        ' General errors
        ERR_Version = &H1002
        ERR_WriteError = &H1005
        ERR_ReadError = &H1006
        ERR_Cancelled = &H1007
        ERR_CommunicationsError = &H1008
        ERR_InsufficientDataReceived = &H100A
        ERR_UnexpectedDataReceived = &H100B
        ERR_Memory = &H100C
        ERR_Timeout = &H100D
        ERR_Checksum = &H100E
        ERR_CommandError = &H100F
        ERR_MutexTimeout = &H1010

        ' Function call errors
        ERR_NotSupported = &H1101
        ERR_InvalidOperation = &H1102
        ERR_InvalidInterface = &H1103
        ERR_InvalidPort = &H1104
        ERR_InvalidBaud = &H1105
        ERR_InvalidHandle = &H1106
        ERR_InvalidDataSize = &H1107
        ERR_InvalidParameter = &H1108
        ERR_InvalidAddress = &H1109
        ERR_ParameterOutOfRange = &H110A
        ERR_ParameterAlignment = &H110B

        ' Interface connection errors
        ERR_NotConnected = &H1201
        ERR_NotResponding = &H1202
        ERR_PortRead = &H1203
        ERR_PortWrite = &H1204
        ERR_OpeningPort = &H1205
        ERR_ConfiguringPort = &H1206
        ERR_InterfaceNotInitialized = &H1207
        ERR_Driver = &H1208
        ERR_AddressInUse = &H1209
        ERR_IPRouting = &H120A
        ERR_OutOfResources = &H120B
        ERR_QueueFull = &H120C
        ERR_QueueEmpty = &H120D
        ERR_ResourceInUse = &H120E
        ERR_SerialOverrun = &H1211
        ERR_SerialBreak = &H1212
        ERR_SerialParity = &H1213
        ERR_SerialFrame = &H1214
        ERR_SerialNoise = &H1215
        ERR_ReceiveOverrun = &H1220
        ERR_CAN_BitStuff = &H1221
        ERR_CAN_Form = &H1222
        ERR_CAN_Acknowledge = &H1223
        ERR_CAN_BitRecessive = &H1224
        ERR_CAN_BitDominant = &H1225
        ERR_CAN_CRC = &H1226
        ERR_CAN_BusOff = &H1228
        ERR_CAN_Passive = &H1229
        ERR_CAN_Active = &H122A

        ' Resource Protocol errors
        ERR_RP_RemoteErrorMask = &H2000
        ERR_RP_Reset = &H2001
        ERR_RP_InvalidVersion = &H2002
        ERR_RP_InvalidResource = &H2003
        ERR_RP_InvalidAddress = &H2004
        ERR_RP_InvalidAction = &H2005
        ERR_RP_InvalidSubAction = &H2006
        ERR_RP_InvalidCommand = &H2007
        ERR_RP_InvalidParameter = &H2008
        ERR_RP_InvalidPacket = &H2009
        ERR_RP_PacketLength = &H200A
        ERR_RP_PacketAlignment = &H200B
        ERR_RP_Checksum = &H200E

        ' Prodigy/CME user code format errors
        ERR_UC_Signature = &H2101
        ERR_UC_Version = &H2102
        ERR_UC_FileSize = &H2103
        ERR_UC_Checksum = &H2104
        ERR_UC_WriteError = &H2105
        ERR_UC_NotProgrammed = &H2106
        ERR_UC_TaskNotCreated = &H2107
        ERR_UC_TaskAlreadyRunning = &H2108
        ERR_UC_TaskNotFound = &H2109
        ERR_UC_Format = &H210A
        ERR_UC_Reserved = &H210B
        ERR_UC_TaskAborted = &H210C

        ERR_NVRAM = &H2200 ' low byte Is an extended error code

        ERR_Unknown = &HFFFF

    End Enum

    ' Motion IC parameter enums

    Public Const AtlasAxisMask = &H20

    Public Enum PMDAxisNumber
        Axis1 = 0
        Axis2 = 1
        Axis3 = 2
        Axis4 = 3
        AtlasAxis1 = Axis1 + AtlasAxisMask
        AtlasAxis2 = Axis2 + AtlasAxisMask
        AtlasAxis3 = Axis3 + AtlasAxisMask
        AtlasAxis4 = Axis4 + AtlasAxisMask
    End Enum

    Public Enum PMDProductInfo
        Version = 1
        ProductClass = 2
        Checksum = 3
        PartNumberWord1 = 5
        PartNumberWord2 = 6
        PartNumberWord3 = 7
        PartNumberWord4 = 8
    End Enum

    Public Enum PMDFamily
        FirstGen = 1
        Navigator = 2
        Pilot = 3
        Magellan = 5
        MotorControl = 7
        MagellanION = 9
    End Enum

    Public Enum PMDMotorTypeVersion
        BrushedServo = 1
        BrushlessServo = 3
        MicroStepping = 4
        Stepping = 5
        AllMotor = 8
        AnyMotor = 9
    End Enum

    Public Enum PMDMotorType
        BrushlessDC3Phase = 0
        BrushlessDC2Phase = 1
        Microstep3Phase = 2
        Microstep2Phase = 3
        Stepping = 4
        DCBrush = 7
    End Enum

    Public Enum PMDProfileMode
        Trapezoidal = 0
        Velocity = 1
        SCurve = 2
        ElectronicGear = 3
    End Enum

    Public Enum PMDStopMode
        None = 0
        Abrupt = 1
        Smooth = 2
    End Enum

    Public Enum PMDMotionCompleteMode
        CommandedPosition = 0
        ActualPosition = 1
    End Enum

    Public Enum PMDActualPositionUnits
        Counts = 0
        Steps = 1
    End Enum

    Public Enum PMDAuxiliaryEncoderMode
        None = 0
        Quadrature = 1
        PulseAndDirection = 2
    End Enum

    Public Enum PMDGearMasterSource
        Actual = 0
        Commanded = 1
    End Enum

    Public Enum PMDSynchronizationMode
        Disabled = 0
        Master = 1
        Slave = 2
    End Enum

    Public Enum PMDBreakpoint
        Breakpoint1 = 0
        Breakpoint2 = 1
    End Enum

    Public Enum PMDBreakpointTrigger
        Disable = 0
        GreaterOrEqualCommandedPosition = 1
        LessOrEqualCommandedPosition = 2
        GreaterOrEqualActualPosition = 3
        LessOrEqualActualPosition = 4
        CommandedPositionCrossed = 5
        ActualPositionCrossed = 6
        Time = 7
        EventStatus = 8
        ActivityStatus = 9
        SignalStatus = 10
        DriveStatus = 11
    End Enum

    Public Enum PMDBreakpointAction
        None = 0
        Update = 1
        AbruptStop = 2
        SmoothStop = 3
        MotorOff = 4
        DisablePositionLoopAndHigherModules = 5
        DisableCurrentLoopAndHigherModules = 6
        DisableMotorOutputAndHigherModules = 7
        AbruptStopWithPositionErrorClear = 8
    End Enum

    Public Enum PMDActivityStatus
        PhasingInitialized = &H1
        AtMaximumVelocity = &H2
        Tracking = &H4
        AxisSettled = &H80
        MotorOn = &H100
        PositionCapture = &H200
        InMotion = &H400
        InPositiveLimit = &H800
        InNegativeLimit = &H1000
    End Enum

    Public Enum PMDEventStatus
        MotionComplete = &H1
        WrapAround = &H2
        Breakpoint1 = &H4
        CaptureReceived = &H8
        MotionError = &H10
        InPositiveLimit = &H20
        InNegativeLimit = &H40
        InstructionError = &H80
        DriveDisabled = &H100
        OvertemperatureFault = &H200
        BusVoltageFault = &H400
        DriveException = &H400
        CommutationError = &H800
        CurrentFoldback = &H1000
        RuntimeError = &H2000
        Breakpoint2 = &H4000
    End Enum

    Public Enum PMDSignalStatus
        EncoderA = &H1
        EncoderB = &H2
        EncoderIndex = &H4
        EncoderHome = &H8
        PositiveLimit = &H10
        NegativeLimit = &H20
        AxisIn = &H40
        HallA = &H80
        HallB = &H100
        HallC = &H200
        AxisOut = &H400
        StepOutputInvert = &H800
        StepInputInvert = &H800
        MotorDirection = &H1000
        EnableIn = &H2000
        FaultOut = &H4000
        StepDirectionInvert = &H8000  ' Juno
    End Enum

    Public Enum PMDActivityStatusBit
        PhasingInitialized = 0
        AtMaximumVelocity = 1
        Tracking = 2
        AxisSettled = 7
        MotorMode = 8
        PositionCapture = 9
        InMotion = 10
        InPositiveLimit = 11
        InNegitiveLimit = 12
    End Enum

    Public Enum PMDEventStatusBit
        MotionComplete = 0
        WrapAround = 1
        Breakpoint1 = 2
        CaptureReceived = 3
        MotionError = 4
        PositiveLimit = 5
        NegativeLimit = 6
        InstructionError = 7
        DriveDisabled = 8
        OvertemperatureFault = 9
        BusVoltageFault = 10
        DriveException = 10
        CommutationError = 11
        CurrentFoldback = 12
        RuntimeError = 13
        Breakpoint2 = 14
    End Enum

    Public Enum PMDSignalStatusBit
        EncoderA = 0
        EncoderB = 1
        EncoderIndex = 2
        EncoderHome = 3
        PositiveLimit = 4
        NegativeLimit = 5
        AxisIn = 6
        HallA = 7
        HallB = 8
        HallC = 9
        AxisOut = 10
        StepOutput = 11
        MotorDirection = 12
        EnableIn = 13
        FaultOut = 14
    End Enum

    Public Enum PMDCaptureSource
        Index = 0
        Home = 1
        HSI = 2
    End Enum

    Public Enum PMDEncoderSource
        Incremental = 0
        Parallel = 1
        None = 2
        Loopback = 3
        PulseAndDirection = 4
        Hall = 5
        Parallel32 = 6
        AnalogHall = 7
    End Enum

    Public Enum PMDOutputMode
        BipolarDAC = 0
        PWMSignMagnitude = 1
        PWM5050Magnitude = 2
        SPIDACOffsetBinary = 3
        UnipolarDAC = 4
        SPIDAC2sComplement = 5
        Atlas = 6
        PWMHighLow = 7
        PulseAndDirection = 8
        AtlasRecovery = 9
        None = 10
    End Enum

    Public Enum PMDCommutationMode
        Sinusoidal = 0
        HallBased = 1
    End Enum

    Public Enum PMDPhaseInitializeMode
        Algorithmic = 0
        HallBased = 1
        Pulse = 2
    End Enum

    Public Enum PMDPhaseCorrectionMode
        Disabled = 0
        Index = 1
        Hall = 2
    End Enum

    Public Enum PMDPhasePrescale
        Off = 0
        PhasePrescale64 = 1
        PhasePrescale128 = 2
        PhasePrescale256 = 3
    End Enum

    Public Enum PMDPhase
        A = 0
        B = 1
        C = 2
    End Enum

    Public Enum PMDTraceVariableNumber
        TraceVariableNumber1 = 0
        TraceVariableNumber2 = 1
        TraceVariableNumber3 = 2
        TraceVariableNumber4 = 3
    End Enum

    Public Enum PMDTraceMode
        OneTime = 0
        RollingBuffer = 1
        ExternalTrigger = &H100
    End Enum

    Public Enum PMDTraceVariable
        None = 0
        PositionError = 1
        CommandedPosition = 2
        CommandedVelocity = 3
        CommandedAcceleration = 4
        ActualPosition = 5
        ActualVelocity = 6
        ActiveMotorCommand = 7
        MotionProcessorTime = 8
        CaptureRegister = 9
        ActualVelocity32 = 83
        RawEncoderValue = 84
        PositionLoopIntegralSum = 10
        PositionLoopIntegralContribution = 57
        PositionLoopDerivative = 11
        PIDOutput = 64
        Biquad1Output = 65
        EventStatusRegister = 12
        ActivityStatusRegister = 13
        SignalStatusRegister = 14
        DriveStatusRegister = 56
        DriveFaultStatusRegister = 79
        AtlasSPIStatus = 80
        PhaseAngle = 15
        PhaseOffset = 16
        PhaseACommand = 17
        PhaseBCommand = 18
        PhaseCCommand = 19
        AnalogInput0 = 20
        AnalogInput1 = 21
        AnalogInput2 = 22
        AnalogInput3 = 23
        AnalogInput4 = 24
        AnalogInput5 = 25
        AnalogInput6 = 26
        AnalogInput7 = 27
        PhaseAngleScaled = 29
        CurrentLoopAReference = 66
        CurrentLoopAError = 30
        CurrentLoopActualCurrentA = 31
        CurrentLoopAIntegratorSum = 32
        CurrentLoopAIntegralContribution = 33
        CurrentLoopAOutput = 34
        CurrentLoopBReference = 67
        CurrentLoopBError = 35
        CurrentLoopActualCurrentB = 36
        CurrentLoopBIntegratorSum = 37
        CurrentLoopBIntegralContribution = 38
        CurrentLoopBOutput = 39
        FOCDReference = 40
        FOCDError = 41
        FOCDFeedback = 42
        FOCDIntegratorSum = 43
        FOCDIntegralContribution = 44
        FOCDOutput = 45
        FOCQReference = 46
        FOCQError = 47
        FOCQFeedback = 48
        FOCQIntegratorSum = 49
        FOCQIntegralContribution = 50
        FOCQOutput = 51
        FOCAlphaOutput = 52
        FOCBetaOutput = 53
        FOCActualCurrentA = 31
        FOCActualCurrentB = 36
        BusVoltage = 54
        Temperature = 55
        I2tEnergy = 68
        LegCurrentA = 69
        LegCurrentB = 70
        LegCurrentC = 71
        LegCurrentD = 72
        PhaseCurrentAlpha = 73
        PhaseCurrentBeta = 74
        PWMOutputA = 75
        PWMOutputB = 76
        PWMOutputC = 77
        EstimatedVelocity = 95
        CommandedVelocityScaled = 96
        VelocityLoopError = 97
        VelocityLoopErrorIntegral = 98
        VelocityLoopOutput = 99
        VelocityLoopBiQuad1Input = 100
        VelocityLoopBiQuad2Input = 101
        TachometerReading = 102
        AnalogCommandReading = 103
        IntegratedPositionLoopOutput = 104
        RawSPICommand = 105
        OuterLoopReference = 117
        OuterLoopFeedback = 118
    End Enum

    Public Enum PMDTraceCondition
        Immediate = 0
        NextUpdate = 1
        EventStatus = 2
        ActivityStatus = 3
        SignalStatus = 4
        DriveStatus = 5
        SPICommand = 6
    End Enum

    Public Enum PMDTraceTriggerState
        Low = 0
        High = 1
    End Enum

    Public Enum PMDTraceStatus
        Mode = &H1
        Activity = &H2
        DataWrap = &H4
        Overrun = &H8
        DataAvailable = &H10
    End Enum

    Public Enum PMDAxisOutRegister
        None = 0
        EventStatus = 1
        ActivityStatus = 2
        SignalStatus = 3
        DriveStatus = 4
    End Enum

    Public Enum PMDSerialBaud
        SerialBaud1200 = 0
        SerialBaud2400 = 1
        SerialBaud9600 = 2
        SerialBaud19200 = 3
        SerialBaud57600 = 4
        SerialBaud115200 = 5
        SerialBaud230400 = 6
        SerialBaud460800 = 7
    End Enum

    Public Enum PMDSerialStopBits
        SerialStopBits1 = 0
        SerialStopBits2 = 1
    End Enum

    Public Enum PMDSerialProtocol
        Point2Point = 0
        MultiDropUsingIdleLineDetection = 1
    End Enum

    Public Enum PMDSerialParity
        None = 0
        Odd = 1
        Even = 2
    End Enum

    Public Enum PMDCANBaud
        CANBaud1000000 = 0
        CANBaud800000 = 1
        CANBaud500000 = 2
        CANBaud250000 = 3
        CANBaud125000 = 4
        CANBaud50000 = 5
        CANBaud20000 = 6
        'CAN FD Is supported by N-Series ION only
        CANFDBaud1000_5000 = &H40
        CANFDBaud1000_4000 = &H50
        CANFDBaud1000_2000 = &H60
        CANFDBaud1000_1000 = &H70
        CANFDBaud500_1000 = &H72
    End Enum

    Public Enum PMDSPIMode
        RisingEdge = 0
        RisingEdgeDelay = 1
        FallingEdge = 2
        FallingEdgeDelay = 3
    End Enum

    Public Enum PMDCurrentControlMode
        CurrentLoop = 0
        FOC = 1
        ThirdLegFloating = 2
    End Enum

    Public Enum PMDFOC
        Direct = 0
        Quadrature = 1
        Both = 2
    End Enum

    Public Enum PMDFOCParameter
        Kp = 0
        Ki = 1
        Ilimit = 2
    End Enum

    Public Enum PMDCurrentLoop
        PhaseA = 0
        PhaseB = 1
        Both = 2
    End Enum

    Public Enum PMDCurrentLoopParameter
        Kp = 0
        Ki = 1
        Ilimit = 2
    End Enum

    Public Enum PMDCurrentLoopValueNode
        Reference = 0
        ActualCurrent = 1
        LoopError = 2
        IntegratorSum = 3
        IntegralContribution = 5
        Output = 6
        I2tEnergy = 10
    End Enum

    Public Enum PMDFOCValueNode
        Reference = 0
        Feedback = 1
        LoopError = 2
        IntegratorSum = 3
        IntegralContribution = 5
        Output = 6
        FOCOutput = 7
        ActualCurrent = 8
        I2tEnergy = 10
    End Enum

    Public Enum PMDDriveStatus
        Calibrated = 1 << 0
        InFoldback = 1 << 1
        OverTemperature = 1 << 2
        ShuntActive = 1 << 3
        InHolding = 1 << 4
        OverVoltage = 1 << 5
        UnderVoltage = 1 << 6
        Disabled = 1 << 7
        OutputClipped = 1 << 12
        NVRAMInitialization = 1 << 14
        AtlasNotConnected = 1 << 15
    End Enum

    Public Enum PMDDriveFaultStatus
        Overcurrent = 1 << 0
        Ground = 1 << 1
        ExternalLogic = 1 << 2
        OpmodeMismatch = 1 << 3
        InternalLogic = 1 << 4
        Overvoltage = 1 << 5
        Undervoltage = 1 << 6
        Disabled = 1 << 7
        Foldback = 1 << 8
        Overtemperature = 1 << 9
        AtlasDetectedSPIChecksum = 1 << 10
        Watchdog = 1 << 11
        PWMDisabled = 1 << 11
        Brake = 1 << 13
        MagellanDetectedSPIChecksum = 1 << 14
        MotorTypeMismatch = 1 << 15  ' Atlas
    End Enum

    Public Enum PMDCurrent
        HoldingMotorLimit = 0
        HoldingDelay = 1
        DriveCurrent = 2
    End Enum

    Public Enum PMDHoldingCurrent
        PMDHoldingMotorLimit = 0
        PMDHoldingDelay = 1
    End Enum

    Public Enum PMDCurrentFoldback
        ContinuousCurrentLimit = 0
        I2tEnergyLimit = 1
    End Enum

    Public Enum PMDOperatingMode
        AxisEnabled = &H1
        MotorOutputEnabled = &H2
        CurrentControlEnabled = &H4
        VelocityLoopEnabled = &H8
        PositionLoopEnabled = &H10
        TrajectoryEnabled = &H20
        CommandSourceEnabled = &H20
        AllEnabled = &H37
    End Enum

    Public Enum PMDEventActionEvent
        Immediate = 0
        PositiveLimit = 1
        NegativeLimit = 2
        MotionError = 3
        CurrentFoldback = 4
        CaptureReceived = 5
        Overtemperature = 6
        Disabled = 7
        CommutationError = 8
        Overcurrent = 9
        Overvoltage = 10
        Undervoltage = 11
        WatchdogTimeout = 12
        BrakeAsserted = 13
    End Enum

    Public Enum PMDEventAction
        None = 0
        AbruptStop = 2
        SmoothStop = 3
        DisableVelocityLoopAndHigherModules = 4
        DisablePositionLoopAndHigherModules = 5
        DisableCurrentLoopAndHigherModules = 6
        DisableMotorOutputAndHigherModules = 7
        AbruptStopWithPositionErrorClear = 8
        PassiveBraking = 10 ' Juno
    End Enum

    Public Enum PMDUpdateMask
        Trajectory = &H1
        PositionLoop = &H2
        CurrentLoop = &H8
    End Enum

    Public Enum PMDPositionLoopValueNode
        IntegratorSum = 0
        IntegralContribution = 1
        Derivative = 2
        Biquad1Input = 3
        Biquad2Input = 4
    End Enum

    Public Enum PMDPositionLoop
        Kp = 0
        Ki = 1
        Ilimit = 2
        Kd = 3
        DerivativeTime = 4
        Kout = 5
        Kvff = 6
        Kaff = 7
        Biquad1Enable = 8
        Biquad1B0 = 9
        Biquad1B1 = 10
        Biquad1B2 = 11
        Biquad1A1 = 12
        Biquad1A2 = 13
        Biquad1K = 14
        Biquad2Enable = 15
        Biquad2B0 = 16
        Biquad2B1 = 17
        Biquad2B2 = 18
        Biquad2A1 = 19
        Biquad2A2 = 20
        Biquad2K = 21
    End Enum

    Public Enum PMDVoltageLimit
        Overvoltage
        Undervoltage
    End Enum

    Public Enum PMDNVRAMOption
        ModeAtlas = 0
        Mode58113 = &H100
        EraseNVRAM = 1
        Write = 2
        BlockWriteBegin = 3
        BlockWriteEnd = 4
        Skip = 8
    End Enum

    Public Enum PMDDriveFaultParameter
        OvervoltageLimit = 0
        UndervoltageLimit = 1
        RecoveryMode = 2
        WatchdogLimit = 3
        TemperatureLimit = 4
        TemperatureHysteresis = 5
        ShuntVoltage = 8
        ShuntPWMDutyCycle = 9
        BusCurrentSupplyLimit = 10
        BusCurrentReturnLimit = 11
    End Enum

    Public Enum PMDDrivePWM
        Limit = 0
        DeadTime = 1
        SignalSense = 2
        Frequency = 3
        RefreshPeriod = 4
        RefreshTime = 5
        CurrentSenseTime = 6
    End Enum

    Public Enum PMDAnalogCalibration
        LegCurrentA = 0
        LegCurrentB = 1
        LegCurrentC = 2
        LegCurrentD = 3
    End Enum

    Public Enum PMDFeedbackParameter
        Modulus = 0
    End Enum

    Public Enum PMDDriveValue
        BusVoltage = 0
        Temperature = 1
        BusCurrentSupply = 2
        BusCurrentReturn = 3
    End Enum

    Public Enum PMDDriveCommandMode
        BLDC_SPI = &H0
        Step_SPI = &H1
        DC_SPI = &H2
        Step_PulseAndDirection = &H101
        Analog = &H20
        SPI = &H21
        InternalProfile = &H22
        PulseAndDirection = &H23
    End Enum

    Public Enum PMDCommutationParameter
        PhaseCounts = 0
        PhaseAngle = 1
        PhaseOffset = 2
        PhaseDenominator = 3
    End Enum

    Public Enum PMDLoop
        VelocityKp = 0
        VelocityKi = 1
        VelocityIlimit = 2
        VelocityKout = 5
        VelocityVelocityErrorLimit = 7
        VelocityBiquad1Enable = 8
        VelocityBiquad1B0 = 9
        VelocityBiquad1B1 = 10
        VelocityBiquad1B2 = 11
        VelocityBiquad1A1 = 12
        VelocityBiquad1A2 = 13
        VelocityBiquad2Enable = 16
        VelocityBiquad2B0 = 17
        VelocityBiquad2B1 = 18
        VelocityBiquad2B2 = 19
        VelocityBiquad2A1 = 20
        VelocityBiquad2A2 = 21
        VelocityFeedbackSource = &H40
        VelocityScalar = &H41
        PositionFeedbackSource = &H42
        LowerLimit = &H43
        UpperLimit = &H44
        PositionKp = &H100
        PositionKi = &H101
        PositionIlimit = &H102
        PositionKd = &H103
        PositionDerivativeTime = &H104
        PositionKout = &H105
        PositionPeriodMultiplier = &H106
        PositionPositionErrorLimit = &H107
    End Enum

    Public Enum PMDLoopValue
        VelocityReference = 0
        VelocityFeedback = 1
        VelocityError = 2
        VelocityIntegratorSum = 3
        VelocityOutput = 5
        VelocityBiquad1Input = 6
        VelocityBiquad2Input = 7
        PositionReference = &H100
        PositionFeedback = &H101
        PositionError = &H102
        PositionIntegratorSum = &H103
        PositionDerivative = &H104
        PositionOutput = &H105
        PositionBiquad1Input = &H106
        PositionBiquad2Input = &H107
    End Enum

    Public Enum PMDCalibrate
        LegCurrentSensors = 0
        AnalogCommand = 1
        Tachometer = 2
    End Enum

    Public Enum PMDPhaseParameter
        RampTime = 0
        PositivePulseTime = 1
        NegativePulseTime = 2
        PulseMotorCommand = 3
        RampCommand = 5
    End Enum

    Public Enum PMDProfileParameter
        WaveAmplitude = &H20
        WavePhase = &H21
        WaveStartingFrequency = &H22
        WaveMaximumFrequency = &H23
        WaveAngularAcceleration = &H24
        Source = 3
        IBufferID = 6
        PBufferID = 0
        RateScalar = 7
        StartIndex = 8
        StartValue = 9
        StopValue = 10
    End Enum

    Public Enum PMDRuntimeError
        VelocityScalarOverflow = 5
    End Enum

    Public Structure SYSTEMTIME
        Public wYear As UInt16
        Public wMonth As UInt16
        Public wDayOfWeek As UInt16
        Public wDay As UInt16
        Public wHour As UInt16
        Public wMinute As UInt16
        Public wSecond As UInt16
        Public wMilliseconds As UInt16
    End Structure


    '*** C library structure declarations

    ' Maintain two versions of this struct so our interface can be CLS compliant.
    <StructLayout(LayoutKind.Sequential)> Friend Structure PMDEvent_internal
        <MarshalAs(UnmanagedType.U2)> Public axis As PMDAxisNumber
        Public EventMask As UInt16
    End Structure

    Public Structure PMDEvent
        Public axis As PMDAxisNumber
        Public EventMask As UInt16
    End Structure

    '*** DLL entry point declarations ***

    Friend Declare Function PMDAxisAlloc Lib "C-Motion.dll" () As IntPtr
    Friend Declare Function PMDAxisClose Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr) As PMDResult
    Friend Declare Sub PMDAxisFree Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr)
    Friend Declare Function PMDAxisOpen Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal hDev As IntPtr,
                            ByVal axis_number As UInt16) As PMDResult
    Friend Declare Function PMDAtlasAxisOpen Lib "C-Motion.dll" (
                            ByVal hSourceAxis As IntPtr,
                            ByVal hAtlasAxis As IntPtr) As PMDResult
    Friend Declare Function PMDCMETaskGetState Lib "C-Motion.dll" (
                            ByVal hDev As IntPtr,
                            ByRef state As PMDTaskState) As PMDResult
    Friend Declare Function PMDCMETaskGetInfo Lib "C-Motion.dll" (
                            ByVal hDev As IntPtr,
                            ByVal taskno As Byte,
                            ByVal infoid As PMDTaskInfo,
                            ByRef value As UInt32) As PMDResult
    Friend Declare Function PMDCMETaskStart Lib "C-Motion.dll" (
                            ByVal hDev As IntPtr, ByVal param As UInt32) As PMDResult
    Friend Declare Function PMDCMETaskStop Lib "C-Motion.dll" (
                            ByVal hDev As IntPtr) As PMDResult

    Friend Declare Function PMDDeviceAlloc Lib "C-Motion.dll" () As IntPtr
    Friend Declare Function PMDDeviceClose Lib "C-Motion.dll" (
                            ByVal hDev As IntPtr) As PMDResult
    Friend Declare Sub PMDDeviceFree Lib "C-Motion.dll" (
                            ByVal hDev As IntPtr)

    ' The C function uses a pointer to void for value
    Friend Declare Function PMDDeviceGetDefaultUInt32 Lib "C-Motion.dll" Alias "PMDDeviceGetDefault" (
                            ByVal hDev As IntPtr,
                            ByVal code As PMDDefault,
                            ByRef value As UInt32,
                            ByVal valuesize As UInt32) As PMDResult
    Friend Declare Function PMDDeviceGetSystemTime Lib "C-Motion.dll" (
                            ByVal hDev As IntPtr,
                            ByRef time As SYSTEMTIME) As PMDResult
    Friend Declare Function PMDDeviceSetSystemTime Lib "C-Motion.dll" (
                            ByVal hDev As IntPtr,
                            ByRef time As SYSTEMTIME) As PMDResult
    Friend Declare Function PMDDeviceSetNodeID Lib "C-Motion.dll" (
                            ByVal hDev As IntPtr,
                            ByVal nodeID As Byte,
                            ByVal DOsignal As Byte,
                            ByVal DIsignal As Byte,
                            ByVal DIsense As Byte) As PMDResult


    Friend Declare Function PMDCMEStoreUserCode Lib "C-Motion.dll" (
                            ByVal hDev As IntPtr,
                            ByRef data As Byte,
                            ByVal length As UInt32) As PMDResult
    Friend Declare Function PMDCMEGetUserCodeName Lib "C-Motion.dll" (
                            ByVal hDev As IntPtr,
                            ByRef name As Byte) As PMDResult
    Friend Declare Function PMDCMEGetUserCodeDate Lib "C-Motion.dll" (
                            ByVal hDev As IntPtr,
                            ByRef filedate As Byte) As PMDResult
    Friend Declare Function PMDCMEGetUserCodeChecksum Lib "C-Motion.dll" (
                            ByVal hDev As IntPtr,
                            ByRef checksum As UInt32) As PMDResult
    Friend Declare Function PMDCMEGetUserCodeVersion Lib "C-Motion.dll" (
                            ByVal hDev As IntPtr,
                            ByRef version As UInt32) As PMDResult
    Friend Declare Function PMDDeviceGetVersion Lib "C-Motion.dll" (
                            ByVal hDev As IntPtr,
                            ByRef major As UInt32,
                            ByRef minor As UInt32) As PMDResult
    Friend Declare Function PMDDeviceGetInfo Lib "C-Motion.dll" (
                            ByVal hDev As IntPtr,
                            ByVal infoid As Byte,
                            ByVal subid As UInt16,
                            ByRef value As UInt32) As PMDResult
    Friend Declare Function PMDDeviceGetFaultCode Lib "C-Motion.dll" (
                            ByVal hDev As IntPtr,
                            ByVal faultid As UInt16,
                            ByRef value As UInt32) As PMDResult
    Friend Declare Function PMDDeviceNoOperation Lib "C-Motion.dll" (
                            ByVal hDev As IntPtr) As PMDResult

    Friend Declare Function PMDDeviceOpenPeriphCAN Lib "C-Motion.dll" (
                            ByVal hPeriph As IntPtr,
                            ByVal hDev As IntPtr,
                            ByVal transmitid As UInt32,
                            ByVal receiveid As UInt32,
                            ByVal eventid As UInt32) As PMDResult
    Friend Declare Function PMDDeviceOpenPeriphCANFD Lib "C-Motion.dll" (
                            ByVal hPeriph As IntPtr,
                            ByVal hDev As IntPtr,
                            ByVal port As PMDCANPort,
                            ByVal baud As UInt32,
                            ByVal transmitid As UInt32,
                            ByVal receiveid As UInt32,
                            ByVal eventid As UInt32) As PMDResult
    Friend Declare Function PMDDeviceOpenPeriphCME Lib "C-Motion.dll" (
                            ByVal hPeriph As IntPtr,
                            ByVal hDev As IntPtr) As PMDResult
    Friend Declare Function PMDDeviceOpenPeriphPRP Lib "C-Motion.dll" (
                            ByVal hPeriph As IntPtr,
                            ByVal hDev As IntPtr,
                            ByVal channel As UInt32,
                            ByVal bufsize As UInt32) As PMDResult
    Friend Declare Function PMDDeviceOpenPeriphSerial Lib "C-Motion.dll" (
                            ByVal hPeriph As IntPtr,
                            ByVal hDev As IntPtr,
                            ByVal portnum As UInt32,
                            ByVal baud As UInt32,
                            ByVal parity As PMDSerialParity,
                            ByVal StopBits As PMDSerialStopBits) As PMDResult
    Friend Declare Function PMDPeriphOpenPeriphMultiDrop Lib "C-Motion.dll" (
                            ByVal hPeriph As IntPtr,
                            ByVal hPeriph As IntPtr,
                            ByVal ARG3 As UInt32) As PMDResult
    Friend Declare Function PMDDeviceOpenPeriphTCP Lib "C-Motion.dll" (
                            ByVal hPeriph As IntPtr,
                            ByVal hDev As IntPtr,
                            ByVal IPAddress As UInt32,
                            ByVal portnum As UInt32,
                            ByVal timeout As UInt32) As PMDResult
    Friend Declare Function PMDDeviceOpenPeriphPIO Lib "C-Motion.dll" (
                            ByVal hPeriph As IntPtr,
                            ByVal hDev As IntPtr,
                            ByVal address As UInt16,
                            ByVal eventIRQ As Byte,
                            ByVal datasize As PMDDataSize) As PMDResult

    Friend Declare Function PMDDeviceReset Lib "C-Motion.dll" (
                            ByVal hDev As IntPtr) As PMDResult
    Friend Declare Function PMDDeviceSetDefaultUInt32 Lib "C-Motion.dll" Alias "PMDDeviceSetDefault" (
                            ByVal hDev As IntPtr,
                            ByVal code As PMDDefault,
                            ByRef value As UInt32,
                            ByVal valuesize As UInt32) As PMDResult

    Friend Declare Function PMDMemoryAlloc Lib "C-Motion.dll" () As IntPtr
    Friend Declare Function PMDMemoryClose Lib "C-Motion.dll" (
                            ByVal hMem As IntPtr) As PMDResult
    Friend Declare Sub PMDMemoryFree Lib "C-Motion.dll" (
                            ByVal hMem As IntPtr)
    Friend Declare Function PMDDeviceOpenMemory Lib "C-Motion.dll" (
                            ByVal hMem As IntPtr,
                            ByVal hDev As IntPtr,
                            ByVal DataSize As PMDDataSize,
                            ByVal MemType As PMDMemoryType) As PMDResult
    ' Note that the ByRef data argument is the first element of an array.
    Friend Declare Function PMDMemoryRead Lib "C-Motion.dll" (
                            ByVal hMem As IntPtr,
                            ByRef data As UInt32,
                            ByVal offset As UInt32,
                            ByVal length As UInt32) As PMDResult
    'Note that the ByRef data argument is the first element of an array
    Friend Declare Function PMDMemoryWrite Lib "C-Motion.dll" (
                            ByVal hMem As IntPtr,
                            ByRef data As UInt32,
                            ByVal offset As UInt32,
                            ByVal length As UInt32) As PMDResult


    Friend Declare Function PMDPeriphAlloc Lib "C-Motion.dll" () As IntPtr
    Friend Declare Function PMDPeriphClose Lib "C-Motion.dll" (
                            ByVal hPeriph As IntPtr) As PMDResult
    Friend Declare Function PMDPeriphFlush Lib "C-Motion.dll" (
                            ByVal hPeriph As IntPtr) As PMDResult
    Friend Declare Function PMDSerial_Sync Lib "C-Motion.dll" (
                            ByVal hPeriph As IntPtr) As PMDResult
    Friend Declare Sub PMDPeriphFree Lib "C-Motion.dll" (
                            ByVal hPeriph As IntPtr)

    ' These functions are for reading and writing 8 or 16 bit data over a parallel bus peripheral.
    ' The width of the data has to be correct for the peripheral type.
    ' The data argument should be the first member of an array of Byte or UInt16.
    Friend Declare Function PMDPeriphReadUInt8 Lib "C-Motion.dll" Alias "PMDPeriphRead" (
                            ByVal hPeriph As IntPtr,
                            ByRef data As Byte,
                            ByVal offset As UInt32,
                            ByVal length As UInt32) As PMDResult
    Friend Declare Function PMDPeriphReadUInt16 Lib "C-Motion.dll" Alias "PMDPeriphRead" (
                            ByVal hPeriph As IntPtr,
                            ByRef data As UInt16,
                            ByVal offset As UInt32,
                            ByVal length As UInt32) As PMDResult
    Friend Declare Function PMDPeriphWriteUInt8 Lib "C-Motion.dll" Alias "PMDPeriphWrite" (
                            ByVal hPeriph As IntPtr,
                            ByRef data As Byte,
                            ByVal offset As UInt32,
                            ByVal length As UInt32) As PMDResult
    Friend Declare Function PMDPeriphWriteUInt16 Lib "C-Motion.dll" Alias "PMDPeriphWrite" (
                            ByVal hPeriph As IntPtr,
                            ByRef data As UInt16,
                            ByVal offset As UInt32,
                            ByVal length As UInt32) As PMDResult
    ' data should be an array of unsigned bytes, the first element should be passed.
    ' strings could be supported as well.
    Friend Declare Function PMDPeriphReceive Lib "C-Motion.dll" (
                            ByVal hPeriph As IntPtr,
                            ByRef data As Byte,
                            ByRef nReceived As UInt32,
                            ByVal nExpected As UInt32,
                            ByVal timeout As UInt32) As PMDResult
    ' data should be an array of unsigned bytes, the first element should be passed.
    ' strings could be supported as well.
    Friend Declare Function PMDPeriphSend Lib "C-Motion.dll" (
                            ByVal hPeriph As IntPtr,
                            ByRef data As Byte,
                            ByVal nCount As UInt32,
                            ByVal timeout As UInt32) As PMDResult

    Friend Declare Function PMDPeriphOpenDeviceMP Lib "C-Motion.dll" (
                            ByVal hDev As IntPtr,
                            ByVal hPeriph As IntPtr) As PMDResult
    Friend Declare Function PMDPeriphOpenDevicePRP Lib "C-Motion.dll" (
                            ByVal hDev As IntPtr,
                            ByVal hPeriph As IntPtr) As PMDResult
    Friend Declare Function PMDWaitForEvent Lib "C-Motion.dll" (
                            ByVal hDev As IntPtr,
                            ByRef EventStruct As PMDEvent_internal,
                            ByVal timeout As UInt32) As PMDResult


    ' Magellan commands
    Friend Declare Function PMDAdjustActualPosition Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal position As Int32) As PMDResult
    Friend Declare Function PMDCalibrateAnalog Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal parameter As UInt16) As PMDResult
    Friend Declare Function PMDClearDriveFaultStatus Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr) As PMDResult
    Friend Declare Function PMDClearInterrupt Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr) As PMDResult
    Friend Declare Function PMDClearPositionError Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr) As PMDResult
    Friend Declare Function PMDDriveNVRAM Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal parameter As UInt16,
                            ByVal value As UInt16) As PMDResult
    Friend Declare Function PMDGetAcceleration Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef acceleration As UInt32) As PMDResult
    Friend Declare Function PMDGetActiveMotorCommand Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef command As Int16) As PMDResult
    Friend Declare Function PMDGetActiveOperatingMode Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef mode As UInt16) As PMDResult
    Friend Declare Function PMDGetActivityStatus Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef status As UInt16) As PMDResult
    Friend Declare Function PMDGetActualPosition Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef position As Int32) As PMDResult
    Friend Declare Function PMDGetActualPositionUnits Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetActualVelocity Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As Int32) As PMDResult
    Friend Declare Function PMDGetAnalogCalibration Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal channel As Int16,
                            ByRef value As Int16) As PMDResult
    Friend Declare Function PMDSetAnalogCalibration Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal channel As Int16,
                            ByVal value As Int16) As PMDResult
    Friend Declare Function PMDGetAuxiliaryEncoderSource Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef mode As Byte,
                            ByRef AuxAxis As UInt16) As PMDResult
    Friend Declare Function PMDGetAxisOutMask Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef SourceAxis As UInt16,
                            ByRef SourceRegister As Byte,
                            ByRef SelectionMask As UInt16,
                            ByRef SenseMask As UInt16) As PMDResult
    Friend Declare Function PMDGetBreakpoint Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal BreakpointId As UInt16,
                            ByRef SourceAxis As UInt16,
                            ByRef Action As Byte,
                            ByRef Trigger As Byte) As PMDResult
    Friend Declare Function PMDGetBreakpointUpdateMask Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByRef ARG3 As UInt16) As PMDResult
    Friend Declare Function PMDGetBreakpointValue Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByRef ARG3 As Int32) As PMDResult
    Friend Declare Function PMDGetBufferLength Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal BufferId As UInt16,
                            ByRef ARG3 As UInt32) As PMDResult
    Friend Declare Function PMDGetBufferReadIndex Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal BufferId As UInt16,
                            ByRef ARG3 As UInt32) As PMDResult
    Friend Declare Function PMDGetBufferStart Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal BufferId As UInt16,
                            ByRef ARG3 As UInt32) As PMDResult
    Friend Declare Function PMDGetBufferWriteIndex Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal BufferId As UInt16,
                            ByRef ARG3 As UInt32) As PMDResult
    Friend Declare Function PMDGetBusVoltage Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetBusVoltageLimits Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal parameter As UInt16,
                            ByRef ARG3 As UInt16) As PMDResult
    Friend Declare Function PMDGetCANMode Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef NodeId As Byte,
                            ByRef TransmissionRate As Byte) As PMDResult
    Friend Declare Sub PMDGetCMotionVersion Lib "C-Motion.dll" (
                            ByRef ARG1 As UInt32,
                            ByRef ARG2 As UInt32)
    Friend Declare Function PMDGetCaptureSource Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetCaptureValue Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As Int32) As PMDResult
    Friend Declare Function PMDGetChecksum Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt32) As PMDResult
    Friend Declare Function PMDGetCommandedAcceleration Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As Int32) As PMDResult
    Friend Declare Function PMDGetCommandedPosition Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As Int32) As PMDResult
    Friend Declare Function PMDGetCommandedVelocity Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As Int32) As PMDResult
    Friend Declare Function PMDGetCommutationMode Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetCurrent Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal parameter As UInt16,
                            ByRef value As UInt16) As PMDResult
    Friend Declare Function PMDGetCurrentControlMode Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetCurrentFoldback Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal parameter As UInt16,
                            ByRef value As UInt16) As PMDResult
    Friend Declare Function PMDGetCurrentLoop Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal phase As Byte,
                            ByVal parameter As Byte,
                            ByRef value As UInt16) As PMDResult
    Friend Declare Function PMDGetCurrentLoopValue Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As Byte,
                            ByVal ARG3 As Byte,
                            ByRef ARG4 As Int32) As PMDResult
    Friend Declare Function PMDGetDeceleration Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt32) As PMDResult
    Friend Declare Function PMDGetDefault Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByRef ARG3 As UInt32) As PMDResult
    Friend Declare Function PMDGetDriveFaultStatus Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetDriveCommandMode Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef transport As Byte,
                            ByRef format As Byte) As PMDResult
    Friend Declare Function PMDGetDriveFaultParameter Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal parameter As UInt16,
                            ByRef value As UInt16) As PMDResult
    Friend Declare Function PMDGetDrivePWM Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal parameter As UInt16,
                            ByRef value As UInt16) As PMDResult
    Friend Declare Function PMDGetDriveStatus Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetDriveValue Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal node As UInt16,
                            ByRef value As UInt16) As PMDResult
    Friend Declare Function PMDGetEncoderModulus Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetEncoderSource Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetEncoderToStepRatio Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef counts As UInt16,
                            ByRef steps As UInt16) As PMDResult
    Friend Declare Function PMDGetEventAction Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByRef ARG3 As UInt16) As PMDResult
    Friend Declare Function PMDGetEventStatus Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetFeedbackParameter Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal parameter As UInt16,
                            ByRef value As UInt32) As PMDResult
    Friend Declare Function PMDGetFOC Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As Byte,
                            ByVal ARG3 As Byte,
                            ByRef ARG4 As UInt16) As PMDResult
    Friend Declare Function PMDGetFOCValue Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As Byte,
                            ByVal ARG3 As Byte,
                            ByRef ARG4 As Int32) As PMDResult
    Friend Declare Function PMDGetFaultOutMask Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetGearMaster Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef MasterAxis As UInt16,
                            ByRef source As UInt16) As PMDResult
    Friend Declare Function PMDGetGearRatio Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As Int32) As PMDResult
    Friend Declare Function PMDGetHoldingCurrent Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByRef ARG3 As UInt16) As PMDResult
    Friend Declare Function PMDGetInstructionError Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetInterruptAxis Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetInterruptMask Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetJerk Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt32) As PMDResult
    Friend Declare Function PMDGetMotionCompleteMode Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetMotorBias Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As Int16) As PMDResult
    Friend Declare Function PMDGetMotorCommand Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As Int16) As PMDResult
    Friend Declare Function PMDGetMotorLimit Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetMotorType Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetOperatingMode Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetOutputMode Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetOvertemperatureLimit Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetPWMFrequency Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetPhaseAngle Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetPhaseCommand Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByRef ARG3 As Int16) As PMDResult
    Friend Declare Function PMDGetPhaseCorrectionMode Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetPhaseCounts Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetPhaseInitializeMode Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetPhaseInitializeTime Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetPhaseOffset Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetPhasePrescale Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetPosition Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As Int32) As PMDResult
    Friend Declare Function PMDGetPositionError Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As Int32) As PMDResult
    Friend Declare Function PMDGetPositionErrorLimit Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt32) As PMDResult
    Friend Declare Function PMDGetPositionLoop Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByRef ARG3 As Int32) As PMDResult
    Friend Declare Function PMDGetPositionLoopValue Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByRef ARG3 As Int32) As PMDResult
    Friend Declare Function PMDGetProfileMode Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetSPIMode Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetSampleTime Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt32) As PMDResult
    Friend Declare Function PMDGetSerialPortMode Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef baud As Byte,
                            ByRef parity As Byte,
                            ByRef StopBits As Byte,
                            ByRef protocol As Byte,
                            ByRef MultiDropId As Byte) As PMDResult
    Friend Declare Function PMDGetSettleTime Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetSettleWindow Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetSignalSense Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetSignalStatus Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetStartVelocity Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt32) As PMDResult
    Friend Declare Function PMDGetStepRange Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetStopMode Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetSynchronizationMode Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetTemperature Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetTime Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt32) As PMDResult
    Friend Declare Function PMDGetTraceCount Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt32) As PMDResult
    Friend Declare Function PMDGetTraceMode Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetTracePeriod Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetTraceStart Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16,
                            ByRef ARG3 As Byte,
                            ByRef ARG4 As Byte,
                            ByRef ARG5 As Byte) As PMDResult
    Friend Declare Function PMDGetTraceStatus Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetTraceStop Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16,
                            ByRef ARG3 As Byte,
                            ByRef ARG4 As Byte,
                            ByRef ARG5 As Byte) As PMDResult
    Friend Declare Function PMDGetTraceVariable Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByRef ARG3 As UInt16,
                            ByRef ARG4 As Byte) As PMDResult
    Friend Declare Function PMDGetTraceValue Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByRef ARG3 As Int32) As PMDResult
    Friend Declare Function PMDGetTrackingWindow Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetUpdateMask Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetVelocity Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As Int32) As PMDResult
    Friend Declare Function PMDGetVersion Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef family As UInt16,
                            ByRef motorType As UInt16,
                            ByRef numberAxes As UInt16,
                            ByRef special_and_chip_count As UInt16,
                            ByRef custom As UInt16,
                            ByRef major As UInt16,
                            ByRef minor As UInt16) As PMDResult
    Friend Declare Function PMDInitializePhase Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr) As PMDResult
    Friend Declare Function PMDReadAnalog Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByRef ARG3 As UInt16) As PMDResult
    Friend Declare Function PMDReadBuffer Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal bufferID As UInt16,
                            ByRef data As Int32) As PMDResult
    Friend Declare Function PMDReadBuffer16 Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal bufferID As UInt16,
                            ByRef data As Int16) As PMDResult
    Friend Declare Function PMDReadIO Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByRef ARG3 As UInt16) As PMDResult
    Friend Declare Function PMDReset Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr) As PMDResult
    Friend Declare Function PMDResetEventStatus Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDRestoreOperatingMode Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr) As PMDResult
    Friend Declare Function PMDSetAcceleration Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt32) As PMDResult
    Friend Declare Function PMDSetActualPosition Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As Int32) As PMDResult
    Friend Declare Function PMDSetActualPositionUnits Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDSetAuxiliaryEncoderSource Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As Byte,
                            ByVal ARG3 As UInt16) As PMDResult
    Friend Declare Function PMDSetAxisOutMask Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByVal ARG3 As Byte,
                            ByVal ARG4 As UInt16,
                            ByVal ARG5 As UInt16) As PMDResult
    Friend Declare Function PMDSetBreakpoint Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByVal ARG3 As UInt16,
                            ByVal ARG4 As Byte,
                            ByVal ARG5 As Byte) As PMDResult
    Friend Declare Function PMDSetBreakpointUpdateMask Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByVal ARG3 As UInt16) As PMDResult
    Friend Declare Function PMDSetBreakpointValue Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByVal ARG3 As Int32) As PMDResult
    Friend Declare Function PMDSetBufferLength Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByVal ARG3 As UInt32) As PMDResult
    Friend Declare Function PMDSetBufferReadIndex Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByVal ARG3 As UInt32) As PMDResult
    Friend Declare Function PMDSetBufferStart Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByVal ARG3 As UInt32) As PMDResult
    Friend Declare Function PMDSetBufferWriteIndex Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByVal ARG3 As UInt32) As PMDResult
    Friend Declare Function PMDSetBusVoltageLimits Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByVal ARG3 As UInt16) As PMDResult
    Friend Declare Function PMDSetCANMode Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As Byte,
                            ByVal ARG3 As Byte) As PMDResult
    Friend Declare Function PMDSetCaptureSource Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDSetCommutationMode Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDSetCurrent Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal parameter As UInt16,
                            ByVal value As UInt16) As PMDResult
    Friend Declare Function PMDSetCurrentControlMode Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDSetCurrentFoldback Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal parameter As UInt16,
                            ByVal value As UInt16) As PMDResult
    Friend Declare Function PMDSetCurrentLoop Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal phase As Byte,
                            ByVal parameter As Byte,
                            ByVal value As UInt16) As PMDResult
    Friend Declare Function PMDSetDeceleration Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt32) As PMDResult
    Friend Declare Function PMDSetDefault Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByVal ARG3 As UInt32) As PMDResult
    Friend Declare Function PMDSetDriveFaultParameter Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal parameter As UInt16,
                            ByVal value As UInt16) As PMDResult
    Friend Declare Function PMDSetDrivePWM Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal parameter As UInt16,
                            ByVal value As UInt16) As PMDResult
    Friend Declare Function PMDSetEncoderModulus Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDSetEncoderSource Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDSetEncoderToStepRatio Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal counts As UInt16,
                            ByVal steps As UInt16) As PMDResult
    Friend Declare Function PMDSetEventAction Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByVal ARG3 As UInt16) As PMDResult
    Friend Declare Function PMDSetFeedbackParameter Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal parameter As UInt16,
                            ByVal value As UInt32) As PMDResult
    Friend Declare Function PMDSetFOC Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As Byte,
                            ByVal ARG3 As Byte,
                            ByVal ARG4 As UInt16) As PMDResult
    Friend Declare Function PMDSetFaultOutMask Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDSetGearMaster Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal MasterAxis As UInt16,
                            ByVal source As UInt16) As PMDResult
    Friend Declare Function PMDSetGearRatio Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As Int32) As PMDResult
    Friend Declare Function PMDSetHoldingCurrent Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByVal ARG3 As UInt16) As PMDResult
    Friend Declare Function PMDSetInterruptMask Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDSetJerk Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt32) As PMDResult
    Friend Declare Function PMDSetMotionCompleteMode Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDSetMotorBias Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As Int16) As PMDResult
    Friend Declare Function PMDSetMotorCommand Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As Int16) As PMDResult
    Friend Declare Function PMDSetMotorLimit Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDSetMotorType Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDSetOperatingMode Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDSetOutputMode Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDSetOvertemperatureLimit Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDSetPWMFrequency Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDSetPhaseAngle Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDSetPhaseCorrectionMode Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDSetPhaseCounts Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDSetPhaseInitializeMode Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDSetPhaseInitializeTime Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDSetPhaseOffset Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDSetPhasePrescale Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDSetPosition Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As Int32) As PMDResult
    Friend Declare Function PMDSetPositionErrorLimit Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt32) As PMDResult
    Friend Declare Function PMDSetPositionLoop Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByVal ARG3 As Int32) As PMDResult
    Friend Declare Function PMDSetProfileMode Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDSetSPIMode Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDSetSampleTime Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt32) As PMDResult
    Friend Declare Function PMDSetSerialPortMode Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As Byte,
                            ByVal ARG3 As Byte,
                            ByVal ARG4 As Byte,
                            ByVal ARG5 As Byte,
                            ByVal ARG6 As Byte) As PMDResult
    Friend Declare Function PMDSetSettleTime Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDSetSettleWindow Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDSetSignalSense Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDSetStartVelocity Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt32) As PMDResult
    Friend Declare Function PMDSetStepRange Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDSetStopMode Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDSetSynchronizationMode Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDSetTraceMode Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDSetTracePeriod Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDSetTraceStart Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByVal ARG3 As Byte,
                            ByVal ARG4 As Byte,
                            ByVal ARG5 As Byte) As PMDResult
    Friend Declare Function PMDSetTraceStop Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByVal ARG3 As Byte,
                            ByVal ARG4 As Byte,
                            ByVal ARG5 As Byte) As PMDResult
    Friend Declare Function PMDSetTraceVariable Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByVal ARG3 As UInt16,
                            ByVal ARG4 As Byte) As PMDResult
    Friend Declare Function PMDSetTrackingWindow Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDSetUpdateMask Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDSetVelocity Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As Int32) As PMDResult
    Friend Declare Function PMDUpdate Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr) As PMDResult
    Friend Declare Function PMDWriteBuffer Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByVal ARG3 As Int32) As PMDResult
    Friend Declare Function PMDWriteIO Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByVal ARG3 As UInt16) As PMDResult
    Friend Declare Function PMDSetLoop Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByVal ARG3 As Int32) As PMDResult
    Friend Declare Function PMDGetLoop Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByRef ARG3 As Int32) As PMDResult
    Friend Declare Function PMDGetLoopValue Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByRef ARG3 As Int32) As PMDResult
    Friend Declare Function PMDSetProfileParameter Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByVal ARG3 As Int32) As PMDResult
    Friend Declare Function PMDGetProfileParameter Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByRef ARG3 As Int32) As PMDResult
    Friend Declare Function PMDGetProductInfo Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByRef ARG3 As UInt32) As PMDResult
    Friend Declare Function PMDSetCommutationParameter Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByVal ARG3 As Int32) As PMDResult
    Friend Declare Function PMDGetCommutationParameter Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByRef ARG3 As Int32) As PMDResult
    Friend Declare Function PMDSetPhaseParameter Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByVal ARG3 As Int16) As PMDResult
    Friend Declare Function PMDGetPhaseParameter Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16,
                            ByRef ARG3 As Int16) As PMDResult
    Friend Declare Function PMDSetDriveCommandMode Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As Int16) As PMDResult
    Friend Declare Function PMDGetDriveCommandMode Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As Int16) As PMDResult
    Friend Declare Function PMDGetRuntimeError Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDSetCurrentLimit Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDGetCurrentLimit Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByRef ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDMultiUpdate Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr,
                            ByVal ARG2 As UInt16) As PMDResult
    Friend Declare Function PMDNoOperation Lib "C-Motion.dll" (
                            ByVal hAxis As IntPtr) As PMDResult



    '*** Utility functions
    Public Function PMDresultString(ByVal status As PMDResult) As String
        Select Case status
            Case PMDResult.NOERROR
                Return "0: No error"
            Case PMDResult.ERR_MP_Reset
                Return "1: Reset"
            Case PMDResult.ERR_MP_InvalidInstruction
                Return "2: Invalid Instruction"
            Case PMDResult.ERR_MP_InvalidAxis
                Return "3: Invalid Axis"
            Case PMDResult.ERR_MP_InvalidParameter
                Return "4: Invalid Parameter"
            Case PMDResult.ERR_MP_TraceRunning
                Return "5: Trace Running"
            Case PMDResult.ERR_MP_BlockOutOfBounds
                Return "7: Block Out Of Bounds"
            Case PMDResult.ERR_MP_TraceBufferZero
                Return "8: Trace Buffer Zero"
            Case PMDResult.ERR_MP_BadSerialChecksum
                Return "9: Bad Serial Checksum"
            Case PMDResult.ERR_MP_InvalidNegativeValue
                Return "11: Invalid Negative Value"
            Case PMDResult.ERR_MP_InvalidParameterChange
                Return "12: Invalid Parameter Change"
            Case PMDResult.ERR_MP_LimitEventPending
                Return "13: Limit Event Pending"
            Case PMDResult.ERR_MP_InvalidMoveIntoLimit
                Return "14: Invalid Move Into Limit"
            Case PMDResult.ERR_MP_InvalidOperatingModeRestore
                Return "16: Invalid Operating Mode Restore"
            Case PMDResult.ERR_MP_InvalidOperatingModeForCommand
                Return "17: Invalid Operating Mode For Command"
            Case PMDResult.ERR_MP_BadState
                Return "18: BadState"
            Case PMDResult.ERR_MP_HardFault
                Return "19: HardFault"
            Case PMDResult.ERR_MP_AtlasNotDetected
                Return "20: AtlasNotDetected"
            Case PMDResult.ERR_MP_BadSPIChecksum
                Return "21: BadSPIChecksum"
            Case PMDResult.ERR_MP_InvalidSPIprotocol
                Return "22: InvalidSPIprotocol"
            Case PMDResult.ERR_MP_InvalidTorqueCommand
                Return "24: InvalidTorqueCommand"
            Case PMDResult.ERR_MP_BadFlashChecksum
                Return "25: BadFlashChecksum"
            Case PMDResult.ERR_MP_InvalidFlashModeCommand
                Return "26: InvalidFlashModeCommand"
            Case PMDResult.ERR_MP_ReadOnly
                Return "27: ReadOnly"
            Case PMDResult.ERR_MP_InitializationOnlyCommand
                Return "28: InitializationOnlyCommand"
            Case PMDResult.ERR_MP_IncorrectDataCount
                Return "29: IncorrectDataCount"
            Case PMDResult.ERR_MP_MoveWhileInError
                Return "30: MoveWhileInError"
            Case PMDResult.ERR_MP_WaitTimedOut
                Return "31: WaitTimeout"
            Case PMDResult.ERR_MP_InitializationRunning
                Return "32: NVRAM Init Running"
            Case PMDResult.ERR_MP_InvalidClock
                Return "33: No Clock"
            Case PMDResult.ERR_MP_InitializationSkipped
                Return "34: NVRAM Init Skipped"


            Case PMDResult.ERR_Version
                Return "H1002: Version"
            Case PMDResult.ERR_WriteError
                Return "H1005: WriteError"
            Case PMDResult.ERR_ReadError
                Return "H1006: ReadError"
            Case PMDResult.ERR_Cancelled
                Return "H1007: Cancelled"
            Case PMDResult.ERR_CommunicationsError
                Return "H1008: CommunicationsError"
            Case PMDResult.ERR_InsufficientDataReceived
                Return "H100A: InsufficientDataReceived"
            Case PMDResult.ERR_UnexpectedDataReceived
                Return "H100B: UnexpectedDataReceived"
            Case PMDResult.ERR_Memory
                Return "H100C: Memory"
            Case PMDResult.ERR_Timeout
                Return "H100D: Timeout"
            Case PMDResult.ERR_Checksum
                Return "H100E: Checksum"
            Case PMDResult.ERR_CommandError
                Return "H100F: CommandError"
            Case PMDResult.ERR_MutexTimeout
                Return "H1010: MutexTimeout"

            Case PMDResult.ERR_NotSupported
                Return "H1101: NotSupported"
            Case PMDResult.ERR_InvalidOperation
                Return "H1102: InvalidOperation"
            Case PMDResult.ERR_InvalidInterface
                Return "H1103: InvalidInterface"
            Case PMDResult.ERR_InvalidPort
                Return "H1104: InvalidPort"
            Case PMDResult.ERR_InvalidBaud
                Return "H1105: InvalidBaud"
            Case PMDResult.ERR_InvalidHandle
                Return "H1106: InvalidHandle"
            Case PMDResult.ERR_InvalidDataSize
                Return "H1107: InvalidDataSize"
            Case PMDResult.ERR_InvalidParameter
                Return "H1108: InvalidParameter"
            Case PMDResult.ERR_InvalidAddress
                Return "H1109: InvalidAddress"
            Case PMDResult.ERR_ParameterOutOfRange
                Return "H110A: ParameterOutOfRange"
            Case PMDResult.ERR_ParameterAlignment
                Return "H110B: ParameterAlignment"

            Case PMDResult.ERR_NotConnected
                Return "H1201: NotConnected"
            Case PMDResult.ERR_NotResponding
                Return "H1202: NotResponding"
            Case PMDResult.ERR_PortRead
                Return "H1203: PortRead"
            Case PMDResult.ERR_PortWrite
                Return "H1204: PortWrite"
            Case PMDResult.ERR_OpeningPort
                Return "H1205: OpeningPort"
            Case PMDResult.ERR_ConfiguringPort
                Return "H1206: ConfiguringPort"
            Case PMDResult.ERR_InterfaceNotInitialized
                Return "H1207: InterfaceNotInitialized"
            Case PMDResult.ERR_Driver
                Return "H1208: Driver"
            Case PMDResult.ERR_AddressInUse
                Return "H1209: AddressInUse"
            Case PMDResult.ERR_IPRouting
                Return "H120A: IPRouting"
            Case PMDResult.ERR_OutOfResources
                Return "H120B: OutOfResources"
            Case PMDResult.ERR_QueueFull
                Return "H120C: QueueFull"
            Case PMDResult.ERR_QueueEmpty
                Return "H120D: QueueEmpty"
            Case PMDResult.ERR_ResourceInUse
                Return "H120E: ResourceInUse"
            Case PMDResult.ERR_SerialOverrun
                Return "H1211: SerialOverrun"
            Case PMDResult.ERR_SerialBreak
                Return "H1212: SerialBreak"
            Case PMDResult.ERR_SerialParity
                Return "H1213: SerialParity"
            Case PMDResult.ERR_SerialFrame
                Return "H1214: SerialFrame"
            Case PMDResult.ERR_SerialNoise
                Return "H1215: SerialNoise"
            Case PMDResult.ERR_ReceiveOverrun
                Return "H1220: ReceiveOverrun"
            Case PMDResult.ERR_CAN_BitStuff
                Return "H1221: CAN_BitStuff"
            Case PMDResult.ERR_CAN_Form
                Return "H1222: CAN_Form"
            Case PMDResult.ERR_CAN_Acknowledge
                Return "H1223: CAN_Acknowledge"
            Case PMDResult.ERR_CAN_BitRecessive
                Return "H1224: CAN_BitRecessive"
            Case PMDResult.ERR_CAN_BitDominant
                Return "H1225: CAN_BitDominant"
            Case PMDResult.ERR_CAN_CRC
                Return "H1226: CAN_CRC"
            Case PMDResult.ERR_CAN_BusOff
                Return "H1228: CAN_BusOff"
            Case PMDResult.ERR_CAN_Passive
                Return "H1229: CAN_Passive"
            Case PMDResult.ERR_CAN_Active
                Return "H122A: CAN_Active"

            Case PMDResult.ERR_RP_Reset
                Return "H2001: RP_Reset"
            Case PMDResult.ERR_RP_InvalidVersion
                Return "H2002: RP_InvalidVersion"
            Case PMDResult.ERR_RP_InvalidResource
                Return "H2003: RP_InvalidResource"
            Case PMDResult.ERR_RP_InvalidAddress
                Return "H2004: RP_InvalidAddress"
            Case PMDResult.ERR_RP_InvalidAction
                Return "H2005: RP_InvalidAction"
            Case PMDResult.ERR_RP_InvalidSubAction
                Return "H2006: RP_InvalidSubAction"
            Case PMDResult.ERR_RP_InvalidCommand
                Return "H2007: RP_InvalidCommand"
            Case PMDResult.ERR_RP_InvalidParameter
                Return "H2008: RP_InvalidParameter"
            Case PMDResult.ERR_RP_InvalidPacket
                Return "H2009: RP_InvalidPacket"
            Case PMDResult.ERR_RP_PacketLength
                Return "H200A: RP_PacketLength"
            Case PMDResult.ERR_RP_PacketAlignment
                Return "H200B: RP_PacketAlignment"
            Case PMDResult.ERR_RP_Checksum
                Return "H200E: RP_Checksum"

            Case PMDResult.ERR_UC_Signature
                Return "H2101: UC_Signature"
            Case PMDResult.ERR_UC_Version
                Return "H2102: UC_Version"
            Case PMDResult.ERR_UC_FileSize
                Return "H2103: UC_FileSize"
            Case PMDResult.ERR_UC_Checksum
                Return "H2104: UC_Checksum"
            Case PMDResult.ERR_UC_WriteError
                Return "H2105: UC_WriteError"
            Case PMDResult.ERR_UC_NotProgrammed
                Return "H2106: UC_NotProgrammed"
            Case PMDResult.ERR_UC_TaskNotCreated
                Return "H2107: UC_TaskNotCreated"
            Case PMDResult.ERR_UC_TaskAlreadyRunning
                Return "H2108: UC_TaskAlreadyRunning"
            Case PMDResult.ERR_UC_TaskNotFound
                Return "H2109: UC_TaskNotFound"
            Case PMDResult.ERR_UC_Format
                Return "H210A: UC_Format"
            Case PMDResult.ERR_UC_Reserved
                Return "H210B: UC_Reserved"
            Case PMDResult.ERR_UC_TaskAborted
                Return "H210C: UC_TaskAborted"

            Case PMDResult.ERR_NVRAM
                Return "H2200: NVRAM"

            Case Else
                Return status.ToString() & ": Undefined error code"
        End Select
    End Function


    Public Class PMDPeripheral
        '*** Private data and utility functions ***
        'hPeriph is a pointer to the C-allocated peripheral handle, it is not user-visible in VB
        Friend hPeriph As IntPtr
        Friend status As PMDResult

        Friend Sub CheckResult(ByVal status As PMDResult)
            Me.status = status
            If (Not status = PMDResult.NOERROR) Then
                Dim e As New Exception("ERROR: PMDPeripheral " & PMDresultString(status))
                e.Data.Add("PMDresult", status)
                Throw e
            End If
        End Sub

        '*** Public Methods ***
        Public Sub New()
            hPeriph = PMDPeriphAlloc()
            If (hPeriph = 0) Then
                Throw New Exception("ERROR: PMD library: could not allocate peripheral object")
            End If
        End Sub

        Protected Overrides Sub Finalize()
            If (Not hPeriph = 0) Then
                PMDPeriphClose(hPeriph)
                PMDPeriphFree(hPeriph)
                hPeriph = 0
            End If
        End Sub

        Public Sub Close()
            Me.Finalize()
        End Sub

        Public ReadOnly Property LastError() As PMDResult
            Get
                Return status
            End Get
        End Property

        Public Sub Flush()
            CheckResult(PMDPeriphFlush(hPeriph))
        End Sub

        Public Sub Send(ByRef data() As Byte, ByVal nCount As UInt32, ByVal timeout As UInt32)
            If (nCount > data.Length()) Then
                Throw New Exception("PMDPeripheral.Send bad nCount")
            End If
            CheckResult(PMDPeriphSend(hPeriph, data(0), nCount, timeout))
        End Sub

        Public Sub Receive(ByRef data() As Byte, ByRef nReceived As UInt32,
                           ByVal nExpected As UInt32, ByVal timeout As UInt32)
            Dim nrecv As UInt32
            If (nExpected > data.Length()) Then
                Throw New Exception("PMDPeripheral.Receive bad nExpected")
            End If
            CheckResult(PMDPeriphReceive(hPeriph, data(0), nrecv, nExpected, timeout))
            nReceived = nrecv
        End Sub
        Public Sub Read(ByRef data As Byte(), ByVal offset As UInt32, ByVal length As UInt32)
            If (length > data.Length()) Then
                Throw New Exception("PMDPeripheral.Read bad length")
            End If
            CheckResult(PMDPeriphReadUInt8(hPeriph, data(0), offset, length))
        End Sub

        Public Sub Read(ByRef data As UInt16(), ByVal offset As UInt32, ByVal length As UInt32)
            If (length > data.Length()) Then
                Throw New Exception("PMDPeripheral.Read bad length")
            End If
            CheckResult(PMDPeriphReadUInt16(hPeriph, data(0), offset, length))
        End Sub

        Public Function Read(ByVal offset As UInt32) As UInt16
            Dim data As UInt16
            CheckResult(PMDPeriphReadUInt16(hPeriph, data, offset, 1))
            Return data
        End Function

        Public Sub Write(ByRef data As Byte(), ByVal offset As UInt32, ByVal length As UInt32)
            If (length > data.Length()) Then
                Throw New Exception("PMDPeripheral.Write bad length")
            End If
            CheckResult(PMDPeriphWriteUInt8(hPeriph, data(0), offset, length))
        End Sub

        Public Sub Write(ByRef data As UInt16(), ByVal offset As UInt32, ByVal length As UInt32)
            If (length > data.Length()) Then
                Throw New Exception("PMDPeripheral.Write bad length")
            End If
            CheckResult(PMDPeriphWriteUInt16(hPeriph, data(0), offset, length))
        End Sub

        Public Sub Write(ByVal data As UInt16, ByVal offset As UInt32)
            CheckResult(PMDPeriphWriteUInt16(hPeriph, data, offset, 1))
        End Sub

    End Class    'PMDPeripheral

    Public Class PMDPeripheralCAN
        Inherits PMDPeripheral

        Public Sub New(ByVal transmitid As UInt32, ByVal receiveid As UInt32, ByVal eventid As UInt32)
            MyBase.New()
            Dim r As PMDResult
            r = PMDDeviceOpenPeriphCAN(hPeriph, 0, transmitid, receiveid, eventid)
            If (Not r = PMDResult.NOERROR) Then
                MyBase.Close()
                CheckResult(r)
            End If
        End Sub
        Public Sub New(ByRef device As PMDDevice, ByVal transmitid As UInt32, ByVal receiveid As UInt32, ByVal eventid As UInt32)
            MyBase.New()
            Dim r As PMDResult
            r = PMDDeviceOpenPeriphCAN(hPeriph, device.hDev, transmitid, receiveid, eventid)
            If (Not r = PMDResult.NOERROR) Then
                MyBase.Close()
                CheckResult(r)
            End If
        End Sub

    End Class   'PMDPeripheralCAN

    Public Class PMDPeripheralCANFD
        Inherits PMDPeripheral

        Public Sub New(ByVal port As PMDCANPort, ByVal baud As UInt32, ByVal transmitid As UInt32, ByVal receiveid As UInt32, ByVal eventid As UInt32)
            MyBase.New()
            Dim r As PMDResult
            r = PMDDeviceOpenPeriphCANFD(hPeriph, 0, port, baud, transmitid, receiveid, eventid)
            If (Not r = PMDResult.NOERROR) Then
                MyBase.Close()
                CheckResult(r)
            End If
        End Sub
        Public Sub New(ByRef device As PMDDevice, ByVal port As PMDCANPort, ByVal baud As UInt32, ByVal transmitid As UInt32, ByVal receiveid As UInt32, ByVal eventid As UInt32)
            MyBase.New()
            Dim r As PMDResult
            r = PMDDeviceOpenPeriphCANFD(hPeriph, device.hDev, port, baud, transmitid, receiveid, eventid)
            If (Not r = PMDResult.NOERROR) Then
                MyBase.Close()
                CheckResult(r)
            End If
        End Sub

    End Class   'PMDPeripheralCAN

    Public Class PMDPeripheralSerial
        Inherits PMDPeripheral

        Public Sub New(ByVal portnum As UInt32, ByVal baud As UInt32,
                       ByVal parity As PMDSerialParity, ByVal StopBits As PMDSerialStopBits)
            MyBase.New()
            Dim r As PMDResult
            r = PMDDeviceOpenPeriphSerial(hPeriph, 0, portnum, baud, parity, StopBits)
            If (Not r = PMDResult.NOERROR) Then
                MyBase.Close()
                CheckResult(r)
            End If

        End Sub
        Public Sub New(ByRef device As PMDDevice, ByVal portnum As UInt32, ByVal baud As UInt32,
                       ByVal parity As PMDSerialParity, ByVal StopBits As PMDSerialStopBits)
            MyBase.New()
            Dim r As PMDResult
            r = PMDDeviceOpenPeriphSerial(hPeriph, device.hDev, portnum, baud, parity, StopBits)
            If (Not r = PMDResult.NOERROR) Then
                MyBase.Close()
                CheckResult(r)
            End If
        End Sub

        Public Sub SerialSync()
            CheckResult(PMDSerial_Sync(hPeriph))
        End Sub

    End Class   'PMDPeripheralSerial

    Public Class PMDPeripheralMultiDrop
        Inherits PMDPeripheral

        Public Sub New(ByRef parent As PMDPeripheral, ByVal address As Int16)
            MyBase.New()
            Dim r As PMDResult
            r = PMDPeriphOpenPeriphMultiDrop(hPeriph, parent.hPeriph, address)
            If (Not r = PMDResult.NOERROR) Then
                MyBase.Close()
                CheckResult(r)
            End If
        End Sub

    End Class   'PMDPeripheralMultiDrop

    Public Class PMDPeripheralCME
        Inherits PMDPeripheral

        Public Sub New(ByRef device As PMDDevice)
            MyBase.New()
            Dim r As PMDResult
            r = PMDDeviceOpenPeriphCME(hPeriph, device.hDev)
            If (Not r = PMDResult.NOERROR) Then
                MyBase.Close()
                CheckResult(r)
            End If
        End Sub

    End Class   'PMDPeripheralCME

    Public Class PMDPeripheralPRP
        Inherits PMDPeripheral

        Public Sub New(ByRef device As PMDDevice, ByVal channel As UInt32, ByVal bufsize As UInt32)
            MyBase.New()
            Dim r As PMDResult
            r = PMDDeviceOpenPeriphPRP(hPeriph, device.hDev, channel, bufsize)
            If (Not r = PMDResult.NOERROR) Then
                MyBase.Close()
                CheckResult(r)
            End If
        End Sub

    End Class   'PMDPeripheralPRP

    Public Class PMDPeripheralTCP
        Inherits PMDPeripheral

        Public Sub New(ByVal address As System.Net.IPAddress,
                       ByVal portnum As UInt32,
                       ByVal timeout As UInt32)
            MyBase.New()
            Dim r As PMDResult
            Dim uaddr As UInt32
            Dim addr_bytes As Byte()
            Dim i As Integer
            If (Not (address.AddressFamily.Equals(System.Net.Sockets.AddressFamily.InterNetwork))) Then
                Throw New Exception("PMDPeripheralTCP supports only IPv4")
            End If

            ' High byte is first, low byte is last
            addr_bytes = address.GetAddressBytes()
            uaddr = 0
            For i = 0 To addr_bytes.Length - 1
                uaddr = (uaddr << 8) + addr_bytes(i)
            Next i

            r = PMDDeviceOpenPeriphTCP(hPeriph, 0, uaddr, portnum, timeout)
            If (Not r = PMDResult.NOERROR) Then
                MyBase.Close()
                CheckResult(r)
            End If
        End Sub

    End Class   'PMDPeripheralTCP

    Public Class PMDPeripheralPIO
        Inherits PMDPeripheral

        Public Sub New(ByVal device As PMDDevice,
                       ByVal address As Int16,
                       ByVal eventIRQ As Byte,
                       ByVal datasize As PMDDataSize)
            MyBase.New()
            Dim r As PMDResult
            r = PMDDeviceOpenPeriphPIO(hPeriph, device.hDev, address, eventIRQ, datasize)
            If (Not r = PMDResult.NOERROR) Then
                MyBase.Close()
                CheckResult(r)
            End If
        End Sub

    End Class   'PMDPeripheralPIO

    Public Class PMDDevice
        '*** Private data and utility functions ***
        Friend hDev As IntPtr
        Public DeviceType As PMDDeviceType
        Friend status As PMDResult

        Friend Sub CheckResult(ByVal status As PMDResult)
            Me.status = status
            If (Not status = PMDResult.NOERROR) Then
                Dim e As New Exception("ERROR: PMDDevice " & PMDresultString(status))
                e.Data.Add("PMDresult", status)
                Throw e
            End If
        End Sub

        '*** Public Methods ***
        Public Sub New(ByVal periph As PMDPeripheral, ByVal dtype As PMDDeviceType)
            hDev = PMDDeviceAlloc()
            If (hDev = 0) Then
                Throw New Exception("ERROR: PMD library: could not allocate device object")
            End If

            Select Case dtype
                Case PMDDeviceType.MotionProcessor
                    CheckResult(PMDPeriphOpenDeviceMP(hDev, periph.hPeriph))
                Case PMDDeviceType.ResourceProtocol
                    CheckResult(PMDPeriphOpenDevicePRP(hDev, periph.hPeriph))

                    '*** Absorb a ERR_RP_Reset
                    'Remove the following if you want to be informed that the conected device was reset at some point before connecting.
                    Me.status = PMDDeviceNoOperation(hDev)
            End Select
            DeviceType = dtype
        End Sub

        Protected Overrides Sub Finalize()
            If (Not hDev = 0) Then
                PMDDeviceClose(hDev)
                PMDDeviceFree(hDev)
                hDev = 0
            End If
        End Sub

        Public Sub Close()
            Me.Finalize()
        End Sub

        Public ReadOnly Property LastError() As PMDResult
            Get
                Return status
            End Get
        End Property

        Public Sub Reset()
            PMDDeviceReset(hDev)
        End Sub

        Public Sub Version(ByRef major As UInt32, ByRef minor As UInt32)
            CheckResult(PMDDeviceGetVersion(hDev, major, minor))
        End Sub

        Public Function GetUserCodeDate() As String
            Dim filedate(0 To 260) As Byte
            CheckResult(PMDCMEGetUserCodeDate(hDev, filedate(0)))
            Return System.Text.Encoding.ASCII.GetString(filedate)
        End Function

        Public Function GetUserCodeName() As String
            Dim filename(0 To 260) As Byte
            CheckResult(PMDCMEGetUserCodeName(hDev, filename(0)))
            Return System.Text.Encoding.ASCII.GetString(filename)
        End Function

        Public Function UserCodeChecksum() As UInt32
            Dim value As UInt32
            CheckResult(PMDCMEGetUserCodeChecksum(hDev, value))
            Return value
        End Function

        Public Function UserCodeVersion() As UInt32
            Dim value As UInt32
            CheckResult(PMDCMEGetUserCodeVersion(hDev, value))
            Return value
        End Function

        Public Function GetInfo(ByVal infoid As PMDDeviceInfo, ByVal subid As UInt16) As UInt32
            Dim value As UInt32
            CheckResult(PMDDeviceGetInfo(hDev, infoid, subid, value))
            Return value
        End Function

        Public Function GetFaultCode(ByVal faultid As PMDFaultCode) As UInt32
            Dim value As UInt32
            CheckResult(PMDDeviceGetFaultCode(hDev, faultid, value))
            Return value
        End Function

        Public Sub GetSystemTime(ByRef time As SYSTEMTIME)
            CheckResult(PMDDeviceGetSystemTime(hDev, time))
        End Sub

        Public Sub SetSystemTime(ByRef time As SYSTEMTIME)
            CheckResult(PMDDeviceSetSystemTime(hDev, time))
        End Sub

        Public Sub SetNodeID(ByVal nodeID As Byte, ByVal DOsignal As Byte, ByVal DIsignal As Byte, ByVal DIsense As Byte)
            CheckResult(PMDDeviceSetNodeID(hDev, nodeID, DOsignal, DIsignal, DIsense))
        End Sub

        Public Function WaitForEvent(ByRef EventStruct As PMDEvent,
                                     ByVal timeout As UInt32) As PMDResult
            Dim r As PMDResult
            Dim ev As PMDEvent_internal
            ev.axis = EventStruct.axis
            ev.EventMask = EventStruct.EventMask
            r = PMDWaitForEvent(hDev, ev, timeout)
            CheckResult(r)
            EventStruct.axis = ev.axis
            EventStruct.EventMask = ev.EventMask
            Return r
        End Function

        Public Function GetDefault(ByVal code As PMDDefault) As UInt32
            Dim value As UInt32
            Dim datasize As UInt32
            datasize = 4
            CheckResult(PMDDeviceGetDefaultUInt32(hDev, code, value, datasize))
            Return value
        End Function

        Public Sub SetDefault(ByVal code As PMDDefault, ByVal value As UInt32)
            CheckResult(PMDDeviceSetDefaultUInt32(hDev, code, CType(value, UInt32), 4))
        End Sub

        'Task control methods
        Public ReadOnly Property TaskState() As PMDTaskState
            Get
                Dim state As PMDTaskState
                CheckResult(PMDCMETaskGetState(hDev, state))
                Return state
            End Get
        End Property

        Public Function GetTaskInfo(ByVal taskno As Byte, ByVal infoid As PMDTaskInfo) As UInt32
            Dim value As UInt32
            CheckResult(PMDCMETaskGetInfo(hDev, taskno, infoid, value))
            Return value
        End Function

        Public Sub TaskStart()
            CheckResult(PMDCMETaskStart(hDev, 0))
        End Sub
        Public Sub TaskStart(ByVal param)
            CheckResult(PMDCMETaskStart(hDev, param))
        End Sub

        Public Sub TaskStop()
            CheckResult(PMDCMETaskStop(hDev))
        End Sub
    End Class 'PMDDevice

    Public Class PMDAxis
        '*** Private data and utility functions ***
        Friend hAxis As IntPtr
        Friend status As PMDResult

        Friend Sub CheckResult(ByVal status As PMDResult)
            Me.status = status
            If (Not status = PMDResult.NOERROR) Then
                Dim e As New Exception("ERROR: PMDAxis " & PMDresultString(status))
                e.Data.Add("PMDresult", status)
                Throw e
            End If
        End Sub

        Friend Sub New()
            hAxis = PMDAxisAlloc()
            If (hAxis = 0) Then
                Throw New Exception("ERROR: PMD library: could not allocate axis object")
            End If
        End Sub


        '*** Public Methods ***
        Public Sub New(ByVal device As PMDDevice, ByVal AxisNumber As PMDAxisNumber)
            hAxis = PMDAxisAlloc()
            If (hAxis = 0) Then
                Throw New Exception("ERROR: PMD library: could not allocate axis object")
            End If
            CheckResult(PMDAxisOpen(hAxis, device.hDev, AxisNumber))
        End Sub

        Public Function AtlasAxis() As PMDAxis
            Dim Atlas As PMDAxis
            Atlas = New PMDAxis()
            Atlas.hAxis = PMDAxisAlloc()
            If (Atlas.hAxis = 0) Then
                Throw New Exception("ERROR: PMD library: could not allocate axis object")
            End If
            CheckResult(PMDAtlasAxisOpen(hAxis, Atlas.hAxis))
            Return Atlas
        End Function

        Protected Overrides Sub Finalize()
            If (Not hAxis = 0) Then
                PMDAxisClose(hAxis)
                PMDAxisFree(hAxis)
                hAxis = 0
            End If
        End Sub

        Public Sub Close()
            Me.Finalize()
        End Sub

        Public ReadOnly Property LastError() As PMDResult
            Get
                Return status
            End Get
        End Property

        Public Sub Reset()
            PMDReset(hAxis)
        End Sub

        ' Axis Class Properties and Methods
        Public Property Acceleration() As UInt32
            Get
                Dim tmp As UInt32
                CheckResult(PMDGetAcceleration(hAxis, tmp))
                Return tmp
            End Get
            Set(ByVal value As UInt32)
                CheckResult(PMDSetAcceleration(hAxis, value))
            End Set
        End Property

        Public ReadOnly Property ActiveMotorCommand() As Int16
            Get
                Dim tmp As Int16
                CheckResult(PMDGetActiveMotorCommand(hAxis, tmp))
                Return tmp
            End Get
        End Property

        Public ReadOnly Property ActiveOperatingMode() As UInt16
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetActiveOperatingMode(hAxis, tmp))
                Return tmp
            End Get
        End Property

        Public ReadOnly Property ActivityStatus() As UInt16
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetActivityStatus(hAxis, tmp))
                Return tmp
            End Get
        End Property

        Public Property ActualPosition() As Int32
            Get
                Dim tmp As Int32
                CheckResult(PMDGetActualPosition(hAxis, tmp))
                Return tmp
            End Get
            Set(ByVal value As Int32)
                CheckResult(PMDSetActualPosition(hAxis, value))
            End Set
        End Property

        Public Sub AdjustActualPosition(ByVal increment As Int32)
            CheckResult(PMDAdjustActualPosition(hAxis, increment))
        End Sub

        Public Property ActualPositionUnits() As PMDActualPositionUnits
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetActualPositionUnits(hAxis, tmp))
                Return CType(tmp, PMDActualPositionUnits)
            End Get
            Set(ByVal value As PMDActualPositionUnits)
                CheckResult(PMDSetActualPositionUnits(hAxis, CType(value, UInt16)))
            End Set
        End Property

        Public ReadOnly Property ActualVelocity() As Int32
            Get
                Dim tmp As Int32
                CheckResult(PMDGetActualVelocity(hAxis, tmp))
                Return tmp
            End Get
        End Property

        Public Function ReadAnalog(ByVal portID As Int16) As UInt16
            Dim tmp As UInt16
            CheckResult(PMDReadAnalog(hAxis, portID, tmp))
            Return tmp
        End Function

        Public Sub GetAnalogCalibration(ByVal channel As PMDAnalogCalibration,
                                        ByRef value As Int16)
            Dim tmp As Int16
            CheckResult(PMDGetAnalogCalibration(hAxis, channel, tmp))
            value = tmp
        End Sub

        Public Sub SetAnalogCalibration(ByVal channel As PMDAnalogCalibration,
                                        ByVal value As Int16)
            CheckResult(PMDSetAnalogCalibration(hAxis, channel, value))
        End Sub

        Public Sub GetAuxiliaryEncoderSource(ByRef mode As PMDAuxiliaryEncoderMode,
                                             ByRef AuxAxis As PMDAxisNumber)
            Dim tmp1 As Byte
            Dim tmp2 As UInt16
            CheckResult(PMDGetAuxiliaryEncoderSource(hAxis, tmp1, tmp2))
            mode = CType(tmp1, PMDAuxiliaryEncoderMode)
            AuxAxis = CType(tmp2, PMDAxisNumber)
        End Sub

        Public Sub SetAuxiliaryEncoderSource(ByVal mode As PMDAuxiliaryEncoderMode,
                                             ByVal AuxAxis As PMDAxisNumber)
            CheckResult(PMDSetAuxiliaryEncoderSource(hAxis, CType(mode, Byte), CType(AuxAxis, UInt16)))
        End Sub


        Public Sub GetAxisOutMask(ByRef sourceAxis As PMDAxisNumber,
                                  ByRef sourceRegister As PMDAxisOutRegister,
                                  ByRef selectionMask As UInt16,
                                  ByRef senseMask As UInt16)
            Dim tmp1 As UInt16
            Dim tmp2 As Byte
            Dim tmp3 As UInt16
            Dim tmp4 As UInt16
            CheckResult(PMDGetAxisOutMask(hAxis, tmp1, tmp2, tmp3, tmp4))
            sourceAxis = CType(tmp1, PMDAxisNumber)
            sourceRegister = CType(tmp2, PMDAxisOutRegister)
            selectionMask = tmp3
            senseMask = tmp4
        End Sub

        Public Sub SetAxisOutMask(ByVal sourceAxis As PMDAxisNumber,
                                  ByVal sourceRegister As PMDAxisOutRegister,
                                  ByVal selectionMask As UInt16,
                                  ByVal senseMask As UInt16)
            CheckResult(PMDSetAxisOutMask(hAxis, CType(sourceAxis, UInt16), CType(sourceRegister, Byte),
                                                  selectionMask, senseMask))
        End Sub


        Public Sub GetBreakpoint(ByRef BreakpointId As Int16,
                                 ByRef SourceAxis As PMDAxisNumber,
                                 ByRef Action As PMDBreakpointAction,
                                 ByRef Trigger As PMDBreakpointTrigger)
            Dim tmp2 As UInt16
            Dim tmp3 As Byte
            Dim tmp4 As Byte
            CheckResult(PMDGetBreakpoint(hAxis, BreakpointId, tmp2, tmp3, tmp4))
            SourceAxis = CType(tmp2, PMDAxisNumber)
            Action = CType(tmp3, PMDBreakpointAction)
            Trigger = CType(tmp4, PMDBreakpointTrigger)
        End Sub

        Public Sub SetBreakpoint(ByVal BreakpointId As Int16,
                                 ByVal SourceAxis As PMDAxisNumber,
                                 ByVal Action As PMDBreakpointAction,
                                 ByVal Trigger As PMDBreakpointTrigger)
            CheckResult(PMDSetBreakpoint(hAxis, BreakpointId, CType(SourceAxis, UInt16),
                                          CType(Action, Byte), CType(Trigger, Byte)))
        End Sub

        Public Sub GetBreakpointUpdateMask(ByVal BreakpointId As Int16,
                                           ByRef Mask As Int16)
            CheckResult(PMDGetBreakpointUpdateMask(hAxis, BreakpointId, Mask))
        End Sub

        Public Sub SetBreakpointUpdateMask(ByVal BreakpointId As Int16,
                                           ByVal Mask As Int16)
            CheckResult(PMDSetBreakpointUpdateMask(hAxis, BreakpointId, Mask))
        End Sub

        Public Function GetBreakpointValue(ByVal BreakpointId As Int16) As Int32
            Dim tmp As Int32
            CheckResult(PMDGetBreakpointValue(hAxis, BreakpointId, tmp))
            Return tmp
        End Function

        Public Sub SetBreakpointValue(ByVal BreakpointId As Int16, ByVal value As Int32)
            CheckResult(PMDSetBreakpointValue(hAxis, BreakpointId, value))
        End Sub

        Public Function ReadBuffer(ByVal BufferId As Int16) As Int32
            Dim tmp As Int32
            CheckResult(PMDReadBuffer(hAxis, BufferId, tmp))
            Return tmp
        End Function

        Public Function ReadBuffer16(ByVal BufferId As Int16) As Int16
            Dim tmp As Int16
            CheckResult(PMDReadBuffer(hAxis, BufferId, tmp))
            Return tmp
        End Function

        Public Sub WriteBuffer(ByVal BufferId As Int16,
                                ByVal value As Int32)
            CheckResult(PMDWriteBuffer(hAxis, BufferId, value))
        End Sub

        Public Function GetBufferLength(ByVal BufferId As Int16) As Int32
            Dim tmp As Int32
            CheckResult(PMDGetBufferLength(hAxis, BufferId, tmp))
            Return tmp
        End Function

        Public Sub SetBufferLength(ByVal BufferId As Int16, ByVal value As Int32)
            CheckResult(PMDSetBufferLength(hAxis, BufferId, value))
        End Sub

        Public Function GetBufferReadIndex(ByVal BufferId As Int16) As Int32
            Dim tmp As Int32
            CheckResult(PMDGetBufferReadIndex(hAxis, BufferId, tmp))
            Return tmp
        End Function

        Public Sub SetBufferReadIndex(ByVal BufferId As Int16, ByVal value As Int32)
            CheckResult(PMDSetBufferReadIndex(hAxis, BufferId, value))
        End Sub

        Public Function GetBufferStart(ByVal BufferId As Int16) As Int32
            Dim tmp As Int32
            CheckResult(PMDGetBufferStart(hAxis, BufferId, tmp))
            Return tmp
        End Function

        Public Sub SetBufferStart(ByVal BufferId As Int16, ByVal value As Int32)
            CheckResult(PMDSetBufferStart(hAxis, BufferId, value))
        End Sub

        Public Function GetBufferWriteIndex(ByVal BufferId As Int16) As Int32
            Dim tmp As Int32
            CheckResult(PMDGetBufferWriteIndex(hAxis, BufferId, tmp))
            Return tmp
        End Function

        Public Sub SetBufferWriteIndex(ByVal BufferId As Int16, ByVal value As Int32)
            CheckResult(PMDSetBufferWriteIndex(hAxis, BufferId, value))
        End Sub

        Public ReadOnly Property BusVoltage() As UInt16
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetBusVoltage(hAxis, tmp))
                Return tmp
            End Get
        End Property

        Public Function GetBusVoltageLimits(ByVal parameter As Int16) As UInt16
            Dim tmp As UInt16
            CheckResult(PMDGetBusVoltageLimits(hAxis, parameter, tmp))
            Return tmp
        End Function

        Public Sub SetBusVoltageLimits(ByVal parameter As Int16, ByVal value As UInt16)
            CheckResult(PMDSetBusVoltageLimits(hAxis, parameter, value))
        End Sub

        Public Sub CalibrateAnalog(ByVal parameter As Int16)
            CheckResult(PMDCalibrateAnalog(hAxis, parameter))
        End Sub

        Public Sub GetCANMode(ByRef NodeId As Byte, ByRef TransmissionRate As PMDCANBaud)
            Dim tmp2 As Byte
            CheckResult(PMDGetCANMode(hAxis, NodeId, tmp2))
            TransmissionRate = CType(tmp2, PMDCANBaud)
        End Sub

        Public Sub SetCANMode(ByVal NodeId As Byte, ByVal TransmissionRate As PMDCANBaud)
            CheckResult(PMDSetCANMode(hAxis, NodeId, CType(TransmissionRate, Byte)))
        End Sub

        Public Property CaptureSource() As PMDCaptureSource
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetCaptureSource(hAxis, tmp))
                Return CType(tmp, PMDCaptureSource)
            End Get
            Set(ByVal value As PMDCaptureSource)
                CheckResult(PMDSetCaptureSource(hAxis, CType(value, UInt16)))
            End Set
        End Property

        Public ReadOnly Property CaptureValue() As Int32
            Get
                Dim tmp As Int32
                CheckResult(PMDGetCaptureValue(hAxis, tmp))
                Return tmp
            End Get
        End Property

        Public ReadOnly Property Checksum() As UInt32
            Get
                Dim tmp As UInt32
                CheckResult(PMDGetChecksum(hAxis, tmp))
                Return tmp
            End Get
        End Property

        Public ReadOnly Property CommandedAcceleration() As Int32
            Get
                Dim tmp As Int32
                CheckResult(PMDGetCommandedAcceleration(hAxis, tmp))
                Return tmp
            End Get
        End Property

        Public ReadOnly Property CommandedPosition() As Int32
            Get
                Dim tmp As Int32
                CheckResult(PMDGetCommandedPosition(hAxis, tmp))
                Return tmp
            End Get
        End Property

        Public ReadOnly Property CommandedVelocity() As Int32
            Get
                Dim tmp As Int32
                CheckResult(PMDGetCommandedVelocity(hAxis, tmp))
                Return tmp
            End Get
        End Property

        Public Property CommutationMode() As PMDCommutationMode
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetCommutationMode(hAxis, tmp))
                Return CType(tmp, PMDCommutationMode)
            End Get
            Set(ByVal value As PMDCommutationMode)
                CheckResult(PMDSetCommutationMode(hAxis, CType(value, UInt16)))
            End Set
        End Property

        Public Function GetCommutationParameter(ByVal parameter As PMDCommutationParameter) As Int32
            Dim tmp As Int32
            CheckResult(PMDGetCommutationParameter(hAxis, parameter, tmp))
            Return tmp
        End Function

        Public Sub SetCommutationParameter(ByVal parameter As PMDCommutationParameter,
                                       ByVal value As Int32)
            CheckResult(PMDSetCommutationParameter(hAxis, parameter, value))
        End Sub

        Public Function GetCurrent(ByVal parameter As PMDCurrent) As UInt16
            Dim tmp As UInt16
            CheckResult(PMDGetCurrent(hAxis, CType(parameter, UInt16), tmp))
            Return tmp
        End Function

        Public Sub SetCurrent(ByVal parameter As PMDCurrent, ByVal value As UInt16)
            CheckResult(PMDSetCurrent(hAxis, CType(parameter, UInt16), value))
        End Sub

        Public Property CurrentControlMode() As PMDCurrentControlMode
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetCurrentControlMode(hAxis, tmp))
                Return CType(tmp, PMDCurrentControlMode)
            End Get
            Set(ByVal value As PMDCurrentControlMode)
                CheckResult(PMDSetCurrentControlMode(hAxis, CType(value, PMDCurrentControlMode)))
            End Set
        End Property

        Public Function GetCurrentFoldback(ByVal parameter As Int16) As UInt16
            Dim tmp As UInt16
            CheckResult(PMDGetCurrentFoldback(hAxis, parameter, tmp))
            Return tmp
        End Function

        Public Sub SetCurrentFoldback(ByVal parameter As Int16, ByVal value As UInt16)
            CheckResult(PMDSetCurrentFoldback(hAxis, parameter, value))
        End Sub

        Public Property CurrentLimit() As Int16
            Get
                Dim tmp As Int16
                CheckResult(PMDGetCurrentLimit(hAxis, tmp))
                Return tmp
            End Get
            Set(ByVal value As Int16)
                CheckResult(PMDSetCurrentLimit(hAxis, value))
            End Set
        End Property

        Public Function GetCurrentLoop(ByVal phase As PMDCurrentLoop,
                                    ByVal parameter As PMDCurrentLoopParameter) As UInt16
            Dim tmp As UInt16
            CheckResult(PMDGetCurrentLoop(hAxis, CType(phase, Byte), CType(parameter, Byte), tmp))
            Return tmp
        End Function

        Public Sub SetCurrentLoop(ByVal phase As PMDCurrentLoop,
                                  ByVal parameter As PMDCurrentLoopParameter,
                                  ByVal value As UInt16)
            CheckResult(PMDSetCurrentLoop(hAxis, CType(phase, Byte), CType(parameter, Byte), value))
        End Sub

        Public Function GetCurrentLoopValue(ByVal phase As PMDCurrentLoop,
                                            ByVal node As PMDCurrentLoopValueNode) As Int32
            Dim tmp As Int32
            CheckResult(PMDGetCurrentLoopValue(hAxis, CType(phase, Byte), CType(node, Byte), tmp))
            Return tmp
        End Function

        Public Property Deceleration() As UInt32
            Get
                Dim tmp As UInt32
                CheckResult(PMDGetDeceleration(hAxis, tmp))
                Return tmp
            End Get
            Set(ByVal value As UInt32)
                CheckResult(PMDSetDeceleration(hAxis, value))
            End Set
        End Property

        ' Note that these are ION defaults, not Prodigy/CME defaults
        ' This should use an enum for the variable argument.
        Public Function GetMPDefault(ByVal variable As Int16) As UInt32
            Dim tmp As UInt32
            CheckResult(PMDGetDefault(hAxis, variable, tmp))
            Return tmp
        End Function

        Public Sub SetMPDefault(ByVal variable As Int16, ByVal value As UInt32)
            CheckResult(PMDSetDefault(hAxis, variable, value))
        End Sub

        Public ReadOnly Property DriveFaultStatus() As UInt16
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetDriveFaultStatus(hAxis, tmp))
                Return tmp
            End Get
        End Property

        Public Sub ClearDriveFaultStatus()
            CheckResult(PMDClearDriveFaultStatus(hAxis))
        End Sub

        Public Function GetDriveCommandMode(ByVal mode As PMDDriveCommandMode)
            Dim tmp As UInt16
            CheckResult(PMDGetDriveCommandMode(hAxis, CType(mode, UInt16), tmp))
            Return tmp
        End Function

        Public Sub SetDriveCommandMode(ByVal mode As PMDDriveCommandMode)
            CheckResult(PMDSetDriveCommandMode(hAxis, CType(mode, UInt16)))
        End Sub

        Public Function GetDriveFaultParameter(ByVal parameter As PMDDriveFaultParameter) As UInt16
            Dim tmp As UInt16
            CheckResult(PMDGetDriveFaultParameter(hAxis, CType(parameter, UInt16), tmp))
            Return tmp
        End Function

        Public Function GetDriveValue(ByVal node As PMDDriveValue) As UInt16
            Dim tmp As UInt16
            CheckResult(PMDGetDriveValue(hAxis, CType(node, UInt16), tmp))
            Return tmp
        End Function

        Public Sub SetDriveFaultParameter(ByVal parameter As PMDDriveFaultParameter, ByVal value As UInt16)
            CheckResult(PMDSetDriveFaultParameter(hAxis, CType(parameter, UInt16), value))
        End Sub

        Public Sub DriveNVRAM(ByVal parameter As PMDNVRAMOption, ByVal value As UInt16)
            CheckResult(PMDDriveNVRAM(hAxis, CType(parameter, UInt16), value))
        End Sub

        Public Function GetDrivePWM(ByVal parameter As PMDDrivePWM) As UInt16
            Dim tmp As UInt16
            CheckResult(PMDGetDrivePWM(hAxis, CType(parameter, UInt16), tmp))
            Return tmp
        End Function

        Public Sub SetDrivePWM(ByVal parameter As PMDDrivePWM, ByVal value As UInt16)
            CheckResult(PMDSetDrivePWM(hAxis, CType(parameter, UInt16), value))
        End Sub

        Public Sub ClearInterrupt()
            CheckResult(PMDClearInterrupt(hAxis))
        End Sub

        Public Sub ClearPositionError()
            CheckResult(PMDClearPositionError(hAxis))
        End Sub

        Public ReadOnly Property DriveStatus() As UInt16
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetDriveStatus(hAxis, tmp))
                Return tmp
            End Get
        End Property

        Public Property EncoderModulus() As UInt16
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetEncoderModulus(hAxis, tmp))
                Return tmp
            End Get
            Set(ByVal value As UInt16)
                CheckResult(PMDSetEncoderModulus(hAxis, value))
            End Set
        End Property

        Public Property EncoderSource() As PMDEncoderSource
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetEncoderSource(hAxis, tmp))
                Return CType(tmp, PMDEncoderSource)
            End Get
            Set(ByVal value As PMDEncoderSource)
                CheckResult(PMDSetEncoderSource(hAxis, CType(value, UInt16)))
            End Set
        End Property

        Public Sub GetEncoderToStepRatio(ByRef counts As UInt16, ByRef steps As UInt16)
            CheckResult(PMDGetEncoderToStepRatio(hAxis, counts, steps))
        End Sub

        Public Sub SetEncoderToStepRatio(ByVal counts As UInt16, ByVal steps As UInt16)
            CheckResult(PMDSetEncoderToStepRatio(hAxis, counts, steps))
        End Sub

        Public Function GetEventAction(ByVal ActionEvent As PMDEventActionEvent) As PMDEventAction
            Dim tmp As UInt16
            CheckResult(PMDGetEventAction(hAxis, CType(ActionEvent, UInt16), tmp))
            Return CType(tmp, PMDEventAction)
        End Function

        Public Sub SetEventAction(ByVal ActionEvent As PMDEventActionEvent, ByVal Action As PMDEventAction)
            CheckResult(PMDSetEventAction(hAxis, CType(ActionEvent, UInt16), CType(Action, UInt16)))
        End Sub

        Public ReadOnly Property EventStatus() As UInt16
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetEventStatus(hAxis, tmp))
                Return tmp
            End Get
        End Property

        Public Sub ResetEventStatus(ByVal StatusMask As UInt16)
            CheckResult(PMDResetEventStatus(hAxis, StatusMask))
        End Sub

        Public Function GetFeedbackParameter(ByVal parameter As PMDFeedbackParameter) As UInt32
            Dim tmp As UInt32
            CheckResult(PMDGetFeedbackParameter(hAxis, CType(parameter, UInt16), tmp))
            Return tmp
        End Function

        Public Sub SetFeedbackParameter(ByVal parameter As PMDFeedbackParameter, ByVal value As UInt32)
            CheckResult(PMDSetFeedbackParameter(hAxis, CType(parameter, UInt16), value))
        End Sub

        Public Function GetFOC(ByVal ControlLoop As PMDFOC,
                               ByVal parameter As PMDFOCParameter) As UInt16
            Dim tmp As UInt16
            CheckResult(PMDGetFOC(hAxis, CType(ControlLoop, Byte), CType(parameter, Byte), tmp))
            Return tmp
        End Function

        Public Sub SetFOC(ByVal ControlLoop As PMDFOC,
                          ByVal parameter As PMDFOCParameter,
                          ByVal value As UInt16)
            CheckResult(PMDSetFOC(hAxis, CType(ControlLoop, Byte), CType(parameter, Byte), value))
        End Sub

        Public Function GetFOCValue(ByVal ControlLoop As PMDFOC,
                                 ByVal node As PMDFOCValueNode) As Int32
            Dim tmp As Int32
            CheckResult(PMDGetFOCValue(hAxis, CType(ControlLoop, Byte), CType(node, Byte), tmp))
            Return tmp
        End Function

        Public Property FaultOutMask() As UInt16
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetFaultOutMask(hAxis, tmp))
                Return tmp
            End Get
            Set(ByVal value As UInt16)
                CheckResult(PMDSetFaultOutMask(hAxis, value))
            End Set
        End Property

        Public Sub GetGearMaster(ByRef MasterAxis As PMDAxisNumber, ByRef source As PMDGearMasterSource)
            Dim tmp1 As UInt16
            Dim tmp2 As UInt16
            CheckResult(PMDGetGearMaster(hAxis, tmp1, tmp2))
            MasterAxis = CType(tmp1, PMDAxisNumber)
            source = CType(tmp2, PMDGearMasterSource)
        End Sub

        Public Sub SetGearMaster(ByVal MasterAxis As PMDAxisNumber, ByVal source As PMDGearMasterSource)
            CheckResult(PMDSetGearMaster(hAxis, CType(MasterAxis, UInt16), CType(source, UInt16)))
        End Sub

        Public Property GearRatio() As Int32
            Get
                Dim tmp As Int32
                CheckResult(PMDGetGearRatio(hAxis, tmp))
                Return tmp
            End Get
            Set(ByVal value As Int32)
                CheckResult(PMDSetGearRatio(hAxis, value))
            End Set
        End Property

        Public Function GetHoldingCurrent(ByVal parameter As PMDHoldingCurrent) As UInt16
            Dim tmp As UInt16
            CheckResult(PMDGetHoldingCurrent(hAxis, CType(parameter, UInt16), tmp))
            Return tmp
        End Function

        Public Sub SetHoldingCurrent(ByVal parameter As PMDHoldingCurrent, ByVal value As UInt16)
            CheckResult(PMDSetHoldingCurrent(hAxis, CType(parameter, UInt16), value))
        End Sub

        Public Sub InitializePhase()
            CheckResult(PMDInitializePhase(hAxis))
        End Sub

        Public ReadOnly Property InstructionError() As PMDInstructionError
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetInstructionError(hAxis, tmp))
                Return CType(tmp, PMDInstructionError)
            End Get
        End Property

        Public ReadOnly Property InterruptAxis() As UInt16
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetInterruptAxis(hAxis, tmp))
                Return tmp
            End Get
        End Property

        Public Property InterruptMask() As UInt16
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetInterruptMask(hAxis, tmp))
                Return tmp
            End Get
            Set(ByVal value As UInt16)
                CheckResult(PMDSetInterruptMask(hAxis, value))
            End Set
        End Property

        Public Function ReadIO(ByVal address As UInt16) As UInt16
            Dim tmp As UInt16
            CheckResult(PMDReadIO(hAxis, address, tmp))
            Return tmp
        End Function

        Public Sub WriteIO(ByVal address As UInt16, ByVal value As UInt16)
            CheckResult(PMDWriteIO(hAxis, address, value))
        End Sub

        Public Property Jerk() As UInt32
            Get
                Dim tmp As UInt32
                CheckResult(PMDGetJerk(hAxis, tmp))
                Return tmp
            End Get
            Set(ByVal value As UInt32)
                CheckResult(PMDSetJerk(hAxis, value))
            End Set
        End Property

        Public Function GetLoop(ByVal ControlLoop As PMDLoop) As Int32
            Dim tmp As Int32
            CheckResult(PMDGetLoop(hAxis, ControlLoop, tmp))
            Return tmp
        End Function

        Public Sub SetLoop(ByVal ControlLoop As PMDLoop,
                          ByVal value As Int32)
            CheckResult(PMDSetLoop(hAxis, ControlLoop, value))
        End Sub

        Public Function GetLoopValue(ByVal ControlLoop As PMDLoop) As Int32
            Dim tmp As Int32
            CheckResult(PMDGetLoopValue(hAxis, ControlLoop, tmp))
            Return tmp
        End Function

        Public Property MotionCompleteMode() As PMDMotionCompleteMode
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetMotionCompleteMode(hAxis, tmp))
                Return CType(tmp, PMDMotionCompleteMode)
            End Get
            Set(ByVal value As PMDMotionCompleteMode)
                CheckResult(PMDSetMotionCompleteMode(hAxis, CType(value, UInt16)))
            End Set
        End Property

        Public Property MotorBias() As Int16
            Get
                Dim tmp As Int16
                CheckResult(PMDGetMotorBias(hAxis, tmp))
                Return tmp
            End Get
            Set(ByVal value As Int16)
                CheckResult(PMDSetMotorBias(hAxis, value))
            End Set
        End Property

        Public Property MotorCommand() As Int16
            Get
                Dim tmp As Int16
                CheckResult(PMDGetMotorCommand(hAxis, tmp))
                Return tmp
            End Get
            Set(ByVal value As Int16)
                CheckResult(PMDSetMotorCommand(hAxis, value))
            End Set
        End Property

        Public Property MotorLimit() As UInt16
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetMotorLimit(hAxis, tmp))
                Return tmp
            End Get
            Set(ByVal value As UInt16)
                CheckResult(PMDSetMotorLimit(hAxis, value))
            End Set
        End Property

        Public Property MotorType() As PMDMotorType
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetMotorType(hAxis, tmp))
                Return CType(tmp, PMDMotorType)
            End Get
            Set(ByVal value As PMDMotorType)
                CheckResult(PMDSetMotorType(hAxis, CType(value, UInt16)))
            End Set
        End Property

        Public Sub MultiUpdate(ByVal AxisMask As UInt16)
            CheckResult(PMDMultiUpdate(hAxis, AxisMask))
        End Sub

        Public Sub NoOperation()
            CheckResult(PMDNoOperation(hAxis))
        End Sub

        Public Property OperatingMode() As UInt16
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetOperatingMode(hAxis, tmp))
                Return tmp
            End Get
            Set(ByVal value As UInt16)
                CheckResult(PMDSetOperatingMode(hAxis, value))
            End Set
        End Property

        Public Property OutputMode() As PMDOutputMode
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetOutputMode(hAxis, tmp))
                Return CType(tmp, PMDOutputMode)
            End Get
            Set(ByVal value As PMDOutputMode)
                CheckResult(PMDSetOutputMode(hAxis, CType(value, UInt16)))
            End Set
        End Property

        Public Property OvertemperatureLimit() As UInt16
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetOvertemperatureLimit(hAxis, tmp))
                Return tmp
            End Get
            Set(ByVal value As UInt16)
                CheckResult(PMDSetOvertemperatureLimit(hAxis, value))
            End Set
        End Property

        Public Property PWMFrequency() As UInt16
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetPWMFrequency(hAxis, tmp))
                Return tmp
            End Get
            Set(ByVal value As UInt16)
                CheckResult(PMDSetPWMFrequency(hAxis, value))
            End Set
        End Property

        Public Property PhaseAngle() As UInt16
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetPhaseAngle(hAxis, tmp))
                Return tmp
            End Get
            Set(ByVal value As UInt16)
                CheckResult(PMDSetPhaseAngle(hAxis, value))
            End Set
        End Property

        Public Function GetPhaseCommand(ByVal phase As PMDPhase) As Int16
            Dim tmp As Int16
            CheckResult(PMDGetPhaseCommand(hAxis, CType(phase, UInt16), tmp))
            Return tmp
        End Function

        Public Property PhaseCorrectionMode() As PMDPhaseCorrectionMode
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetPhaseCorrectionMode(hAxis, tmp))
                Return CType(tmp, PMDPhaseCorrectionMode)
            End Get
            Set(ByVal value As PMDPhaseCorrectionMode)
                CheckResult(PMDSetPhaseCorrectionMode(hAxis, CType(value, UInt16)))
            End Set
        End Property

        Public Property PhaseCounts() As UInt16
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetPhaseCounts(hAxis, tmp))
                Return tmp
            End Get
            Set(ByVal value As UInt16)
                CheckResult(PMDSetPhaseCounts(hAxis, value))
            End Set
        End Property

        Public Property PhaseInitializeMode() As PMDPhaseInitializeMode
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetPhaseInitializeMode(hAxis, tmp))
                Return CType(tmp, PMDPhaseInitializeMode)
            End Get
            Set(ByVal value As PMDPhaseInitializeMode)
                CheckResult(PMDSetPhaseInitializeMode(hAxis, value))
            End Set
        End Property

        Public Property PhaseInitializeTime() As UInt16
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetPhaseInitializeTime(hAxis, tmp))
                Return tmp
            End Get
            Set(ByVal value As UInt16)
                CheckResult(PMDSetPhaseInitializeTime(hAxis, value))
            End Set
        End Property

        Public Property PhaseOffset() As UInt16
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetPhaseOffset(hAxis, tmp))
                Return tmp
            End Get
            Set(ByVal value As UInt16)
                CheckResult(PMDSetPhaseOffset(hAxis, value))
            End Set
        End Property

        Public Function GetPhaseParameter(ByVal parameter As PMDPhaseParameter) As Int32
            Dim tmp As Int32
            CheckResult(PMDGetPhaseParameter(hAxis, parameter, tmp))
            Return tmp
        End Function

        Public Sub SetPhaseParameter(ByVal parameter As PMDPhaseParameter,
                                       ByVal value As Int32)
            CheckResult(PMDSetPhaseParameter(hAxis, parameter, value))
        End Sub

        Public Property PhasePrescale() As PMDPhasePrescale
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetPhasePrescale(hAxis, tmp))
                Return CType(tmp, PMDPhasePrescale)
            End Get
            Set(ByVal value As PMDPhasePrescale)
                CheckResult(PMDSetPhasePrescale(hAxis, CType(value, UInt16)))
            End Set
        End Property

        Public Property Position() As Int32
            Get
                Dim tmp As Int32
                CheckResult(PMDGetPosition(hAxis, tmp))
                Return tmp
            End Get
            Set(ByVal value As Int32)
                CheckResult(PMDSetPosition(hAxis, value))
            End Set
        End Property

        Public ReadOnly Property PositionError() As Int32
            Get
                Dim tmp As Int32
                CheckResult(PMDGetPositionError(hAxis, tmp))
                Return tmp
            End Get
        End Property

        Public Property PositionErrorLimit() As UInt32
            Get
                Dim tmp As UInt32
                CheckResult(PMDGetPositionErrorLimit(hAxis, tmp))
                Return tmp
            End Get
            Set(ByVal value As UInt32)
                CheckResult(PMDSetPositionErrorLimit(hAxis, value))
            End Set
        End Property

        Public Function GetPositionLoop(ByVal parameter As PMDPositionLoop) As Int32
            Dim tmp As Int32
            CheckResult(PMDGetPositionLoop(hAxis, CType(parameter, UInt16), tmp))
            Return tmp
        End Function

        Public Sub SetPositionLoop(ByVal parameter As PMDPositionLoop, ByVal value As Int32)
            CheckResult(PMDSetPositionLoop(hAxis, CType(parameter, UInt16), value))
        End Sub

        Public Function GetPositionLoopValue(ByVal parameter As PMDPositionLoopValueNode) As Int32
            Dim tmp As Int32
            CheckResult(PMDGetPositionLoopValue(hAxis, CType(parameter, UInt16), tmp))
            Return tmp
        End Function

        Public Function GetProductInfo(ByVal parameter As PMDProductInfo) As Int32
            Dim tmp As Int32
            CheckResult(PMDGetProductInfo(hAxis, parameter, tmp))
            Return tmp
        End Function

        Public Property ProfileMode() As PMDProfileMode
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetProfileMode(hAxis, tmp))
                Return CType(tmp, PMDProfileMode)
            End Get
            Set(ByVal value As PMDProfileMode)
                CheckResult(PMDSetProfileMode(hAxis, CType(value, PMDProfileMode)))
            End Set
        End Property

        Public Function GetProfileParameter(ByVal parameter As PMDProfileParameter) As Int32
            Dim tmp As Int32
            CheckResult(PMDGetProfileParameter(hAxis, parameter, tmp))
            Return tmp
        End Function

        Public Sub SetProfileParameter(ByVal parameter As PMDProfileParameter,
                                       ByVal value As Int32)
            CheckResult(PMDSetProfileParameter(hAxis, parameter, value))
        End Sub

        Public Sub RestoreOperatingMode()
            CheckResult(PMDRestoreOperatingMode(hAxis))
        End Sub

        Public ReadOnly Property RuntimeError() As PMDRuntimeError
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetRuntimeError(hAxis, tmp))
                Return CType(tmp, PMDRuntimeError)
            End Get
        End Property

        Public Property SPIMode() As PMDSPIMode
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetSPIMode(hAxis, tmp))
                Return CType(tmp, PMDSPIMode)
            End Get
            Set(ByVal value As PMDSPIMode)
                CheckResult(PMDSetSPIMode(hAxis, CType(value, UInt16)))
            End Set
        End Property

        Public Property SampleTime() As UInt32
            Get
                Dim tmp As UInt32
                CheckResult(PMDGetSampleTime(hAxis, tmp))
                Return tmp
            End Get
            Set(ByVal value As UInt32)
                CheckResult(PMDSetSampleTime(hAxis, value))
            End Set
        End Property

        Public Sub GetSerialPortMode(ByRef baud As PMDSerialBaud,
                                     ByRef parity As PMDSerialParity,
                                     ByRef StopBits As PMDSerialStopBits,
                                     ByRef protocol As PMDSerialProtocol,
                                     ByRef MultiDropId As Byte)
            Dim tmp1 As Byte
            Dim tmp2 As Byte
            Dim tmp3 As Byte
            Dim tmp4 As Byte
            CheckResult(PMDGetSerialPortMode(hAxis, tmp1, tmp2, tmp3, tmp4, MultiDropId))
            baud = CType(tmp1, PMDSerialBaud)
            parity = CType(tmp2, PMDSerialParity)
            StopBits = CType(tmp3, PMDSerialStopBits)
            protocol = CType(tmp4, PMDSerialProtocol)
        End Sub

        Public Sub SetSerialPortMode(ByVal baud As PMDSerialBaud,
                                     ByVal parity As PMDSerialParity,
                                     ByVal StopBits As PMDSerialStopBits,
                                     ByVal protocol As PMDSerialProtocol,
                                     ByVal MultiDropId As Byte)
            CheckResult(PMDSetSerialPortMode(hAxis, CType(baud, Byte), CType(parity, Byte),
                                              CType(StopBits, Byte), CType(protocol, Byte),
                                              MultiDropId))
        End Sub

        Public Property SettleTime() As UInt16
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetSettleTime(hAxis, tmp))
                Return tmp
            End Get
            Set(ByVal value As UInt16)
                CheckResult(PMDSetSettleTime(hAxis, value))
            End Set
        End Property

        Public Property SettleWindow() As UInt16
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetSettleWindow(hAxis, tmp))
                Return tmp
            End Get
            Set(ByVal value As UInt16)
                CheckResult(PMDSetSettleWindow(hAxis, value))
            End Set
        End Property

        Public Property SignalSense() As UInt16
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetSignalSense(hAxis, tmp))
                Return tmp
            End Get
            Set(ByVal value As UInt16)
                CheckResult(PMDSetSignalSense(hAxis, value))
            End Set
        End Property

        Public ReadOnly Property SignalStatus() As UInt16
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetSignalStatus(hAxis, tmp))
                Return tmp
            End Get
        End Property

        Public Property StartVelocity() As UInt32
            Get
                Dim tmp As UInt32
                CheckResult(PMDGetStartVelocity(hAxis, tmp))
                Return tmp
            End Get
            Set(ByVal value As UInt32)
                CheckResult(PMDSetStartVelocity(hAxis, value))
            End Set
        End Property

        ' See documentation for step ranges
        Public Property StepRange() As UInt16
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetStepRange(hAxis, tmp))
                Return tmp
            End Get
            Set(ByVal value As UInt16)
                CheckResult(PMDSetStepRange(hAxis, value))
            End Set
        End Property

        Public Property StopMode() As PMDStopMode
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetStopMode(hAxis, tmp))
                Return CType(tmp, PMDStopMode)
            End Get
            Set(ByVal value As PMDStopMode)
                CheckResult(PMDSetStopMode(hAxis, CType(value, UInt16)))
            End Set
        End Property

        Public Property SynchronizationMode() As PMDSynchronizationMode
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetSynchronizationMode(hAxis, tmp))
                Return CType(tmp, PMDSynchronizationMode)
            End Get
            Set(ByVal value As PMDSynchronizationMode)
                CheckResult(PMDSetSynchronizationMode(hAxis, CType(value, UInt16)))
            End Set
        End Property

        Public ReadOnly Property Temperature() As UInt16
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetTemperature(hAxis, tmp))
                Return tmp
            End Get
        End Property

        Public ReadOnly Property Time() As UInt32
            Get
                Dim tmp As UInt32
                CheckResult(PMDGetTime(hAxis, tmp))
                Return tmp
            End Get
        End Property

        Public ReadOnly Property TraceCount() As UInt32
            Get
                Dim tmp As UInt32
                CheckResult(PMDGetTraceCount(hAxis, tmp))
                Return tmp
            End Get
        End Property

        Public Property TraceMode() As PMDTraceMode
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetTraceMode(hAxis, tmp))
                Return CType(tmp, PMDTraceMode)
            End Get
            Set(ByVal value As PMDTraceMode)
                CheckResult(PMDSetTraceMode(hAxis, CType(value, UInt16)))
            End Set
        End Property

        Public Property TracePeriod() As UInt16
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetTracePeriod(hAxis, tmp))
                Return tmp
            End Get
            Set(ByVal value As UInt16)
                CheckResult(PMDSetTracePeriod(hAxis, value))
            End Set
        End Property

        Public Sub GetTraceStart(ByRef triggerAxis As PMDAxisNumber,
                                 ByRef condition As PMDTraceCondition,
                                 ByRef bit As Byte,
                                 ByRef state As PMDTraceTriggerState)
            Dim tmp1 As UInt16
            Dim tmp2 As Byte
            Dim tmp4 As Byte
            CheckResult(PMDGetTraceStart(hAxis, tmp1, tmp2, bit, tmp4))
            triggerAxis = CType(tmp1, PMDAxisNumber)
            condition = CType(tmp2, PMDTraceCondition)
            state = CType(tmp4, PMDTraceTriggerState)
        End Sub

        Public Sub SetTraceStart(ByVal triggerAxis As PMDAxisNumber,
                                 ByVal condition As PMDTraceCondition,
                                 ByVal bit As Byte,
                                 ByVal state As PMDTraceTriggerState)
            CheckResult(PMDSetTraceStart(hAxis, CType(triggerAxis, UInt16), CType(condition, Byte),
                                          bit, CType(state, Byte)))
        End Sub

        Public ReadOnly Property TraceStatus() As UInt16
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetTraceStatus(hAxis, tmp))
                Return tmp
            End Get
        End Property

        Public Sub GetTraceStop(ByRef triggerAxis As PMDAxisNumber,
                                ByRef condition As PMDTraceCondition,
                                ByRef bit As Byte,
                                ByRef state As PMDTraceTriggerState)
            Dim tmp1 As UInt16
            Dim tmp2 As Byte
            Dim tmp4 As Byte
            CheckResult(PMDGetTraceStop(hAxis, tmp1, tmp2, bit, tmp4))
            triggerAxis = CType(tmp1, PMDAxisNumber)
            condition = CType(tmp2, PMDTraceCondition)
            state = CType(tmp4, PMDTraceTriggerState)
        End Sub

        Public Sub SetTraceStop(ByVal triggerAxis As PMDAxisNumber,
                                ByVal condition As PMDTraceCondition,
                                ByVal bit As Byte,
                                ByVal state As PMDTraceTriggerState)
            CheckResult(PMDSetTraceStop(hAxis, CType(triggerAxis, UInt16), CType(condition, Byte),
                                          bit, CType(state, Byte)))
        End Sub

        Public Sub GetTraceVariable(ByVal VariableNumber As PMDTraceVariableNumber,
                                    ByRef TraceAxis As PMDAxisNumber,
                                    ByRef variable As PMDTraceVariable)
            Dim tmp3 As UInt16
            Dim tmp4 As Byte
            CheckResult(PMDGetTraceVariable(hAxis, CType(VariableNumber, UInt16), tmp3, tmp4))
            TraceAxis = CType(tmp3, PMDAxisNumber)
            variable = CType(tmp4, PMDTraceVariable)
        End Sub

        Public Sub SetTraceVariable(ByVal VariableNumber As PMDTraceVariableNumber,
                                    ByVal TraceAxis As PMDAxisNumber,
                                    ByVal variable As PMDTraceVariable)
            CheckResult(PMDSetTraceVariable(hAxis, CType(VariableNumber, UInt16),
                                            CType(TraceAxis, UInt16), CType(variable, Byte)))
        End Sub

        Public Sub GetTraceValue(ByVal variable As PMDTraceVariable,
                                    ByRef value As Int32)
            Dim tmp3 As UInt16
            CheckResult(PMDGetTraceValue(hAxis, CType(variable, UInt16), tmp3))
            value = CType(tmp3, Int32)
        End Sub

        Public Property TrackingWindow() As UInt16
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetTrackingWindow(hAxis, tmp))
                Return tmp
            End Get
            Set(ByVal value As UInt16)
                CheckResult(PMDSetTrackingWindow(hAxis, value))
            End Set
        End Property

        Public Sub Update()
            CheckResult(PMDUpdate(hAxis))
        End Sub

        Public Property UpdateMask() As UInt16
            Get
                Dim tmp As UInt16
                CheckResult(PMDGetUpdateMask(hAxis, tmp))
                Return tmp
            End Get
            Set(ByVal value As UInt16)
                CheckResult(PMDSetUpdateMask(hAxis, value))
            End Set
        End Property

        Public Property Velocity() As Int32
            Get
                Dim tmp As Int32
                CheckResult(PMDGetVelocity(hAxis, tmp))
                Return tmp
            End Get
            Set(ByVal value As Int32)
                CheckResult(PMDSetVelocity(hAxis, value))
            End Set
        End Property

        Public Sub GetVersion(ByRef family As UInt16,
                                ByRef MotorType As PMDMotorTypeVersion,
                                ByRef NumberAxes As UInt16,
                                ByRef special_and_chip_count As UInt16,
                                ByRef custom As UInt16,
                                ByRef major As UInt16,
                                ByRef minor As UInt16)
            Dim mtype As UInt16
            CheckResult(PMDGetVersion(hAxis, family, mtype, NumberAxes, special_and_chip_count,
                                       custom, major, minor))
            MotorType = CType(mtype, PMDMotorTypeVersion)
        End Sub

    End Class 'Axis

    Public Class PMDMemory
        '*** Private data and utility functions ***
        Friend hMem As IntPtr
        Friend DataSize As PMDDataSize
        Friend status As PMDResult

        Friend Sub CheckResult(ByVal status As PMDResult)
            Me.status = status
            If (Not status = PMDResult.NOERROR) Then
                Dim e As New Exception("ERROR: PMDMemory " & PMDresultString(status))
                e.Data.Add("PMDresult", status)
                Throw e
            End If
        End Sub

        '*** Public Methods ***
        Public Sub New(ByVal device As PMDDevice,
                       ByVal DataSize As PMDDataSize,
                       ByVal MemType As PMDMemoryType)
            hMem = PMDMemoryAlloc()
            If (hMem = 0) Then
                Throw New Exception("ERROR: PMD library: could not allocate memory object")
            End If
            Me.DataSize = DataSize
            CheckResult(PMDDeviceOpenMemory(hMem, device.hDev, DataSize, MemType))
        End Sub

        Protected Overrides Sub Finalize()
            If (Not hMem = 0) Then
                PMDMemoryClose(hMem)
                PMDMemoryFree(hMem)
                hMem = 0
            End If
        End Sub

        Public Sub Close()
            Me.Finalize()
        End Sub

        Public ReadOnly Property LastError() As PMDResult
            Get
                Return status
            End Get
        End Property

        ' NB only 32 bit reads are supported at this time
        Public Sub Read(ByRef data() As UInt32, ByVal offset As UInt32, ByVal length As UInt32)
            CheckResult(PMDMemoryRead(hMem, data(0), offset, length))
        End Sub

        Public Sub Write(ByRef data() As UInt32, ByVal offset As UInt32, ByVal length As UInt32)
            CheckResult(PMDMemoryWrite(hMem, data(0), offset, length))
        End Sub


    End Class 'PMDMemory

End Module