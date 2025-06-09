//********************************************************
// PMDRPperiph.cpp
// PMD Prodigy/CME Resource Protocol wrapper class implementation
//********************************************************

#include "PMDtypes.h"
#include "PMDsys.h"
#include "PMDperiph.h"
#include "PMDPfunc.h"
#include "PMDRPperiph.h"


PMDMutexDefine(g_MutexRemote);
PMDMutexDefine(g_MutexCOM);
PMDMutexDefine(g_MutexCAN);
PMDMutexDefine(g_MutexSPI);


// max size of PRP response packet including the PRP status byte
#define MAX_RECEIVE_PACKET_LENGTH          USER_PACKET_LENGTH + 1


//********************************************************
// generic RP protocol wrapper class
PMDRPperiph::PMDRPperiph(PMDPeriphHandle* hPeriph)
{
    m_hPeriph = hPeriph;
    m_Address = 0;
}

void PMDRPperiph::SetAddress(PMDparam address)
{
    m_Address = address;
}

void PMDRPperiph::WriteByte(BYTE byte)
{
    PMDPeriphSend(m_hPeriph, &byte, 1, RP_TIMEOUT);
}

PMDresult PMDRPperiph::SendPacket(BYTE* pbuff, PMDparam nCount)
{
    return PMDPeriphSend(m_hPeriph, pbuff, nCount, RP_TIMEOUT);
}

PMDresult PMDRPperiph::ReceivePacket(BYTE* pbuff, PMDparam nMaxCount, PMDparam timeout, PMDparam* nReceived)
{
    return PMDPeriphReceive(m_hPeriph, pbuff, (PMDparam*)nReceived, nMaxCount, timeout);
}

PMDresult PMDRPperiph::SendReceivePacket(PMDparam timeout, BYTE* pdatain, BYTE* pdataout, PMDparam* nbytes)
{
    PMDresult result;

    result = SendPacket(pdatain, *nbytes);
    if (result == PMD_ERR_OK)
        result = ReceivePacket(pdataout, MAX_RECEIVE_PACKET_LENGTH, timeout, nbytes);

    return result;
}

//********************************************************
// Remote PRP peripheral 
PMDRPperiphRemote::PMDRPperiphRemote(PMDPeriphHandle* hPeriph) : PMDRPperiph(hPeriph)
{
    if (g_MutexRemote == NULL)
      g_MutexRemote = PMDMutexCreate();
}

PMDresult PMDRPperiphRemote::Open()
{
    PMDresult result;
    // Get the device handle attached to the peripheral
    PMDDeviceHandle* phDevice = (PMDDeviceHandle*)m_hPeriph->transport_data;
    // Get the PMDRPperiph object attached to the device handle
    PMDResourceProtocol* rp = (PMDResourceProtocol*)phDevice->transport_data; 
    result = rp->OpenRPDevice((int*)&m_Address, (int)m_hPeriph->address);

    return result;
}

PMDresult PMDRPperiphRemote::SendReceivePacket(PMDparam timeout, BYTE* pdatain, BYTE* pdataout, PMDparam* nbytes)
{
    PMDresult result;
    // Get the device handle attached to the peripheral
    PMDDeviceHandle* phDevice = (PMDDeviceHandle*)m_hPeriph->transport_data;
    // Get the PMDRPperiph object attached to the device handle
    PMDResourceProtocol* rp = (PMDResourceProtocol*)phDevice->transport_data; 

    if (!PMDMutexLock(g_MutexRemote))
      return PMD_ERR_MutexTimeout;

    PMDparam nCount = *nbytes;
    result = rp->RPCommand(m_Address, pdatain, pdataout, nCount, RP_TIMEOUT + timeout);
    *nbytes = rp->GetNumberBytesRead();

    PMDMutexUnlock(g_MutexRemote);

    return result;
}


//********************************************************
// COM port protocol is slightly different
PMDRPperiphCOM::PMDRPperiphCOM(PMDPeriphHandle* hPeriph) : PMDRPperiph(hPeriph)
{
    if (g_MutexCOM == NULL)
      g_MutexCOM = PMDMutexCreate();
}

