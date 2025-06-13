# Made and created by Hugh Elliott for the purpose of the Design and Control of a Stroke Therapy Device
nIONcon = True              # Turns on/ off serial communication
TEST = False                 # Test functions for send/ receive
ALLDATA = False           # Switches between receiving all data at the end or as it goes
SIMULATION = True      # Turns on/off matlab simulation code
plotOn = True                   # Turns on/off plotting using matplotlib
SENDINLOOP = False      # Turns on/off including the Send function in the main loop vs only calling the Send function when a button is pressed
TRAJFORCE = False            # Turns on/off saving the force components from Assitive Mode Trajectory
TRAJVAR = True              # Saves x0_dot and x0_ddot
NORESAMPLE = False             # If a resampled time is used for Assistive Trajectory Simulation

from tkinter import *
from tkinter import ttk
import time # Used for timing for plot
import matplotlib.pyplot as plt
plt.ion()

window = Tk()

import sys

window.title("Stroke Therapy Device")
window.geometry('800x400')
window.minsize(800, 400)

if(nIONcon): # this will handle communication
    import serial
    NIONCME = serial.Serial(port='COM1', baudrate=115200, parity='N', stopbits=1,  timeout=0.05)

def GetTimeNow():                    #Read the timer function
    return(time.perf_counter())

## Variables

# Entry box variables
posMax = 31.45
posMin = 0
countInput = IntVar()
pos1 = DoubleVar()
pos2 = DoubleVar()
sped = DoubleVar()
torq = DoubleVar()
countInput.set(3)
pos1.set(posMin) # in cm
pos2.set(posMax) # in cm
sped.set(10) # cm/sec
torq.set(10) #for now PWM

# Radio button variables
m = IntVar(window, 0)
sm = IntVar(window, 4)

# GUI Variables
z = 10 ## Button Width
space = 100 ## Button Spacing
s = 20 ## Button Start Value, x
h = 100 ## Button Height, y

lxspace = 90 # Label x spacing
lyspace = 20 # Label y spacing
ly = 200 # Label y start was 150
unLbw = 5 # Unit label width

gz = 0

# Send Variables
M = m.get()
R = countInput.get()
P1 = pos1.get()
P2 = pos2.get()
V = sped.get()
T = torq.get()
#LastM = M

# Saving flags
tSave = 0
pSave = 0
vSave = 0
fSave = 0
mSave = 0
dSave = 0
sSave = 0
curSaving = False
allSaveFlags = 0

# Impedance variable save flags
mImpSave = 0
MdImpSave = 0
DdImpSave = 0
KdImpSave = 0

mSAVED = 1
MdSAVED = 1
DdSAVED = 1
KdSAVED = 1

# Saving arrays
TIME = []
POS = []
VeL = []
force = []
motCom = []
DesVeL = []

# Unit adjusted arrays
TIME2 = []
POS2 = []
VeL2 = []
force2 = []
motCom2 = []
DesVeL2 = []

# Offsets
pOffset = 10000
vOffset = 20000
fOffset = 1000000000
mOffset = 15000

#  Motor scale factor
motScale = 32768.0 / 100

# Figure variables
fig1 = None
fig2 = None
fig3 = None

# Impedance Controller Variables
Md_Imp = 0
Dd_Imp = 0
Kd_Imp = 0
m_Imp = 0

# Impedance Entry Box Variables
mImpEntryVar = DoubleVar()
MdImpEntryVar = DoubleVar()
DdImpEntryVar = DoubleVar()
KdImpEntryVar = DoubleVar()
mImpEntryVar.set(.125)
MdImpEntryVar.set(.05)
DdImpEntryVar.set(15)
KdImpEntryVar.set(45)

# Variables needed for the trajectory force breakdown
if (TRAJFORCE):
    # Save flags
    TFSave = 0
    FSave = 0
    FextSave = 0
    e_termSave = 0
    e_dot_termSave = 0
    TcSave = 0
    TvSave = 0
    aSave = 0

    # Saving Arrays
    TIMEFORCE = []
    FORCE = []
    FEXT = []
    E_TERM = []
    E_DOT_TERM = []
    TC = []
    TV = []
    ACCELERATION = []

    # Unit adjusted arrays
    TIMEFORCE2 = []
    FORCE2 = []
    FEXT2 = []
    E_TERM2 = []
    E_DOT_TERM2 = []
    TC2 = []
    TV2 = []
    ACCELERATION2 = []

if (TRAJVAR):
    # Save flags
    x0_dotSave = 0
    x0_ddotSave = 0

    # Saving Arrays
    X0_DOT = []
    X0_DDOT = []

    # Unit adjusted arrays
    X0_DOT2 = []
    X0_DDOT2 = []
    
def print_dimensions():
    width = window.winfo_width()
    height = window.winfo_height()
    print(f"Width: {width}, Height: {height}")

window.after(100, print_dimensions)

## Button Functions
def Start():
    gg = 0
    start.place_forget()
    Begin()
    loop()
def loop():
    global gz
    if nIONcon:
        while(2>1):
            #Buttons()
            if TEST:
                SendTest()  # Send data
                ReceiveTest()  # Try to receive data
            else:
                if (SENDINLOOP):
                    Send()
                Receive()
                #ReceiveTest() 
            #gz += 1
            #print(gz)
            Prog()
            window.update()
    if(timesent < 10):
        window.after(50, loop)  # Call loop again after 100 ms
def stoP():      ## Stop Button
    global LastM, M
    global sSave
    LastM = M
    m.set(0)
    sm.set(4)
    Buttons()
    M = LastM

    #UpdateVAL()
def Kill():             ## Exit Button
    global M
    if(nIONcon):
        gg = 1
        M = 9
        UpdateVAL()
        NIONCME.close()
    quit()
def Buttons():
    global M
    M = m.get()
    if (m.get() == 4):
        submode_frame.pack(side=LEFT, anchor=N)
        SubFunc()
    else:
        submode_frame.pack_forget()
        print(M)
        if (nIONcon):
            print("Buttons -> UpdateVAL")
            UpdateVAL()
    if (M == 1 or M == 2):
        GoButton.place(x = s+3*lxspace, y = ly+5*lyspace)
    else:
        GoButton.place_forget()

def SubFunc():
    global M
    M = sm.get()
    print(M)
    if (nIONcon):
        UpdateVAL()
def UpdateVAL():
    global M
    global R
    global P1
    global P2
    global V
    global T
    R = int(countInput.get())
    P1 = float(pos1.get())
    P2 = float(pos2.get())
    V = float(sped.get())
    T = float(torq.get())
    if (R < 1):
        R = 1
    if (P1 < posMin):
        P1 = posMin
    elif (P1 >= posMax):
        P1 = posMax - 10
    if (P2 > posMax):
        P2 = posMax
    elif (P2 <=posMin):
        P2 = 5
    if (P2 <= P1):
        P2 = P1 + 5
    if (V < 0):
        V = abs(V)
    if  (T > 100):
        T = 100
    elif (T < 0):
        T = 0
    countInput.set(R)
    pos1.set(P1)
    pos2.set(P2)
    sped.set(V)
    torq.set(T)
    curCountLabel.configure(text = R)
    curPos1Label.configure(text = P1)
    curPos2Label.configure(text = P2)
    curVelLabel.configure(text = V)
    curTorqLabel.configure(text = T)
    Convert()
    if(nIONcon):
        if TEST:
            SendTest()
        else:
            if (not SENDINLOOP):
                print("UpdateVAL -> Send")
                Send()
    if (M == 7):
        if (m.get() == 4):
            M = sm.get()
        else:
            
            M = m.get()
        
