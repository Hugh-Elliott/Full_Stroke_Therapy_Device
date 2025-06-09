///////////////////////////////////////////////////////////////////////////
//
//	Program.cs -- PMD C# example program
//
//	Performance Motion Devices, Inc.
//
//  This example demonstrates Magellan communications.
//
//  Dependencies:
//      PMDLibrary.dll
//      C-Motion.dll
///////////////////////////////////////////////////////////////////////////


using System;
using static PMDLibrary.PMD;

class Example
{
    PMDPeripheral periph;
    PMDDevice device;

    static void Main(string[] args)
    {
        Example ex = new Example();
        ex.Run();
    }
    public void Run()
    {
        PMDInterfaceType interfacetype = PMDInterfaceType.CAN;

        try
        {
            bool bPRP = true;
            if (bPRP)
            {
                // connect to a PMD CME product via the PMD resource protocol (PRP) over Ethernet.
                if (interfacetype == PMDInterfaceType.TCP)
                {
                    String ipaddress = "192.168.2.2";
                    periph = new PMDPeripheralTCP(System.Net.IPAddress.Parse(ipaddress), 40100, 1000);
                    Console.WriteLine("Connected to {0}", ipaddress);
                }
                else if (interfacetype == PMDInterfaceType.CAN)
                {
                    uint nodeid = 0;
                    periph = new PMDPeripheralCANFD(PMDCANPort.CANPort1, (uint)PMDCANBaud.CANBaud1000000, 0x600 + nodeid, 0x580 + nodeid, 0);
                }
                else if (interfacetype == PMDInterfaceType.Serial)
                {
                    periph = new PMDPeripheralSerial(0, 57600, PMDSerialParity.None, PMDSerialStopBits.SerialStopBits1);
                }
                device = new PMDDevice(periph, PMDDeviceType.ResourceProtocol);
            }
            else
            {
                // connect to a non-CME PMD product via PMD's Motion IC protocol over the COM1 serial interface.
                periph = new PMDPeripheralSerial(0, 57600, PMDSerialParity.None, PMDSerialStopBits.SerialStopBits1);
//                device = new PMDDevice(periph, PMDDeviceType.MotionProcessor);
                device = new PMDDevice(periph, PMDDeviceType.ResourceProtocol);
            }
            UInt32 value;
            value = device.GetInfo(PMDDeviceInfo.CMEVersion, 0);
            Console.WriteLine("CME version: {0:D}.{1:D}", (value >> 16) & 0xFF, value & 0xFF);
            value = device.UserCodeVersion();
            value = device.UserCodeChecksum();
            string filename = device.GetUserCodeName();
            string filedate = device.GetUserCodeDate();
            Console.WriteLine("User code file name: " + filename);
            Console.WriteLine("User code file date: " + filedate);


            PMDAxis axis1 = new PMDAxis(device, PMDAxisNumber.Axis1);

            Console.WriteLine("Getting Actual Position...");
            Int32 pos;
            // C-Motion procedures returning a single value become class properties, and may be
            // retrieved or set by using an assignment.  The "Get" or "Set" part of the name is dropped.
            pos = axis1.ActualPosition;

            Console.WriteLine("Actual Position = {0}", pos);

            // The following line sets the actual position of the axis to zero.
            axis1.ActualPosition = 0;

            // connect to remote CAN device via the Expansion CAN port.
            bool bConnectRemote = false;
            if (bConnectRemote)
            {
                uint nodeid = 1;
                var Node0Can = new PMDPeripheralCANFD(PMDCANPort.CANPort1, (uint)PMDCANBaud.CANBaud1000000, 0x600 + nodeid, 0x580 + nodeid, 0);
                var Node1Device = new PMDDevice(Node0Can, PMDDeviceType.ResourceProtocol);
                var Node1Axis = new PMDAxis(Node1Device, PMDAxisNumber.Axis1);
                var Node1Position = Node1Axis.Position;
                Console.WriteLine("Node #{0} Actual Position = {1}", nodeid, Node1Position);
                Node1Axis.Close();
                Node1Device.Close();
                Node0Can.Close();
            }

            axis1.Close();
            device.Close();
            periph.Close();
        }
        catch (Exception e)
        {
            Console.WriteLine(e.Message);
        }
    }
}