PMDresult PMDRPperiphCOM::SendPacket(BYTE* pbuff, PMDparam nByteCount)
{
    BYTE bMultidrop = (BYTE)(m_hPeriph->param >> 8);
    BYTE RS485address = (BYTE)m_hPeriph->param;
    BYTE checksum = 0;
    BYTE packetlength = (BYTE)nByteCount;
    BYTE COMpacket[TOTAL_PACKET_LENGTH + 8];
    PMDparam i;

    PMDPCOM_FlushRecv(m_hPeriph);

    if (nByteCount == 0 || nByteCount > TOTAL_PACKET_LENGTH)
        return PMD_ERR_RP_PacketLength;

    for (i=0; i<nByteCount; i++)
    {
        checksum += pbuff[i];
    }

    // form the packet to send.
    i = 0;
    if (bMultidrop)
        COMpacket[i++] = RS485address;
    COMpacket[i++] = checksum;
    COMpacket[i++] = packetlength;
    memcpy(&COMpacket[i], pbuff, nByteCount);

    return PMDPeriphSend(m_hPeriph, COMpacket, nByteCount+i, RP_TIMEOUT);
}

PMDresult PMDRPperiphCOM::ReceivePacket(BYTE* pbuff, PMDparam nMaxCount, PMDparam timeout, PMDparam* nReceived)
{
    PMDresult result = PMD_ERR_OK;
    BYTE  bMultidrop = (BYTE)(m_hPeriph->param >> 8);
    BYTE  RS485address = (BYTE)m_hPeriph->param;
    BYTE  checksum = 0;
    BYTE  calcchecksum = 0;
    PMDparam packetlength = 0;
    PMDparam nBytesReceived = 0;
    PMDparam nBytesExpected = 2;
    BYTE header[4];
    BYTE* pData = pbuff;
    PMDparam i = 0;

    if (bMultidrop)
    {
        nBytesExpected++;
        i = 1;
    }
    result = PMDPeriphReceive(m_hPeriph, &header, &nBytesReceived, nBytesExpected, timeout);
    if (result == PMD_ERR_OK)
    {
        checksum = header[i++];
        packetlength = header[i++];
        if (packetlength > 0 && packetlength <= nMaxCount)
        {
            nBytesExpected = packetlength;
            result = PMDPeriphReceive(m_hPeriph, pData, &nBytesReceived, nBytesExpected, timeout);
            if (result == PMD_ERR_OK)
            {
                for (i=0; i<nBytesReceived; i++)
                {
                    calcchecksum += pData[i];
                }
                *nReceived = nBytesReceived;
				if (calcchecksum != checksum)
					result = PMD_ERR_RP_Checksum;
				// return invalid address AFTER absorbing the packet to keep things in sync.
				if ((bMultidrop) && (header[0] != RS485address))
					result = PMD_ERR_InvalidAddress;
            }
        }
        else 
            result = PMD_ERR_RP_InvalidPacket;
    }

  return result;
}

PMDresult PMDRPperiphCOM::SendReceivePacket(PMDparam timeout, BYTE* pdatain, BYTE* pdataout, PMDparam* nbytes)
{
    PMDresult result;

    if (!PMDMutexLock(g_MutexCOM))
      return PMD_ERR_MutexTimeout;

    result = SendPacket(pdatain, *nbytes);
    if (result == PMD_ERR_OK)
        result = ReceivePacket(pdataout, MAX_RECEIVE_PACKET_LENGTH, timeout, nbytes);

    PMDMutexUnlock(g_MutexCOM);
    PMDTaskWait(0); // Yield to any waiting tasks.

    return result;
}


//*********************************************************************************
// The CAN port protocol includes a sequence number as the first byte
PMDRPperiphCAN::PMDRPperiphCAN(PMDPeriphHandle* hPeriph) : PMDRPperiph(hPeriph)
{
    if (g_MutexCAN == NULL)
      g_MutexCAN = PMDMutexCreate();
}