def Begin():
    # Place Buttons
    Update.place(x = s+2*lxspace, y = ly+5*lyspace)
    mode_frame.pack(side=LEFT, anchor=N, padx=20)
    StopButton.place(x = s+3*space, y = h)
    #ConvertTest.place(x = s+3*space, y = h+ space)
    # place entry boxes
    countEntry.place(x = s+2*lxspace, y = ly)
    posStartEntry.place(x = s+2*lxspace, y = ly+1*lyspace)
    posEndEntry.place(x = s+2*lxspace, y = ly+2*lyspace)
    velEntry.place(x = s+2*lxspace, y = ly+3*lyspace)
    torqEntry.place(x = s+2*lxspace, y = ly+4*lyspace)
    # place labels
    countLabel.place(x = s, y = ly)
    curCountLabel.place(x = s+lxspace, y = ly)
    posStartLabel.place(x = s, y = ly+lyspace)
    curPos1Label.place(x = s+lxspace, y = ly+lyspace)
    posEndLabel.place(x = s, y = ly+2*lyspace)
    curPos2Label.place(x = s+lxspace, y = ly+2*lyspace)
    velLabel.place(x = s, y = ly+3*lyspace)
    curVelLabel.place(x = s+lxspace, y = ly+3*lyspace)
    torqLabel.place(x = s, y = ly+4*lyspace)
    curTorqLabel.place(x = s+lxspace, y = ly+4*lyspace)
    current.place(x = s+lxspace, y = ly-lyspace)
    countUnitLabel.place(x = s+3*lxspace, y = ly)
    pos1UnitLabel.place(x = s+3*lxspace, y = ly+1*lyspace)
    pos2UnitLabel.place(x = s+3*lxspace, y = ly+2*lyspace)
    velUnitLabel.place(x = s+3*lxspace, y = ly+3*lyspace)
    torqUnitLabel.place(x = s+3*lxspace, y = ly+4*lyspace)
    actCount.place(x = s+4*lxspace, y = ly)
    actCountNum.place(x = s+550, y = ly)
    # Place Impedance Variable Entry Boxes and Labels
    mImpEntry.place(x = 50, y = 375)
    mImpLabel.place(x = 50, y = 350)
    MdImpEntry.place(x = 50+1*75, y = 375)
    MdImpLabel.place(x = 50+1*75, y = 350)
    DdImpEntry.place(x = 50+2*75, y = 375)
    DdImpLabel.place(x = 50+2*75, y = 350)
    KdImpEntry.place(x = 50+3*75, y = 375)
    KdImpLabel.place(x = 50+3*75, y = 350)
    ImpButton.place(x = 50+4*75, y = 375)
    
if TEST:
    data = [0,1,2,3]
else:
    data = [0] * 6
olddata = data.copy()
timesent = 0

def SendTest():
    global data
    global olddata
    global timesent
    try:
        if (olddata != data):
            SendLabels()
            ion = NIONCME.read(NIONCME.in_waiting)  # Read all available bytes
            # Convert the list of integers to a bytearray to send, ensuring all values are within byte range
            if (isinstance(data, str)):
                byte_data = data.encode()
            else:
                byte_data = bytes([i % 256 for i in data])
            #print(f"Data to be sent: {byte_data.hex()}")  # Debugging line to view the data being sent
            nCount = len(byte_data)  # Number of bytes to send
            NIONCME.write(byte_data)  # Send the data via serial port (assuming NIONCME is the peripheral handle)
            olddata = data.copy()
            time.sleep(0.05)  # Allow time for data to be sent
            timesent += 1
            print(f"Data sent ({nCount} bytes): {byte_data.hex()}")
    except Exception as e:
        print(f"Error sending data: {e}")
        
def ReceiveTest():
    #global data
    global olddata
    try:
        if (NIONCME.in_waiting > 0):
            # Read the incoming data from the serial port
            ion = NIONCME.read(NIONCME.in_waiting)  # Read all available bytes
            icount = len(ion)
            print(f"Received {icount} raw bytes: {ion.hex()}")  # Print the raw bytes in hexadecimal
            # Convert the raw bytes into a list of integers
            data2 = [byte for byte in ion]
            # Optional: Add 1 to each byte, as you did in the microcontroller code
            data = [i + 1 for i in data2]         
            # Convert the list of integers into a hexadecimal string for display
            data_str = ' '.join([f"{byte:02x}" for byte in data2])   
            # Update the Test label with the processed data
            Test.configure(text=data_str)
            olddata = [i+1 for i in data]
##            if (ion[0].decode('utf-8') == 'T'):
##                print("Decoded Bytes = T")
#            if (icount != 4):
#                print("Not 4!")
    except Exception as e:
        print(f"Error receiving data: {e}")
        
def SendLabels(): # Used as a test to send labels for data
    global data
    if (timesent == 0):
        data = "M"
    elif (timesent == 1):
        data = "R"
    elif (timesent == 2):
        data = "S"
    elif (timesent == 3):
        data = "E"
    elif (timesent == 4):
        data = "V"
    elif (timesent == 5):
        data = "T"
    else:
        data = "1"
        
def Convert():      # Used to convert GUI numbers to C-Motion numbers
    global data
    global olddata
##    print(f"olddata = {olddata}, data = {data}, first")

    if (olddata == data):
        data[0] = -1
        olddata = data.copy()
    data[0] = M                                    # Mode
    data[1] = R                                     # Repetitions
    p1 = P1 * 10                                   # P1 in cm convert to mm
    data[2] =  round(p1 * (4000/74))            # mm * (counts/mm) Converts mm to encoder counts
    data[3] = round(P2 * 10 * (4000/74))     # Same for P2
    data[4] = round(V * 10 * (4000/74))        # Converts cm/ sec to encoder counts per second
    data[5] = round(T*motScale)                                # Will convert Nm (or something else) to PWM % (don't have the conversion right now)
##    dSize = len(data)
##    lSize = len(dataLab)
##    print(f"Data size = {dSize}")
##    print(f"Label Size = {lSize}")
##    print(f"olddata = {olddata}, data = {data}, second")

def build_packet(dataLab, data):        # Builds a packet with framing and checksum to be sent
    if (len(dataLab) != len(data)):
        raise ValueError("dataLab and data must have the same length")
    
    byte_data = []

    for label, value in zip(dataLab, data):
        label_byte = ord(label)
        byte_data.append(label_byte)
        print(f"Adding character '{label}': {label_byte:02x}")

        if (value > 0xFFFF):  # Checks if value is larger than 2 bytes
            size = 3
            byte_data.append(size)
            high_byte = (value >> 16) & 0xFF
            middle_byte = (value >> 8) & 0xFF
            low_byte = value & 0xFF
            byte_data.extend([high_byte, middle_byte, low_byte])
            print(f"Adding 3-byte value {value}: bytes [{high_byte:02x}, {middle_byte:02x}, {low_byte:02x}]")
        elif (value > 255):  # Check if the value is larger than 1 byte
            size = 2
            byte_data.append(size)
            high_byte = (value >> 8) & 0xFF
            low_byte = value & 0xFF
            byte_data.extend([high_byte, low_byte])
            print(f"Adding value {value}: high byte {high_byte} (0x{high_byte:x}), low byte {low_byte} (0x{low_byte:x})")
        else:
            size = 1
            byte_data.append(size)
            byte_data.append(value)  # Single byte for values 0-255
            print(f"Adding small value {value}: {value:02x}")
    
    flat_byte_data = bytes(byte_data)
    print(f"Flat Data sent ({len(flat_byte_data)} bytes): {flat_byte_data.hex()}")

    return bytes(flat_byte_data)


dataLab = ["M", "R", "S", "E", "V", "T"]
def Send():         # Send Values
    global data
    global olddata
    global nCount

    packet = []
    try:
        if (olddata != data):
            print(f"olddata = {olddata}, data = {data}")
            ion = NIONCME.read(NIONCME.in_waiting)  # Read all available bytes

            packet = build_packet(dataLab, data)
            nCount = len(packet)  # Number of bytes to send

            print(f"Packet to send ({nCount} bytes): {packet.hex()}")  # Show the packet in hex format
            NIONCME.write(packet)  # Send the packet over the serial connection
            olddata = data.copy()
            print()
    except Exception as e:
        print(f"Error sending data: {e}")
    