PMDresult PMDRPperiphCAN::SendPacket(BYTE* pbuff, PMDparam nByteCount)
{
    int nSequence;
    int nContinued;
    int nPacketSize;
    BYTE CANpacket[64];
    const BYTE* pDataTx;
    PMDErrorCode result;
    int maxbytes = 8;
    int PacketDataSize;

    // Flush any data that may have been received erroneously prior to sending a new command.
//  PMDPCAN_FlushReceive(m_hPeriph);
    
    if (nByteCount == 0 || nByteCount > TOTAL_PACKET_LENGTH)
        return PMD_ERR_RP_PacketLength;

    // if data baud is non-zero we are in FD mode.
    if (m_hPeriph->param & 0xF000)
        maxbytes = 64;

    pDataTx = pbuff;
    PacketDataSize = maxbytes - 1;
    nContinued = (nByteCount-1) / PacketDataSize;
    nSequence = 1;
    CANpacket[0] = (char)nContinued | 0x80;
    while (nContinued-- > 0)
    {
        memcpy(&CANpacket[1], pDataTx, PacketDataSize);
        result = PMDPeriphSend(m_hPeriph, CANpacket, maxbytes, RP_TIMEOUT);
        if (PMD_ERR_OK != result)
            return result;
        pDataTx += PacketDataSize;
        CANpacket[0] = (char)nSequence++;
    }
    {
        nPacketSize = (nByteCount-1) % PacketDataSize + 1;
        if (nPacketSize > 0)
        {
            memcpy(&CANpacket[1], pDataTx, nPacketSize);
            nPacketSize += 1;
            result = PMDPeriphSend(m_hPeriph, CANpacket, nPacketSize, RP_TIMEOUT);
        }
    }
    return result;
}

PMDresult PMDRPperiphCAN::ReceivePacket(BYTE* pbuff, PMDparam nMaxCount, PMDparam timeout, PMDparam* nReceived)
{
    int nSequence;
    int nExpectedSequence;
    int nContinued;
    BYTE CANpacket[64];
    BYTE* pDataRx = &pbuff[0];
    PMDparam bInitialMessage;
    PMDparam nBytesReceived = 0;
    PMDparam nBytesExpected = 1; // We will receive 1-8 bytes in each PMDPeriphReceive call.
    PMDparam nBytesReceivedTotal = 0;
    PMDErrorCode result;

    result = PMDPeriphReceive(m_hPeriph, CANpacket, &nBytesReceived, nBytesExpected, timeout);
    if (PMD_ERR_OK != result)
        return result;

    if (nBytesReceived == 0)
        return PMD_ERR_InsufficientDataReceived;

    bInitialMessage = CANpacket[0] >> 7;
    nContinued = CANpacket[0] & 0x7F;
    if (bInitialMessage)
    {
        nBytesReceived--; // skip header byte
        // copy CAN packet to command packet
        memcpy(pDataRx, &CANpacket[1], nBytesReceived);
        pDataRx += nBytesReceived;
        nBytesReceivedTotal += nBytesReceived;
        nExpectedSequence = 1;
        while (nContinued-- > 0)
        {
            result = PMDPeriphReceive(m_hPeriph, CANpacket, &nBytesReceived, nBytesExpected, RP_TIMEOUT);
            if (PMD_ERR_OK == result)
            {
                if (nBytesReceived == 0)
                    return PMD_ERR_InsufficientDataReceived;

                bInitialMessage = CANpacket[0] >> 7;
                if (bInitialMessage)
                {
                    return PMD_ERR_RP_InvalidPacket;
                }
                nSequence = CANpacket[0] & 0x7F;
                if (nSequence != nExpectedSequence)
                {
                    return PMD_ERR_RP_InvalidPacket;
                }
                nExpectedSequence++;
                nBytesReceived--; // skip header byte
                memcpy(pDataRx, &CANpacket[1], nBytesReceived);
                pDataRx += nBytesReceived;
                nBytesReceivedTotal += nBytesReceived;
            }
            else
                break;
        }
    }

    *nReceived = nBytesReceivedTotal;

    return result;
}