ion = []
endValue = 0x3e3e3e3e
otherEnd = 0x3e3e3e00
def Receive():
    global tSave
    global pSave
    global vSave
    global fSave
    global mSave
    global dSave
    global sSave
    global allSaveFlags

    global TIME
    global POS
    global VeL
    global force
    global motCom
    global DesVeL

    global TIME2
    global POS2
    global VeL2
    global force2
    global motCom2
    global DesVeL2

    global title, actCountVar

    global mImpSave
    global MdImpSave
    global DdImpSave
    global KdImpSave
    global m_Imp
    global Md_Imp
    global Dd_Imp
    global Kd_Imp
    global mSAVED, MdSAVED, DdSAVED, KdSAVED

    global X0_DOT, X0_DDOT, x0_dotSave, x0_ddotSave

    allSaveFlags = tSave|pSave|vSave|fSave|mSave|dSave|mImpSave|MdImpSave|DdImpSave|KdImpSave
    if(TRAJFORCE):
        allSaveFlags = allSaveFlags|TFSave|FSave|FextSave|e_termSave|e_dot_termSave|TcSave|TvSave|aSave
    if (TRAJVAR):
        allSaveFlags = allSaveFlags|x0_dotSave|x0_ddotSave
    try:
        ion = [0x00]
        if (NIONCME.in_waiting > 0 and allSaveFlags == 0):
            if ((NIONCME.in_waiting == 14 or NIONCME.in_waiting == 15) and NIONCME.in_waiting == 666):
                ion = NIONCME.read(NIONCME.in_waiting)
                icount = len(ion)
                print(f"Received {icount} raw bytes: {ion.hex()}") 
            else:
                ion = NIONCME.read(3)
                #data_str = ion.decode('ascii')
            icount = len(ion)
            rem = icount % 2
##            if (rem == 1):
##                print(f"Rem = {rem}")
            if(ALLDATA):
                b = 1
                #print(f"Received {icount} raw bytes: {ion.hex()}")
            if (ion == b'<T>'):
                tSave = 1
                SaveLABEL(tSave)
                if (ALLDATA):
                    print("ion = <T")
            if (ion == b'<P>'):
                pSave = 1
                if (ALLDATA):
                    print("ion = <P")
            if (ion == b'<V>'):
                vSave = 1
                if (ALLDATA):
                    print("ion = <V")
            if (ion == b'<F>'):
                fSave = 1
                if (ALLDATA):
                    print("ion = <F")
            if (ion == b'<M>'):
                mSave = 1
                if (ALLDATA):
                    print("ion = <M")
            if (ion == b'<D>'):
                dSave = 1
                if (ALLDATA):
                    print("ion = <D")
            if (ion == b'<S>'):
                sSave = 1
                print("ion = <S")
            if (ion == b'<W>'):
                mImpSave = 1
                print("ion = <W")
            if (ion == b'<X>'):
                MdImpSave = 1
                print("ion = <X")
            if (ion == b'<Y>'):
                DdImpSave = 1
                print("ion = <Y")
            if (ion == b'<Z>'):
                KdImpSave = 1
                print("ion = <Z")
            if (TRAJFORCE and M == 77):
                TrajFlag(ion)
            if (TRAJVAR and M == 77):
                if (ion == b'<d>'):
                    x0_dotSave = 1
                if (ion == b'<b>'):
                    x0_ddotSave = 1
        
        if (NIONCME.in_waiting > 3):
            ion = NIONCME.read(4)
            temp = int.from_bytes(ion, byteorder='little')
            #time.sleep(0.02)
            #print(f"temp = {hex(temp)}")
            #print(f"Expected endValue = {hex(endValue)}")
            if (temp == endValue or temp == otherEnd):
                if (temp == otherEnd):
                    dump = NIONCME.read(1)
                    print(f"endValue= {temp:x}, dump = {dump}")
                if (ALLDATA):
                    print(f"endValue, {temp}'")
                if (tSave):
                    tSave = 0
                    #print(f"TIME2 = {TIME2}")
                if (pSave):
                    pSave = 0
                if (vSave):
                    vSave = 0                        
                if (fSave):
                    fSave = 0                      
                if (mSave):
                    mSave = 0                       
                if (dSave):
                    dSave = 0
                if (mImpSave):
                    mImpSave = 0
                if (MdImpSave):
                    MdImpSave = 0
                if (DdImpSave):
                    DdImpSave = 0
                if (KdImpSave):
                    KdImpSave = 0
                if (ALLDATA):
                    icount = len(ion)
                    print(f"Received {icount} raw bytes: {ion.hex()}")
                if (TRAJFORCE and M == 77):
                    TrajFlag(ion)
                if (TRAJVAR and M == 77):
                    if (x0_dotSave):
                        x0_dotSave = 0
                    if (x0_ddotSave):
                        x0_ddotSave = 0
            else:
                if (tSave):
                    TIME.append(temp)
                    tsize = len(TIME)
                    Test.configure(text = f"{tsize}")
                if (pSave):
                    POS.append(temp)
                if (vSave):
                    VeL.append(temp)
                if (fSave):
                    force.append(temp)
                if (mSave):
                    ionT1 = ion[0:2]
                    ionT2 = ion[2:4]
                    #print(f"ionT1 = {ionT1.hex()}, ionT2 = {ionT2.hex()}")
                    temp1 = int.from_bytes(ionT1, byteorder='little')
                    temp2 = int.from_bytes(ionT2, byteorder='little')
                    motCom.append(temp1)
                    if(ALLDATA):
                        motCom.append(temp2)
                if (dSave):
                    DesVeL.append(temp)
                if (mImpSave and mSAVED):
                    m_Imp = temp * .001
                    if (m_Imp != 0):
                        mSAVED = 0
                    print(f"m = {m_Imp}")
                    print(f"{temp}")
                if (MdImpSave and MdSAVED):
                    Md_Imp = temp * .001
                    if (Md_Imp != 0):
                        MdSAVED = 0
                    print(f"Md = {Md_Imp}")
                    print(f"{temp}")
                if (DdImpSave and DdSAVED):
                    Dd_Imp = temp * .001
                    if (Dd_Imp != 0):
                        DdSAVED = 0
                    print(f"Dd = {Dd_Imp}")
                    print(f"{temp}")
                if (KdImpSave and KdSAVED):
                    Kd_Imp = temp * .001
                    if (Kd_Imp != 0):
                        KdSAVED = 0
                    print(f"Kd = {Kd_Imp}")
                    print(f"{temp}")
                if (TRAJFORCE and M == 77):
                    TrajFlag(ion)
                if (TRAJVAR and M == 77):
                    if (x0_dotSave):
                        X0_DOT.append(temp)
                    if (x0_ddotSave):
                        X0_DDOT.append(temp)
                    
            if(ALLDATA):
                icount = len(ion)
                #print(f"Received {icount} raw bytes: {ion.hex()}")            
        if(sSave and curSaving):
            if (TIME == []):
                sSave = 0
                SaveLABEL(sSave)
                return
            # Checks if any data was lost and fixes it
            Sizing()            
            #Converts received data to appropriate units
           # print(f"TIME = {TIME}")
            TIME2 = [(i/1000) for i in TIME]
            #print(f"POS = {POS}")
            POS2 = [(i-pOffset)/(10 * (4000/74)) for i in POS]
            #print(f"VeL = {VeL}")
            VeL2 = [(i-vOffset)/(10 * (4000/74) * .001) for i in VeL]
            #print(f"force = {force}")
            force2 = [(i-fOffset)*.001 for i in force]
            #print(f"motCom = {motCom}")
            motCom2 = [((i-mOffset) / motScale) for i in motCom]
            #print(f"DesVeL = {DesVeL}")
            if (M != 3):
                DesVeL2 = [(i-vOffset)/(10 * (4000/74) * 1) for i in DesVeL]
##                DesVeL2 = [(i-vOffset) for i in DesVeL]
            else:
                DesVeL2 = [.01 * (i-vOffset) for i in DesVeL]
            if (M == 22):
                DesVeL2 = [(i-vOffset) for i in DesVeL]
            if (M == 77):
                DesVeL2 = [((i-vOffset)*.01) for i in DesVeL]
                if (SIMULATION):
                    X0_DOT2 = [((i-vOffset)*.01) for i in X0_DOT]
                    X0_DDOT2 = [((i-vOffset)*.01) for i in X0_DDOT]
            # Opens file and writes data
            #if (LastM != M):
              #  M = LastM
            while (len(TIME2) < len(POS2)):
                TIME2.append(TIME2[-1]+.03)
            if (M == 1):
                title = "Passive"
            elif (M == 2):
                title = "Admittance"
            elif (M == 3):
                title = "Assistive"
            elif (M == 4 or M == 5 or M == 6):
                if (M == 4):
                    title = "Resistive"
                elif (M == 5):
                    title = "Resistive - Push"
                else:
                    title = "Resistive - Pull"
            elif ( M == 99):
                title = "Transparent"
            elif ( M == 22):
                title = "Impedance"
            elif (M == 33):
                title = "Assistive Force"
            elif (M == 77):
                title = "Assistive Trajectory"
            else:
                title = "Title"
            mSize = len(motCom2)
            # Opens and writes to file

####################################################################
            file1 = open('TrapM.m', 'w')
####################################################################
            
            file1.write("clc;" + '\n')
            file1.write("clear;" + '\n')
            file1.write("% clf;" + '\n')
            if (M == 8):
                file1.write("comp = 0;" + '\n')
                file1.write('\n')
                file1.write("velsum = 0;" + '\n')
                file1.write("velcount = 0;" + '\n')
            file1.write(f"time = {TIME2};" + '\n')
            file1.write(f"pos = {POS2};" + '\n')
            file1.write(f"vel = {VeL2};" + '\n')
            file1.write(f"force = {force2};" + '\n')
            file1.write(f"motCom = {motCom2};" + '\n')
            #file1.write(f"%motCom2 size = {mSize}" + '\n')
            if (M == 77):
                file1.write(f"x0 = {DesVeL2};" + '\n')
                if (SIMULATION):
                    file1.write(f"x0_dot = {X0_DOT2};" + '\n')
                    file1.write(f"x0_ddot = {X0_DDOT2};" + '\n')
            else:
                file1.write(f"desvel = {DesVeL2};" + '\n')
            file1.write("vel2 = 0;" + '\n')
            file1.write("d = size(pos);" + '\n')
            file1.write("velApp = 0;" + '\n')
            file1.write("for i = 2:1:d(2)" + '\n')
            file1.write("vel2(i) = (pos(i)-pos(i-1))/(time(i)-time(i-1));" + '\n')
            if (M == 3):
                file1.write("if (motCom(i) > 0)" + '\n')
                file1.write("    velApp(i) = 2*motCom(i)-20;" + '\n')
                file1.write("elseif (motCom(i) == 0)" + '\n')
                file1.write("    velApp(i) = 0;" + '\n')
                file1.write("elseif (motCom(i) >-6)" + '\n')
                file1.write("    velApp(i) = 0;" + '\n')
                file1.write("else" + '\n')
                file1.write("    velApp(i) = 2*motCom(i)+20;" + '\n')
                file1.write("end" + '\n')
            if (M == 8):
                file1.write("if (time(i) < 1.5)" + '\n')
                file1.write("    if (vel(i) >= 5.5)" + '\n')
                file1.write("        velcount = velcount+1;" + '\n')
                file1.write("        velsum = velsum + vel(i);" + '\n')
                file1.write("    end" + '\n')
                file1.write("end" + '\n')
            file1.write("end" + '\n')
            file1.write('\n')
            if (M == 8):
                file1.write("velaverage = velsum/velcount" + '\n')
                file1.write("if (comp == 1)" + '\n')
            file1.write("figure(1)"+'\n')
            file1.write("plot(time, pos, 'LineWidth', 1)"+'\n')
            if (M == 77):
                file1.write("hold on" + '\n')
                file1.write("plot(time, x0, 'LineWidth', 1)"+'\n')
                file1.write("legend('Actual Position', 'x0')"+'\n')
                file1.write("hold off" + '\n')
            file1.write("grid on" + '\n')
            #file1.write("legend('Position (cm)')"+'\n')
            file1.write("xlabel('Time (s)')" + '\n')
            file1.write("ylabel('Position (cm)')" + '\n')
            file1.write(f"title('{title}')" + '\n')
            file1.write("figure(2)"+'\n')
            if (M == 3):
                file1.write("plot(time, vel, time, motCom, 'LineWidth', 1)" + '\n')
                file1.write("legend('Velocity (cm/sec)', 'Motor PWM%')"+'\n')
            elif (M == 33):
                file1.write("plot(time, force, time, motCom, 'LineWidth', 1)" + '\n')
                file1.write("legend('Force (N)', 'Motor PWM%')"+'\n')
            elif (M == 4 or M == 5 or M == 6):
                file1.write("plot(time, pos, time, motCom, time, force, 'LineWidth', 1)" + '\n')
                file1.write("legend('Position (cm)', 'Motor PWM%', 'Force (N')"+'\n')
            elif (M == 8):
                file1.write("plot(time, vel, 'LineWidth', 1)"+'\n')
                file1.write("legend('Velocity (cm/sec)')"+'\n')
            elif(M == 1 or M == 2):
                file1.write("plot(time, vel, time, desvel, 'LineWidth', 1)"+'\n')
                file1.write("legend('Velocity (cm/sec)', 'Desired Velocity')"+'\n')
            elif ( M == 99):
                file1.write("plot(time, vel, time, force, 'LineWidth', 1)"+'\n')
                file1.write("legend('Velocity (cm/sec)', 'Force (N)')"+'\n')
            elif (M == 22):
                file1.write("plot(time, pos, time, motCom, time, force, time, vel, 'LineWidth', 1)"+'\n')
                file1.write("legend('Position (cm)', 'Motor PWM%', 'Force (N)', 'Velocity (cm/sec)')"+'\n')
            elif (M == 77):
                file1.write("plot(time, motCom, time, force, time, vel, 'LineWidth', 1)"+'\n')
                file1.write("legend('Motor PWM%', 'Force (N)', 'Velocity (cm/s)')"+'\n')
            else:
                file1.write("plot(time, vel, 'LineWidth', 1)"+'\n')
                file1.write("legend('Velocity (cm/sec)')"+'\n')
            file1.write("xlabel('Time (s)')" + '\n')
            if (M < 4):
                file1.write("ylabel('Velocity (cm/s)')" + '\n')
            file1.write("grid on" + '\n')
            file1.write(f"title('{title}')" + '\n')
            if (M == 8):
                file1.write("xlim([0 1])" + '\n')
                file1.write("end" + '\n')
            if (SIMULATION and (M == 3 or M == 33 or M == 22)):
                velLegend = "'Experimental- Velocity' , 'Simulation- Velocity', 'Simulation- Voltage'"
                motLegend = "'Experimental- PWM%' , 'Simulation- PWM%', 'Force (N)'"
                if (M == 3):
                    modelname = 'Motor_Compare'
                    desBlock = 'DesiredVelocity'
                    desBlockVar = 'desvel(2)'
                elif (M == 33):
                    modelname = 'AssistForce_Compare'
                    desBlock = 'DesiredForce'
                    desBlockVar = T
                elif (M == 22):
                    modelname = 'Impedance_Compare'
                    velLegend = "'Experimental- Velocity', 'Impedance- Velocity', 'Admittance- Velocity'"
                    motLegend = "'Experimental- PWM%', 'Impedance- PWM%', 'Force (N)', 'Admittance- PWM%'"
                file1.write("figure(3)" + '\n')
                file1.write("volt = (24/100)*motCom;" + '\n')
                file1.write("ave_Time = mean(diff(time));" + '\n')
                file1.write("time_new = 0:ave_Time:max(time);" + '\n')
                file1.write("time_new = time;" + '\n')
                file1.write("VoltIn = timeseries(volt.', time_new);" + '\n')
                file1.write("PosIn = timeseries(pos.', time_new);" + '\n')
                file1.write("VelIn = timeseries(vel.', time_new);" + '\n')
                file1.write("ForceIn = timeseries(force.', time_new);" + '\n')
                file1.write(f"load_system('{modelname}');" + '\n')
                file1.write(f"set_param('{modelname}', 'StopTime', num2str(max(time)));" + '\n')
                #file1.write(f"set_param('{modelname}', 'MinStep', 'auto');" + '\n')
                if (M == 3 or M == 33):
                    file1.write(f"set_param('{modelname}/{desBlock}', 'Value', num2str({desBlockVar}));" + '\n')
                file1.write(f"out = sim('{modelname}.slx');" + '\n')
                #file1.write("yyaxis left" + '\n')
                file1.write("plot(time, vel, 'LineWidth', 1 );" + '\n')
                file1.write("hold on" + '\n')
                if (M == 3 or M == 33):
                    file1.write("plot(out.sim_vel2.Time, out.sim_vel2.Data, 'LineWidth', 1 );" + '\n')
                    if ( M == 3):
                        file1.write("%plot(out.sim_vel.Time, out.sim_vel.Data, 'LineWidth', 1 );" + '\n')
                else:
                    file1.write("plot(out.Imp_Vel.Time, out.Imp_Vel.Data, 'LineWidth', 1 );" + '\n')
                    file1.write("%plot(out.Ad_Vel.Time, out.Ad_Vel.Data, 'LineWidth', 1 );" + '\n')
                file1.write("ylabel('Velocity (cm/s)')" + '\n')
                file1.write("xlabel('Time (s)')" + '\n')
                file1.write(f"title('{title}')" + '\n')
                file1.write(f"legend({velLegend})"+'\n')
                file1.write("grid on" + '\n')
                file1.write("hold off" + '\n')
                file1.write("figure(4)" + '\n')
                #file1.write("yyaxis right" + '\n')
                file1.write("plot(time, motCom, 'LineWidth', 1 )" + '\n')
                file1.write("hold on" + '\n')
                if (M == 3 or M == 33):
                    file1.write("plot(out.sim_mot.Time, out.sim_mot.Data, '--', 'LineWidth', 1 );" + '\n')
                else:
                    file1.write("plot(out.Imp_mot.Time, out.Imp_mot.Data, '--', 'LineWidth', 1 );" + '\n')
                    file1.write("%plot(out.Ad_mot.Time, out.Ad_mot.Data, '--', 'LineWidth', 1 );" + '\n')
                if ( M == 3):
                    file1.write("ylabel('PWM %')" + '\n')
                else:
                    file1.write("plot(time, force, 'LineWidth', 1 );" + '\n')
                file1.write(f"legend({motLegend})"+'\n')
                file1.write("hold off" + '\n')
                file1.write("grid on" + '\n')
                file1.write(f"title('{title}')" + '\n')
                if (M == 22):
                    file1.write("figure(5)" + '\n')
                    file1.write("plot(time, pos, 'LineWidth', 1)"+'\n')
                    file1.write("hold on" + '\n')
                    file1.write("plot(out.Imp_Pos.Time, out.Imp_Pos.Data, '--', 'LineWidth', 1 );" + '\n')
                    file1.write("%plot(out.Ad_Pos.Time, out.Ad_Pos.Data, '--', 'LineWidth', 1 );" + '\n')
                    file1.write("ylabel('Position (cm)')" + '\n')
                    file1.write("xlabel('Time (s)')" + '\n')
                    file1.write(f"title('{title}')" + '\n')
                    file1.write(f"legend('Experimental- Position', 'Impedance- Position', 'Admittance- Position')"+'\n')
                    file1.write("hold off" + '\n')
                    file1.write("grid on" + '\n')
            if (SIMULATION and M == 77):
                modelname = 'AssistTrajectory'
                file1.write("figure(5)" + '\n')
                file1.write("figure(4)" + '\n')
                file1.write("figure(3)" + '\n')
                if (NORESAMPLE): 
                    file1.write("ForceIn = timeseries(force.', time);" + '\n')
                    file1.write("X0In = timeseries(x0.', time);" + '\n')
                    file1.write("X0_DOTIn = timeseries(x0_dot.', time);" + '\n')
                    file1.write("X0_DDOTIn = timeseries(x0_ddot.', time);" + '\n')
                else:
                    file1.write("new_time = 0:0.001:max(time);" + '\n')
                    file1.write("Ftemp = spline(time, force, new_time);" + '\n')
                    file1.write("x0temp = spline(time, x0, new_time);" + '\n')
                    file1.write("x0dottemp = spline(time, x0_dot, new_time);" + '\n')
                    file1.write("x0ddottemp = spline(time, x0_ddot, new_time);" + '\n')
                    file1.write("ForceIn = timeseries(Ftemp.', new_time);" + '\n')
                    file1.write("X0In = timeseries(x0temp.', new_time);" + '\n')
                    file1.write("X0_DOTIn = timeseries(x0dottemp.', new_time);" + '\n')
                    file1.write("X0_DDOTIn = timeseries(x0ddottemp.', new_time);" + '\n')
                file1.write(f"load_system('{modelname}');" + '\n')
                file1.write(f"set_param('{modelname}', 'StopTime', num2str(max(time)));" + '\n')
                file1.write(f"set_param('{modelname}/m', 'Value', num2str({m_Imp}));" + '\n')
                file1.write(f"set_param('{modelname}/Md', 'Value', num2str({Md_Imp}));" + '\n')
                file1.write(f"set_param('{modelname}/Dd', 'Value', num2str({Dd_Imp}));" + '\n')
                file1.write(f"set_param('{modelname}/Kd', 'Value', num2str({Kd_Imp}));" + '\n')
                if (TRAJVAR):
                    file1.write(f"set_param('{modelname}/Impedance Control2/DerivativeVsWorkspace/Use Workspace', 'Value', num2str(1));" + '\n')
                else:
                    file1.write(f"set_param('{modelname}/Impedance Control2/DerivativeVsWorkspace/Use Workspace', 'Value', num2str(0));" + '\n')
                file1.write(f"set_param('AssistTrajectory/Impedance Control2/Gain', 'Gain', num2str(1));" + '\n')
                file1.write(f"out = sim('{modelname}.slx');" + '\n')
                file1.write('\n')
                file1.write("plot(time, pos, 'LineWidth', 1)" + '\n')
                file1.write("hold on" + '\n')
                file1.write("plot(time, x0, 'LineWidth', 1)" + '\n')
                file1.write("plot(out.Pos.Time, out.Pos.Data, 'LineWidth', 1)" + '\n')
                file1.write("legend('Actual Position', 'x0', 'x-sim')" + '\n')
                file1.write("hold off" + '\n')
                file1.write("grid on" + '\n')
                file1.write("xlabel('Time (s)')" + '\n')
                file1.write("ylabel('Position (cm)')" + '\n')
                file1.write("title('Assistive Trajectory')" + '\n')
                file1.write("figure(4)" + '\n')
                file1.write("plot(time, motCom, time, force, out.motCom.Time, out.motCom.Data, 'LineWidth', 1)" + '\n')
                file1.write("legend('Motor PWM%', 'Force (N)', 'PWM%-sim')" + '\n')
                file1.write("xlabel('Time (s)')" + '\n')
                file1.write("grid on" + '\n')
                file1.write("title('Assistive Trajectory')" + '\n')
                file1.write("figure(5)" + '\n')
                file1.write("plot(time, vel, time, x0_dot, out.Vel.Time, out.Vel.Data, 'LineWidth', 1)" + '\n')
                file1.write("legend('Actual Velocity', 'x0-dot', 'Vel-sim')" + '\n')
                file1.write("grid on" + '\n')
                file1.write("xlabel('Time (s)')" + '\n')
                file1.write("title('Assistive Trajectory')" + '\n')      
              
            file1.write("tmax = max(diff(time))" + '\n')
            file1.write("tmin = min(diff(time))" + '\n')
            file1.write("tmean = mean(diff(time))" + '\n')
            if (M == 77):
                file1.write(f"% m = {m_Imp}" + '\n')
                file1.write(f"% Md = {Md_Imp}" + '\n')
                file1.write(f"% Dd = {Dd_Imp}" + '\n')
                file1.write(f"% Kd = {Kd_Imp}" + '\n')
                if (TRAJFORCE):
                    TrajWrite(file1)
            file1.close()
            if (plotOn):
                Plots()
            sSave = 0
            TIME = []
            POS = []
            VeL = []
            force = []                  
            motCom = []
            DesVeL = []
            SaveLABEL(sSave)
            actCountVar = 0
            mSAVED = 1
            MdSAVED = 1
            DdSAVED = 1
            KdSAVED = 1

            X0_DOT = []
            X0_DDOT = []
            
    except Exception as e:
        print(f"Error receiving data: {e}")
        print(f"tSize = {len(TIME)}, pSize = {len(POS)}, vSize = {len(VeL)}, fSize = {len(force)}, mSize = {len(motCom)}, dSize = {len(DesVeL)}")
        print(f"ion = {ion}")