PMDresult PMDRPperiphCAN::SendReceivePacket(PMDparam timeout, BYTE* pdatain, BYTE* pdataout, PMDparam* nbytes)
{
    PMDresult result;

    if (!PMDMutexLock(g_MutexCAN))
      return PMD_ERR_MutexTimeout;

    result = SendPacket(pdatain, *nbytes);
    if (result == PMD_ERR_OK)
        result = ReceivePacket(pdataout, MAX_RECEIVE_PACKET_LENGTH, timeout, nbytes);

    PMDMutexUnlock(g_MutexCAN);
    PMDTaskWait(0); // Yield to any waiting tasks.

    return result;
}


//*********************************************************************************
// The CANFD port protocol includes the bytes remaining as the first word of each CAN frame.
PMDRPperiphCANFD::PMDRPperiphCANFD(PMDPeriphHandle* hPeriph) : PMDRPperiphCAN(hPeriph)
{
}


// CAN packet format for PRP commands
// CAN packets (max 8 bytes)
//              | nContinued | up to 7 data bytes |
//              | Sequence#  | up to 7 data bytes |
// CANFD packets (max 64 bytes) 
//             | bytes remaining | up to 63 data bytes |
// where bytes remaining is the #bytes remaining to be sent or receieved in the entire PRP packet which can consist of several CANFD packets
// A length field is necessary because the CAN FD large packets DLC values are not evenly incremental (0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64).
PMDresult PMDRPperiphCANFD::SendPacket(BYTE* pbuff, PMDparam nByteCount)
{
    PMDErrorCode result;
    BYTE CANpacket[64];
    const BYTE* pDataTx;
    int maxbytes = 64;
    int BytesRemaining = nByteCount;
    int PacketSize;
    int PacketDataSize;
    int PacketDataOffset = 2; //array start index to where the data gets copied to

    // Flush any data that may have been received erroneously prior to sending a new command.
//  PMDPCAN_FlushReceive(m_hPeriph);

    if (nByteCount == 0 || nByteCount > TOTAL_PACKET_LENGTH)
        return PMD_ERR_RP_PacketLength;

    pDataTx = pbuff;

    *(WORD*)&CANpacket[0] = (WORD)BytesRemaining | 0x8000;

    while (BytesRemaining > 0)
    {
        PacketDataSize = min(maxbytes - PacketDataOffset, BytesRemaining);
        PacketSize = PacketDataSize + PacketDataOffset;
        memcpy(&CANpacket[PacketDataOffset], pDataTx, PacketDataSize);
        result = PMDPeriphSend(m_hPeriph, CANpacket, PacketSize, RP_TIMEOUT);
        if (PMD_ERR_OK != result)
            return result;
        pDataTx += PacketDataSize;
        BytesRemaining -= PacketDataSize;
        *(WORD*)&CANpacket[0] = BytesRemaining;
    }
    return result;
}