def conVertTest():
    global byte_data
    data[0] = M                                # Mode
    data[1] = R                                 # Repetitions
    data[2] = 0                                 # P1
    data[3] = 17000                          # Same for P2
    data[4] = 20000                          # Converts cm/ sec to encoder counts per second
    data[5] = int(T)

    print("Convert Test")
    ion = NIONCME.read(NIONCME.in_waiting)  # Read all available bytes
    byte_data = []
    for L in range(len(dataLab)):
        byte_data.append(dataLab[L].encode())
        print(f"Adding character '{dataLab[L]}': {dataLab[L].encode()}")
        value = data[L]
        if value > 255:  # Check if the value is larger than 1 byte
            high_byte = (value >> 8) & 0xFF
            low_byte = value & 0xFF
            byte_data.append(bytes([high_byte]))         # High byte (big-endian)
            byte_data.append(bytes([low_byte]))           # Low byte (big-endian)
            print(f"Adding value {value}: high byte {high_byte} ({hex(high_byte)}), low byte {low_byte} ({hex(low_byte)})")

        else:
            byte_data.append(bytes([value]))  # Single byte for values 0-255
            print(f"Adding small value {value}: {bytes([value]).hex()}")
    nCount = len(byte_data)  # Number of bytes to send
    #print(f"Data sent ({nCount} bytes): {byte_data}")
    flat_byte_data = b''.join(byte_data)
    nCount = len(flat_byte_data)
    print(f"Flat Data sent ({nCount} bytes): {flat_byte_data.hex()}")
    if nIONcon:
        if TEST:
            b = 0
        else:
            NIONCME.write(flat_byte_data)
            
def GoFunc():
    global M
    M = 7
    if(nIONcon):
        UpdateVAL()

def Plots():
    global fig1, fig2, ax1, ax2, fig3, ax3

    if fig1 is not None and not plt.fignum_exists(fig1.number):
        fig1 = None
    if fig2 is not None and not plt.fignum_exists(fig2.number):
        fig2 = None

    padding = (max(TIME2) - min(TIME2)) * 0.05
    
    try:
        # Position Plot
        if fig1 is None:
            fig1, ax1 = plt.subplots(num="Position Plot")
##            ax1 = fig1.add_subplot(111)
        else:
            fig1.clf()
            ax1 = fig1.add_subplot(111)
##            ax1.clear()

        if (M == 77):
            ax1.plot(TIME2, POS2, label = 'Actual Position')
            ax1.plot(TIME2, DesVeL2, label = 'x0')
            ax1.legend()
        else:
            ax1.plot(TIME2, POS2)
        ax1.set_title(f"{title}")
        ax1.set_xlabel("Time (s)")
        ax1.set_ylabel("Position (cm)")
        ax1.grid(True)
        ax1.set_xlim(0, max(TIME2)+padding)

        fig1.canvas.draw_idle()
        fig1.canvas.flush_events()

        # Velocity/ Force/ Motor Command Plot
        if fig2 is None:
            fig2, ax2 = plt.subplots(num="Plot 2")
##            ax2 = fig2.add_subplot(111)
        else:
            fig2.clf()
            ax2 = fig2.add_subplot(111)
##            ax2.clear()
            
        ax2.set_title(f"{title}")
        ax2.set_xlabel("Time (s)")
        if (M == 1): # Passive
            ax2.plot(TIME2, VeL2, label = 'Velocity')
            ax2.plot(TIME2, DesVeL2, label = 'Desired Velocity', linewidth = 2)
            ax2.set_ylabel("Velocity (cm/s)")
        elif (M == 3): # Assistive
            ax2.plot(TIME2, VeL2, label = "Velocity (cm/s)")
            ax2.plot(TIME2, motCom2, label = "Motor PWM%")
        elif (M == 33): # Assistive Force
            ax2.plot(TIME2, force2, label = "Force (N)")
            ax2.plot(TIME2, motCom2, label = "Motor PWM%")
        elif (M == 99): # Transparent
            ax2.plot(TIME2, VeL2, label = "Velocity (cm/s)")
            ax2.plot(TIME2, force2, label = "Force (N)")
        elif (M == 22): # Impedance
            ax2.plot(TIME2, POS2, label = "Position (cm)")
            ax2.plot(TIME2, motCom2, label = "Motor PWM%")
            ax2.plot(TIME2, force2, label = "Force (N)")
            ax2.plot(TIME2, VeL2, label = "Velocity (cm/s)")
##            ax2.plot(TIME2, DesVeL2, label = "Control Output")
##            ax2.set_ylabel("Velocity (cm/s)")
        elif (M == 4 or M == 5 or M == 6): # resistive
            ax2.plot(TIME2, POS2, label = 'Position (cm)')
            ax2.plot(TIME2, force2, label = "Force (N)")
            ax2.plot(TIME2, motCom2, label = "Motor PWM%")
        elif ( M == 77):
            ax2.plot(TIME2, motCom2, label = "Motor PWM%")
            ax2.plot(TIME2, force2, label = "Force (N)")
            ax2.plot(TIME2, VeL2, label = "Velocity (cm/s)")

        ax2.legend()    
        ax2.grid(True)
        ax2.set_xlim(0, max(TIME2)+padding)
        #plt.show()

        fig2.canvas.draw_idle()
        fig2.canvas.flush_events()

        if(TRAJFORCE and M == 77):
            try:
                if fig3 is None:
                    fig3, ax3 = plt.subplots(num="Force Plot")
                else:
                    fig3.clf()
                    ax3 = fig3.add_subplot(111)
                ax3.set_title(f"{title}")
                ax3.set_xlabel("Time (s)")
                ax3.set_ylabel("Force (N)")
                ax3.plot(TIMEFORCE2, FORCE2, label = "Control Force")
                ax3.plot(TIMEFORCE2, FEXT2, label = "Fext")
                ax3.plot(TIMEFORCE2, E_TERM2, label = "Position Error")
                ax3.plot(TIMEFORCE2, E_DOT_TERM2, label = "Velocity Error")
                ax3.plot(TIMEFORCE2, TC2, label = "Tcoulomb")
                ax3.plot(TIMEFORCE2, TV2, label = "Tviscous")
                ax3.plot(TIMEFORCE2, ACCELERATION2, label = "x0_ddot")

                ax3.legend()    
                ax3.grid(True)
                trajForcePadding = (max(TIMEFORCE2) - min(TIMEFORCE2)) * 0.05
                ax3.set_xlim(0, max(TIMEFORCE2)+trajForcePadding)

                fig3.canvas.draw_idle()
                fig3.canvas.flush_events()
            except Exception as e:
                print(f"Error plotting figure 3: {e}")

        plt.pause(0.001)
        
    except Exception as e:
        print(f"Error plotting data: {e}")
    