PMDresult PMDRPperiphCANFD::ReceivePacket(BYTE* pbuff, PMDparam nMaxCount, PMDparam timeout, PMDparam* nReceived)
{
    BYTE CANpacket[64];
    BYTE* pDataRx = &pbuff[0];
    PMDparam bInitialMessage;
    PMDparam BytesReceived = 0;
    PMDparam BytesExpected = 1; // We will receive 1-64 bytes in each PMDPeriphReceive call.
    PMDparam BytesReceivedTotal = 0;
    PMDparam BytesExpectedTotal = 0;
    PMDparam BytesRemaining = 0;
    PMDErrorCode result;

    result = PMDPeriphReceive(m_hPeriph, CANpacket, &BytesReceived, BytesExpected, timeout);
    if (PMD_ERR_OK != result)
        return result;

    if (BytesReceived == 0)
        return PMD_ERR_InsufficientDataReceived;

    BytesRemaining = *(WORD*)&CANpacket[0];
    bInitialMessage = (WORD)BytesRemaining & 0x8000;
    BytesRemaining = BytesRemaining & 0x7FFF;
    BytesExpectedTotal  = BytesRemaining;

    if (bInitialMessage)
    {
        BytesReceived -= 2; // skip length word 
        // copy CAN packet to command packet
        memcpy(pDataRx, &CANpacket[2], BytesReceived);
        pDataRx += BytesReceived;
        BytesReceivedTotal += BytesReceived;
        while (BytesReceivedTotal < BytesExpectedTotal)
        {
            result = PMDPeriphReceive(m_hPeriph, CANpacket, &BytesReceived, BytesExpected, RP_TIMEOUT);
            if (PMD_ERR_OK == result)
            {
                if (BytesReceived == 0)
                    return PMD_ERR_InsufficientDataReceived;

                bInitialMessage = (WORD)BytesRemaining & 0x8000;
                if (bInitialMessage)
                {
                    return PMD_ERR_RP_InvalidPacket;
                }
                BytesReceived -= 2; // skip length word 
                memcpy(pDataRx, &CANpacket[2], BytesReceived);
                pDataRx += BytesReceived;
                BytesReceivedTotal += BytesReceived;
                BytesRemaining = *(WORD*)&CANpacket[0];
            }
            else
                break;
        }
    }
    else
        result = PMD_ERR_RP_InvalidPacket;

//    BytesReceivedTotal += PACKET_PADDING_BYTES; // calling function expects header length to be 4 bytes
    *nReceived = BytesReceivedTotal;

    return result;
}

PMDRPperiphTCP::PMDRPperiphTCP(PMDPeriphHandle* hPeriph) : PMDRPperiph(hPeriph)
{
}

PMDresult PMDRPperiphTCP::SendPacket(BYTE* pbuff, PMDparam nCount)
{
    return PMDPeriphSend(m_hPeriph, pbuff, nCount, RP_TIMEOUT);
}

PMDresult PMDRPperiphTCP::ReceivePacket(BYTE* pbuff, PMDparam nMaxCount, PMDparam timeout, PMDparam* nReceived)
{
    PMDresult result;
    BYTE* pdata = pbuff;

    // pbuff is pointing 3 bytes into the packet buffer in PMDRP.cpp to accomodate the padding bytes.
    // PRP over TCP includes the 3 padding bytes in the response so reset the pointer to the start of the buffer.

    pdata -= PACKET_PADDING_BYTES;

    result = PMDPeriphReceive(m_hPeriph, pdata, nReceived, nMaxCount, timeout);

    if (*nReceived > 3)
        *nReceived -= 3;

    return result;
}

PMDresult PMDRPperiphTCP::SendReceivePacket(PMDparam timeout, BYTE* pdatain, BYTE* pdataout, PMDparam* nbytes)
{
    PMDresult result;

    result = SendPacket(pdatain, *nbytes);
    if (result == PMD_ERR_OK)
        result = ReceivePacket(pdataout, MAX_RECEIVE_PACKET_LENGTH, timeout, nbytes);

    return result;
}

PMDRPperiphSPI::PMDRPperiphSPI(PMDPeriphHandle* hPeriph) : PMDRPperiph(hPeriph)
{
    if (g_MutexSPI == NULL)
      g_MutexSPI = PMDMutexCreate();
}

#define SPIDataType             char
#define SPIDataTypeSize		sizeof(SPIDataType)

static PMDresult PMDSPI_WaitUntilReady(PMDPeriphHandle* hPeriph, PMDparam timeoutms, int bIsActive)
{
    unsigned long EndTime = PMDDeviceGetTickCount() + timeoutms;
    unsigned long CurrentTime;
    PMDuint16 DIOstate;
    int bActive;

    // If no PMDPIO_DIOn specified as HostSPIStatus then just return
    if ((hPeriph->param) == 0)
        return PMD_ERR_OK;
    
    do {
        PMDPeriphRead(hPeriph, &DIOstate, 0, 0);
        // HostSPIStatus is active low 
        // hPeriph->param is the PMDPIO_DIOn to read
        bActive = !(DIOstate & (hPeriph->param & 0xFF));
        if (bIsActive == bActive)
            return PMD_ERR_OK;
        CurrentTime = PMDDeviceGetTickCount();
    } while (CurrentTime < EndTime);


    return PMD_ERR_Timeout;
}