def Sizing():
    # used to suppliment arrays in the event that data is lost in communication
    global TIME, POS, VeL, force, motCom, DesVeL
    
    try:
        Sum = 0
        tSize = len(TIME)
        pSize = len(POS)
        vSize = len(VeL)
        fSize = len(force)
        mSize = len(motCom)
        dSize = len(DesVeL)
        sizeMax = max(tSize, pSize, vSize, fSize, mSize, dSize)
        
        for i in range(1,tSize):
            Sum += int(TIME[i]) - int(TIME[i-1])
        deltaT = Sum/(tSize-1)
        for i in range(1,tSize):
            temp = int(TIME[i]) - int(TIME[i-1])
            if (temp > 3*deltaT):
                TIME[i] = int(TIME[i-1]) + deltaT
        if (tSize < sizeMax and tSize >0):
            for i in range(tSize, sizeMax):
                TIME.append(int(TIME[-1]) + deltaT)
        if (pSize < sizeMax and pSize >0):
            for i in range(pSize, sizeMax):
                POS.append(POS[-1])
        if (vSize < sizeMax and vSize >0):
            for i in range(vSize, sizeMax):
                VeL.append(VeL[-1])
        if (fSize < sizeMax and fSize >0):
            for i in range(fSize, sizeMax):
                force.append(force[-1])
        if (mSize < sizeMax and mSize >0):
            for i in range(mSize, sizeMax):
                motCom.append(motCom[-1])
        if (dSize < sizeMax and dSize >0):
            for i in range(dSize, sizeMax):
                DesVeL.append(DesVeL[-1])
        if (TRAJFORCE and M == 77):
            global TIMEFORCE, FORCE, FEXT, E_TERM, E_DOT_TERM, TC, TV, ACCELERATION
            
            FSize = len(FORCE)
            FESize = len(FEXT)
            ESize = len(E_TERM)
            EDOTSize = len(E_DOT_TERM)
            TCSize = len(TC)
            TVSize = len(TV)
            ACCELSize = len(ACCELERATION)

            if (FSize < sizeMax and FSize >0):
                for i in range(FSize, sizeMax):
                    FORCE.append(FORCE[-1])
            if (FESize < sizeMax and FESize >0):
                for i in range(FESize, sizeMax):
                    FEXT.append(FEXT[-1])
            if (ESize < sizeMax and ESize >0):
                for i in range(ESize, sizeMax):
                    E_TERM.append(E_TERM[-1])
            if (EDOTSize < sizeMax and EDOTSize >0):
                for i in range(EDOTSize, sizeMax):
                    E_DOT_TERM.append(E_DOT_TERM[-1])
            if (TCSize < sizeMax and TCSize >0):
                for i in range(TCSize, sizeMax):
                    TC.append(TC[-1])
            if (TVSize < sizeMax and TVSize >0):
                for i in range(TVSize, sizeMax):
                    TV.append(TV[-1])
            if (ACCELSize < sizeMax and ACCELSize >0):
                for i in range(ACCELSize, sizeMax):
                    ACCELERATION.append(ACCELERATION[-1])
        if (SIMULATION and M == 77):
            global X0_DOT, X0_DDOT

            if (X0_DOT == []):
                X0_DOT.append(0)
            if (X0_DDOT == []):
                X0_DDOT.append(0)
            x0dotSize = len(X0_DOT)
            x0ddotSize = len(X0_DDOT)

            if (x0dotSize < sizeMax and x0dotSize > 0):
                for i in range(x0dotSize, sizeMax):
                    X0_DOT.append(X0_DOT[-1])
            if (x0ddotSize < sizeMax and x0ddotSize > 0):
                for i in range(x0ddotSize, sizeMax):
                    X0_DDOT.append(X0_DDOT[-1])
            
    except Exception as e:
        print(f"Sizing Error: {e}")

impdataLab = ["W", "X", "Y", "Z"]
impdata = [0, 0, 0, 0]
def IMPSEND():      # Sends Impedance Variables
    global t

    impdata[0] = int(mImpEntryVar.get() * 1000)
    impdata[1] = int(MdImpEntryVar.get() * 1000)
    impdata[2] = int(DdImpEntryVar.get() * 1000)
    impdata[3] = int(KdImpEntryVar.get() * 1000)

    packet = []
    try:
        print(f"olddata = {olddata}, data = {data}")
        ion = NIONCME.read(NIONCME.in_waiting)  # Read all available bytes

        packet = build_packet(impdataLab, impdata)
        nCount = len(packet)  # Number of bytes to send

        print(f"Packet to send ({nCount} bytes): {packet.hex()}")  # Show the packet in hex format
        NIONCME.write(packet)  # Send the packet over the serial connection
            
    except Exception as e:
        print(f"Error sending data: {e}")
    print()

def TrajFlag(ion):         # Flags and save values for assitive mode trajectory force breakdown. Used with SendTrajForce on nION side 
    global TFSave, FSave, FextSave, e_termSave, e_dot_termSave, TcSave, TvSave, aSave
    global TIMEFORCE, FORCE, FEXT, E_TERM, E_DOT_TERM, TC , TV, ACCELERATION

    if (len(ion) != 4):
        dataOn = 0
    else:
        dataOn = 1
    if (ion == b'<t>'):
        TFSave = 1
##        print("ion = <t>")
    if (ion == b'<f>'):
        FSave = 1
##        print("ion = <f>")
    if (ion == b'<g>'):
        FextSave = 1
##        print("ion = <g>")
    if (ion == b'<e>'):
        e_termSave = 1
##        print("ion = <e>")
    if (ion == b'<v>'):
        e_dot_termSave = 1
##        print("ion = <v>")
    if (ion == b'<c>'):
        TcSave = 1
##        print("ion = <c>")
    if (ion == b'<w>'):
        TvSave = 1
##        print("ion = <w>")
    if (ion == b'<a>'):
        aSave = 1
##        print("ion = <a>")
    if (dataOn):
        temp = int.from_bytes(ion, byteorder='little')
        if (temp == endValue or temp == otherEnd):
            if (temp == otherEnd):
                    dump = NIONCME.read(1)
                    print(f"endValue= {temp:x}, dump = {dump}")
            if(TFSave):
                TFSave = 0
            if(FSave):
                FSave = 0
            if(FextSave):
                FextSave = 0
            if(e_termSave):
                e_termSave = 0
            if(e_dot_termSave):
                e_dot_termSave = 0
            if(TcSave):
                TcSave = 0
            if(TvSave):
                TvSave = 0
            if(aSave):
                aSave = 0
        else:
            if(TFSave):
                TIMEFORCE.append(temp)
            if(FSave):
                FORCE.append(temp)
            if(FextSave):
                FEXT.append(temp)
            if(e_termSave):
                E_TERM.append(temp)
            if(e_dot_termSave):
                E_DOT_TERM.append(temp)
            if(TcSave):
                TC.append(temp)
            if(TvSave):
                TV.append(temp)
            if(aSave):
                ACCELERATION.append(temp)
        
def TrajWrite(file1):        # Writes values from TrajFlag to matlab script
    
    global TIMEFORCE, FORCE, FEXT, E_TERM, E_DOT_TERM, TC, TV, ACCELERATION
    global TIMEFORCE2, FORCE2, FEXT2, E_TERM2, E_DOT_TERM2, TC2, TV2, ACCELERATION2
    
    TIMEFORCE2 = [(i-1000)*.001 for i in TIME]
##    FORCE2 = [(i-fOffset)/10000 for i in FORCE]
##    FEXT2 = [(i-fOffset)/10000 for i in FEXT]
##    E_TERM2 = [(i-fOffset)/10000 for i in E_TERM]
##    E_DOT_TERM2 = [(i-fOffset)/10000 for i in E_DOT_TERM]
##    TC2 = [(i-fOffset)/10000 for i in TC]
##    TV2 = [(i-fOffset)/10000 for i in TV]
##    ACCELERATION2 = [(i-fOffset)/10000 for i in ACCELERATION]
    FORCE2 = [((i/10000)-200) for i in FORCE]
    FEXT2 = [((i/10000)-200) for i in FEXT]
    E_TERM2 = [((i/10000)-200) for i in E_TERM]
    E_DOT_TERM2 = [((i/10000)-200) for i in E_DOT_TERM]
    TC2 = [((i/10000)-200) for i in TC]
    TV2 = [((i/10000)-200) for i in TV]
    ACCELERATION2 = [((i/10000)-200) for i in ACCELERATION]


##    print(f"TIMEFORCE = {TIMEFORCE2}")
##    print(f"FORCE = {FORCE}")
##    print(f"FORCE2 = {FORCE2}")
##    print(f"FEXT = {FEXT2}")
##    print(f"E_TERM = {E_TERM}")
##    print(f"E_DOT_TERM = {E_DOT_TERM2}")
##    print(f"TC = {TC2}")
##    print(f"TV = {TV2}")
##    print(f"ACCELERATION = {ACCELERATION2}")

    file1.write('\n')
    file1.write(f"TIMEFORCE = {TIMEFORCE2};" + '\n')
    file1.write(f"FORCE = {FORCE2};" + '\n')
    file1.write(f"FEXT = {FEXT2};" + '\n')
    file1.write(f"E_TERM = {E_TERM2};" + '\n')
    file1.write(f"E_DOT_TERM = {E_DOT_TERM2};" + '\n')
    file1.write(f"TC = {TC2};" + '\n')
    file1.write(f"TV = {TV2};" + '\n')
    file1.write(f"ACCELERATION = {ACCELERATION2};" + '\n')
    file1.write('\n')
    file1.write("figure(3)" + '\n')
    file1.write("plot(TIMEFORCE, FORCE, 'LineWidth', 1)" + '\n')
    file1.write("hold on" + '\n')
    file1.write("plot(TIMEFORCE, FEXT, 'LineWidth', 1)" + '\n')
    file1.write("plot(TIMEFORCE, E_TERM, 'LineWidth', 1)" + '\n')
    file1.write("plot(TIMEFORCE, E_DOT_TERM, 'LineWidth', 1)" + '\n')
    file1.write("plot(TIMEFORCE, TC, 'LineWidth', 1)" + '\n')
    file1.write("plot(TIMEFORCE, TV, 'LineWidth', 1)" + '\n')
    file1.write("plot(TIMEFORCE, ACCELERATION, 'LineWidth', 1)" + '\n')
    file1.write("legend('Control Force', 'Fext', 'Position Error', 'Velocity Error', 'Tcoulomb', 'Tviscous', 'x0_ddot', 'Location', 'best')" + '\n')
    file1.write("hold off" + '\n')
    file1.write("grid on" + '\n')
    file1.write("xlabel('Time (s)')" + '\n')
    file1.write("ylabel('Force (N)')" + '\n')
    file1.write("title('Assistive Trajectory')" + '\n')

    TIMEFORCE = []
    FORCE = []
    FEXT = []
    E_TERM = []
    E_DOT_TERM = []
    TC = []
    TV = []
    ACCELERATION = []
    
    
# Buttons
start = Button(text="Start", fg="black", width = z, command = Start)
start.place(x = s+1.5*space, y = h/2)

Exit = Button(text = "Exit", fg="black", width = z, command = Kill)
Exit.place(x = s+4*space, y = h)

Update = Button(text="Update", fg="black", width = z, command = UpdateVAL)

StopButton = Button(text="Stop", fg="black", width = z, command = stoP)

ConvertTest = Button(text="Convert", fg="black", width = z, command = conVertTest)

GoButton = Button(text="Go!", fg="black", width = z, command = GoFunc)

# Radio Buttons
# Initialize Mode radio buttons frame
mode_frame = Frame(window)

Mode = { "Off" : 0,
              "Passive" : 1,
##              "Admittance" : 2,
##              "Assistive" : 3,
##              "Assistive Force" : 33,         
              "Assistive" : 77,
              "Transparent" : 99,
              "Resistive" : 4,
##              "Step Response" : 8,
##              "Load CME" : 66,
##              "Impedance" : 22
         }
for (text, Mode) in Mode.items(): 
    Radiobutton(master = mode_frame, command = Buttons, text = text, variable = m, 
        value = Mode).pack(side = TOP, anchor=W, ipady = 1) # ipady = 1

# Initialize SubMode radio buttons frame
submode_frame = Frame(window)

SubMode = { "Both" : 4,
                    "Push" : 5,
                    "Pull" : 6}
for (text, SubMode) in SubMode.items(): 
    Radiobutton(master = submode_frame, command = SubFunc, text = text, variable = sm, 
        value = SubMode).pack(side=TOP, anchor=W, ipady=1)
#subrdbt = window.Radiobutton()

## Entry Boxes + Labels
# Number of repetitions
countEntry = Entry(fg="black", width=z+2, textvariable = countInput)
countLabel = Label(text = "Repetitions: ", width = z+2, height = 1, fg = "white", bg = "black")
curCountLabel = Label(text = countInput.get(), width = z+2, height = 1, fg = "black", bg = "gainsboro")
countUnitLabel = Label(text = "count", width = unLbw, height = 1, fg = "black")

# Position Start
posStartEntry = Entry(fg="black", width=z+2, textvariable = pos1)
posStartLabel = Label(text = "Start Position: ", width = z+2, height = 1, fg = "white", bg = "black")
curPos1Label = Label(text = pos1.get(), width = z+2, height = 1, fg = "black", bg = "gainsboro")
pos1UnitLabel = Label(text = "(cm)", width = unLbw, height = 1, fg = "black")

# Position End
posEndEntry = Entry(fg="black", width=z+2, textvariable = pos2)
posEndLabel = Label(text = "End Position: ", width = z+2, height = 1, fg = "white", bg = "black")
curPos2Label = Label(text = pos2.get(), width = z+2, height = 1, fg = "black", bg = "gainsboro")
pos2UnitLabel = Label(text = "(cm)", width = unLbw, height = 1, fg = "black")

# Velocity
velEntry = Entry(fg="black", width=z+2, textvariable = sped)
velLabel = Label(text = "Speed: ", width = z+2, height = 1, fg = "white", bg = "black")
curVelLabel = Label(text = sped.get(), width = z+2, height = 1, fg = "black", bg = "gainsboro")
velUnitLabel = Label(text = "(cm/s)", width = unLbw, height = 1, fg = "black")

# Torque for Resistive
torqEntry = Entry(fg="black", width=z+2, textvariable = torq)
torqLabel = Label(text = "Resistive Torque: ", width = z+2, height = 1, fg = "white", bg = "black")
curTorqLabel = Label(text = torq.get(), width = z+2, height = 1, fg = "black", bg = "gainsboro")
torqUnitLabel = Label(text = "(PWM)", width = unLbw, height = 1, fg = "black")

# Current Value Label
current = Label(text = "Set Values: ", width = z+2, height = 1, fg = "black")

## Other Labels
# Test label
Test = Label(text = "Test", width = 2*z+2, height = 1, fg = "black")
Test.place(x = 400, y = 50)
# Save Label
SaveLabel = Label(width = z+2, height = 1)
SaveLabel.place(x = 200, y = 100)
def SaveLABEL(Save):
    global curSaving
    if (Save == 1):
        SaveLabel.configure(text = "Saving!", fg = "black", bg = "yellow")
        actCountNum.configure(text = actCountVar)
        curSaving = True
    else:
        SaveLabel.configure(text = "Saved!", fg = "black", bg = "lime")
        curSaving = False

# Counter labels
actCount = Label(text = "Current Count:", width = 4*z+2, height = 1, fg = "black")
actCountNum = Label(text = "0", width = 1, height = 1, fg = "black")
actCountVar = 0
        
# Progress bar
pbar1val = DoubleVar()
progressBar1 = ttk.Progressbar(variable = pbar1val, maximum = posMax, length = 350)
progressBar1.place(x= 400, y = 250)
pbar1val.set(posMax)
ProgLabelLeft = Label(text = "Left Side", width = 6, height = 1)
ProgLabelLeft.place(x = 375, y = 250+22)
ProgLabelRight = Label(text = "Right Side", width = z+1, height = 1)
ProgLabelRight.place(x = 705, y = 250+22)
atMaxPos = False

progLastUpdate = GetTimeNow()
def Prog():
    global progLastUpdate
    global pbar1val
    global atMaxPos, actCountVar

    z = 0
    if (GetTimeNow() - progLastUpdate > 0.1):
        if (len(POS) != 0):
            z = (POS[-1]-10000)/(10 * (4000/74))
        pbar1val.set(posMax - z)
        progLastUpdate = GetTimeNow()
        if (posMax - z < 0.25):
            atMaxPos = True
        if (posMax - z > (posMax - 0.25) and atMaxPos):
            actCountVar += 1
            atMaxPos = False
            actCountNum.configure(text = actCountVar)

## Impedance Variables Items
mImpEntry = Entry(fg="black", width=z+2, textvariable = mImpEntryVar)
mImpLabel = Label(text = "m: ", width = z, height = 1, fg = "black")

MdImpEntry = Entry(fg="black", width=z+2, textvariable = MdImpEntryVar)
MdImpLabel = Label(text = "Md: ", width = z, height = 1, fg = "black")

DdImpEntry = Entry(fg="black", width=z+2, textvariable = DdImpEntryVar)
DdImpLabel = Label(text = "Dd: ", width = z, height = 1, fg = "black")

KdImpEntry = Entry(fg="black", width=z+2, textvariable = KdImpEntryVar)
KdImpLabel = Label(text = "Kd: ", width = z, height = 1, fg = "black")

ImpButton = Button(text="Update", fg="black", width = z, command = IMPSEND)