/*
    The checksum is the logical not of the 16 bit ones complement checksum of the entire packet, but excluding the header.  
    The ones complement checksum computed on the packet words should be 0xFFFF.  
*/
static WORD CalculateChecksum(WORD* pbuff, int datacount)
{
    int i;
    int nWords = datacount / 2;
    DWORD checksum = 0;
    
    for (i=0; i<nWords; i++)
    {
        checksum += pbuff[i];
    }
    do {
        checksum = (checksum & 0xFFFF) + ((checksum >> 16) & 0xFFFF);
    } while (checksum & 0xFFFF0000);

    return (WORD)~checksum;
}

/* SPI send packet format
   Outgoing SPI packet in memory (PRP payload should be 32-bit aligned)
   |                      32-bits                      |                      32-bits
   ------------------------- --------------------------|---------------------------------------------------|           --------------------------- 
   |   byte 0   |   byte 1   |   byte 2   |   byte 3   |   byte 4   |   byte 5   |   byte 6   |   byte 7   |           |   byte n   |   byte n+1 |  
   |     NU     |     NU     |   length   |    rsvd    |        PRP header       | subaction  | subcommand |  payload  |         checksum        |
   ------------------------- --------------------------|---------------------------------------------------|           ---------------------------

The length is the number of bytes in the PRP packet to follow.  
The packet is always sent as a sequence of 16 bit words when using the SPI transport.  
If the number of bytes is odd then the last byte (the high byte of the last word) is a zero pad.

                        bytecount rsvd PRPstatus+command subaction subcommand  checksum       <- bytecount 0 0 PRPstatus data checksum
    8-bit CMENoOperation      02     00     60    00                           9D    FF       <- 01 00 00 40 FE BF
   16-bit CMENoOperation         0002         0060                               FF9D         <- 01 00 00 40 FE BF
    8-bit CMEGetVersion       04     00     6A    00         01        00      90    FF       <- 05 00 00 40 17 00 00 00 sum 
   16-bit CMEGetVersion          0004         006A               0001            FF90         <- 05 00 00 40 17 00 00 00 sum
*/

static const char NullBuffer[TOTAL_PACKET_LENGTH] = {0};

PMDresult PMDRPperiphSPI::SendPacket(BYTE* pbuff, PMDparam nByteCount)
{
    PMDresult result = PMD_ERR_OK;
    WORD checksum = 0;
    PMDparam datacount = nByteCount;
    PMDparam i;

    if (nByteCount == 0 || nByteCount > TOTAL_PACKET_LENGTH)
        return PMD_ERR_RP_PacketLength;

    i = 0;
    SPIpacket[0] = (BYTE)nByteCount;
    SPIpacket[1] = 0;
    int j = 2;
    for (i=0; i<nByteCount; i++)
    {
        SPIpacket[j] = pbuff[i];  //copy pbuff to SPIpacket
        j++;
    }
    // add padding byte if byte count is odd.
    if (nByteCount % 2)
    {
        datacount++;
        j++;
    }
    datacount += 2; // 2 bytes for the header which is included in the checksum
    checksum = CalculateChecksum((WORD*)&SPIpacket[0], datacount);
    *(WORD*)&SPIpacket[j] = checksum;
    datacount += 2; // 2 bytes for the checksum

    // Wait for HostSPIStatus signal to go inactive after previous command.
    result = PMDSPI_WaitUntilReady(m_hPeriph, 2, 0);
    
    if (result == PMD_ERR_OK)
        result = PMDPeriphSend(m_hPeriph, SPIpacket, datacount, RP_TIMEOUT);
    
    return result;
}

/* SPI receive packet format
   -----------------------------------------------------           --------------------------- 
   |   byte 0   |   byte 1   |   byte 2   |   byte 3   |           |   byte n   |   byte n+1 | 
   |   length   |    rsvd    |      0     | PRP status |  payload  |         checksum        |
   -----------------------------------------------------           ---------------------------
*/
PMDresult PMDRPperiphSPI::ReceivePacket(BYTE* pbuff, PMDparam nMaxCount, PMDparam timeout, PMDparam* nReceived)
{
    PMDresult result = PMD_ERR_OK;
    WORD  checksum = 0;
    WORD  calculatedchecksum = 0;
    PMDparam packetlength = 0;
    PMDparam nBytesReceived = 0;
    PMDparam nBytesExpected = 2; // first 2 bytes contain the header
    PMDparam datacount;
    BYTE* pData = &SPIpacket[0];

    result = PMDSPI_WaitUntilReady(m_hPeriph, timeout, 1);
      
    // The send performs the bi-directional transaction. The receive retrieves the data from a buffer that was filled during the send.
    if (result == PMD_ERR_OK)
        result = PMDPeriphSend(m_hPeriph, NullBuffer, nBytesExpected, timeout);
    // Retrieve the packetlength and SPIStatus bits in the first 2 bytes
    if (result == PMD_ERR_OK)
        result = PMDPeriphReceive(m_hPeriph, pData, &nBytesReceived, nBytesExpected, timeout);
    if (result == PMD_ERR_OK)
    {
        packetlength = pData[0]; // first byte is the length
        if (packetlength > 0 && packetlength <= nMaxCount)
        {
            nBytesExpected = packetlength;
            nBytesExpected++;    // include front pad byte
            nBytesExpected += 2; // checksum is not included in packetlength
            if (nBytesExpected & 1)
              nBytesExpected++;    // include any trailing pad byte
            if (nBytesExpected > LINK_PACKET_LENGTH - 2)
              return PMD_ERR_UnexpectedDataReceived;
            // Send null data in order to receive
            if (result == PMD_ERR_OK)
                result = PMDPeriphSend(m_hPeriph, NullBuffer, nBytesExpected, timeout);
            result = PMDPeriphReceive(m_hPeriph, &pData[2], &nBytesReceived, nBytesExpected, timeout);
            if (result == PMD_ERR_OK)
            {
                datacount = nBytesReceived;
                checksum = *(WORD*)&pData[datacount];
                calculatedchecksum = CalculateChecksum((WORD*)pData, datacount);
                if (calculatedchecksum != checksum)
                    result = PMD_ERR_RP_Checksum;
//                datacount += PACKET_PADDING_BYTES; // calling function expects return header to start at 4th byte
                datacount -= 2; // exclude trailing checksum
                *nReceived = datacount;
                memcpy(pbuff, &pData[3], datacount);
            }
        }
        else 
            result = PMD_ERR_RP_InvalidPacket;
    }

   
    return result;
}

PMDresult PMDRPperiphSPI::SendReceivePacket(PMDparam timeout, BYTE* pdatain, BYTE* pdataout, PMDparam* nbytes)
{
    PMDresult result;

    if (!PMDMutexLock(g_MutexSPI))
      return PMD_ERR_MutexTimeout;

    result = SendPacket(pdatain, *nbytes);
    if (result == PMD_ERR_OK)
        result = ReceivePacket(pdataout, MAX_RECEIVE_PACKET_LENGTH, timeout, nbytes);

    PMDMutexUnlock(g_MutexSPI);
    PMDTaskWait(0); // Yield to any waiting tasks.

    return result;
}


// C++ wrapper functions for calling Packet functions from C code when accessing a remote PRP device
#if defined(__cplusplus)
extern "C" {
#endif

PMDresult SendReceivePacket(PMDDeviceHandle* pDevice, PMDparam timeout, BYTE* pdatain, BYTE* pdataout, PMDparam* nbytes)
{
    return ((PMDRPperiph*)(pDevice->transport_data))->SendReceivePacket(timeout, pdatain, pdataout, nbytes); 
}

#if defined(__cplusplus)
}
#endif
