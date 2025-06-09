/*

  Multitask.c
  
  This example demonstrates multitasking features such as Mailboxes and Mutexes

  Performance Motion Devices, Inc.

*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

// Uncomment this line to see how disabling mutexes affect the output of the MutexExample tasks.
//#define NOT_USING_MUTEXES

#include "c-motion.h"
#include "PMDdevice.h"
#include "PMDperiph.h"
#include "PMDsys.h"
#include "PMDdiag.h"


#define MAJOR_VERSION 1
#define MINOR_VERSION 0

USER_CODE_VERSION(MAJOR_VERSION,MINOR_VERSION)

void MailboxConsumer( void* pvTaskParam ); 
void MailboxProducer( void* pvTaskParam ); 
void MutexExample1( void* pvTaskParam ); 
void MutexExample2( void* pvTaskParam ); 
void GPAnalogInput( void* pvTaskParam );
void TaskInfo( void* pvTaskParam );

char* TaskStates[] = {
    "Invalid     ",
    "NoCode      ",
    "Not Started ",
    "Running     ",
    "Aborted     ",
    "Error       ",
};

typedef struct tagMailObject
{
    int dataTx;
    int dataRx;
    char string1[20];
} MailObject;

PMDMailboxHandle hMailbox1;
PMDMutexHandle hMutex;
PMDPeriphHandle hPeriphPIO;

// The sole purpose of the main task is to start sub tasks based on the TaskParam bit map that is passed in from the CMEStartTask command
USER_CODE_TASK( Multitask )
{
    PMDresult result;
    int tasknumber;
    size_t stacksize = 200;
    PMDTaskPriority priority = PMDTaskPriority_Normal;
    void* parameter = NULL;
    // The task parameter is a 32-bit number used to indicate which tasks to start.
    PMDuint32 taskswitches = TaskParam;

    hMutex = PMDMutexCreate();
    PMDPeriphOpenPIO(&hPeriphPIO, NULL, 0, 0, PMDDataSize_16Bit);

    PMDprintf("\r\n");
    PMDprintf("Task switches = %X\r\n", taskswitches);


    if (taskswitches & 0x100)
    {
        tasknumber = PMDTaskCreate(TaskInfo, "TaskInfo", stacksize, parameter, priority);
        PMDprintf("Created TaskInfo task #%d\r\n", tasknumber);
    }

    if (taskswitches & 1)
    {
        tasknumber = PMDTaskCreate(MailboxProducer, "MBProducer", stacksize, parameter, priority);
        PMDprintf("Created MailboxProducer task #%d\r\n", tasknumber);
    }

    if (taskswitches & 2)
    {
        tasknumber = PMDTaskCreate(MailboxConsumer, "MBConsumer", stacksize, parameter, priority);
        PMDprintf("Created MailboxConsumer task #%d\r\n", tasknumber);
    }

    PMDMutexLock(hMutex);
    if (taskswitches & 4)
    {
        tasknumber = PMDTaskCreate(MutexExample1, "Mut1", stacksize, parameter, priority);
        PMDprintf("Created MutexExample1 task #%d\r\n", tasknumber);
    }

    if (taskswitches & 8)
    {
        tasknumber = PMDTaskCreate(MutexExample2, "Mut2", stacksize, parameter, priority);
        PMDprintf("Created MutexExample2 task #%d\r\n", tasknumber);
    }
    PMDMutexUnlock(hMutex);

    if (taskswitches & 0x10)
    {
        tasknumber = PMDTaskCreate(GPAnalogInput, "GPAnalogInput", stacksize, parameter, priority);
        PMDprintf("Created GPAnalogInput task #%d\r\n", tasknumber);
    }

    PMDTaskAbort(0);
}

void TaskInfo( void* pvTaskParam )
{
    PMDresult result;
    PMDint32 RemainingStack;
    PMDint32 StackSize;
    PMDint32 State;
    PMDint32 AbortCode;
    int tasknumber;
    PMDint32 Name;

    for (;;)
    {
        PMDMutexLock(hMutex);
        for (tasknumber=1; tasknumber<5; tasknumber++)
        {
            State = PMDTaskState_NotStarted;
            PMD_RESULT(PMDTaskGetInfo(NULL, tasknumber, PMDTaskInfo_State, &State));
            if (result == PMD_ERR_OK && State != PMDTaskState_NotStarted)
            {
                PMD_RESULT(PMDTaskGetInfo(NULL, tasknumber, PMDTaskInfo_StackSize, &StackSize));
                PMD_RESULT(PMDTaskGetInfo(NULL, tasknumber, PMDTaskInfo_StackRemaining, &RemainingStack));
                PMD_RESULT(PMDTaskGetInfo(NULL, tasknumber, PMDTaskInfo_AbortCode, &AbortCode));
                PMD_RESULT(PMDTaskGetInfo(NULL, tasknumber, PMDTaskInfo_Name, &Name));
                //PMDprintf("Task #%d State: %d, Stack size = %d, Remaining stack space = %d, \r\n", tasknumber, StackSize, RemainingStack);
                PMDprintf("Task #%d State: %11s\r\n", tasknumber, TaskStates[State]);
                PMDprintf("Task #%d Stack size = %d, \r\n", tasknumber, StackSize);
                PMDprintf("Task #%d Remaining stack space = %d, \r\n", tasknumber, RemainingStack);
                PMDprintf("Task #%d AbortCode: %d\r\n", tasknumber, AbortCode);
                PMDprintf("Task #%d Name: %c%c%c%c\r\n", tasknumber, Name >> 0 & 0xFF, Name >> 8 & 0xFF, Name >> 16 & 0xFF, Name >> 24 & 0xFF);
            }
        }
        PMDMutexUnlock(hMutex);
        PMDTaskWait(1000);
    }
}

void MailboxProducer( void* pvTaskParam )
{
    PMDresult result;
    PMDuint32 timeout = 4000;
    MailObject mail;
    int mailboxId1 = 1;
    int mailboxId2 = 2;
    int depth = 5;
    int itemsize = sizeof(MailObject);
    int tasknumber = PMDTaskGetNumber();

    mail.dataTx = 1;
    mail.dataRx = 1;
    strcpy(mail.string1, "Producer.");

    PMDprintf("Starting MailboxProducer task\r\n");

    PMD_ABORTONERROR(PMDMailboxOpen(&hMailbox1, NULL, mailboxId1, depth, itemsize));

    for (int i=0; i<9; i++)
    {
        PMD_RESULT(PMDMailboxSend(&hMailbox1, &mail, timeout))
        if (result == PMD_ERR_OK)
        {
            PMDMutexLock(hMutex);
            PMDprintf("Producer -> Tx = %3d, Rx = %3d, string %s\r\n", mail.dataTx, mail.dataRx, mail.string1 );
            PMDMutexUnlock(hMutex);
        }
        mail.dataTx++;
        PMDTaskWait(1000);
    }
    PMDTaskAbort(tasknumber);
}

void MailboxConsumer( void* pvTaskParam )
{
    PMDresult result;
    PMDuint32 timeout = 5000;
    MailObject mail;
    MailObject mailPeek;
    int mailboxId1 = 1;
    int mailboxId2 = 2;
    int depth = 2;
    int itemsize = sizeof(MailObject);
    int tasknumber = PMDTaskGetNumber();

    mail.dataTx = 0;
    mail.dataRx = 0;
    strcpy(mail.string1, "Consumer.");

    PMDprintf("Starting MailboxConsumer task\r\n");

    for (int i=0; i<11; i++)
    {
        PMD_RESULT(PMDMailboxPeek(&hMailbox1, &mailPeek, timeout));
        PMD_RESULT(PMDMailboxReceive(&hMailbox1, &mail, timeout));
        if (mailPeek.dataTx != mail.dataTx)
            PMDprintf("Error! Peek failed.\r\n");
        if (result == PMD_ERR_OK)
        {
            PMDMutexLock(hMutex);
            PMDprintf("<- Consumer Tx = %3d, Rx = %3d, string %s\r\n", mail.dataTx, mail.dataRx, mail.string1 );
            mail.dataRx++;
            PMDMutexUnlock(hMutex);
        }
        PMDTaskWait(100);
    }
    PMDTaskAbort(tasknumber);
}

PMDuint32 waittime = 300;

void MutexExample1( void* pvTaskParam )
{
    bool bSuccess;
    PMDuint16 GreenLED = PMDLEDBlinkRate_3Hz33Percent;
    int tasknumber = PMDTaskGetNumber();

    PMDprintf("MutexExample1 is task #%d\r\n", PMDTaskGetNumber() );

    for (int i=0; i<9; i++)
    {
        // if mutexes are disabled the output will be interspersed with MutexExample2 output
        bSuccess = PMDMutexLockEx(hMutex, 4000);
        PMDprintf("\r\n");
        if (bSuccess)
            PMDprintf("MutexExample1 locked\r\n" );
        else
            PMDprintf("MutexExample1 timeout\r\n" );
        // Set Green LED 
        PMDPeriphOut(&hPeriphPIO, PMDPIO_LED_GREEN, PMDLEDBlinkRate_On);
        PMDprintf("   MutexExample1 " );
        PMDTaskWait(waittime);
        PMDprintf("MutexExample1 " );
        PMDTaskWait(waittime);
        PMDprintf("MutexExample1 " );
        PMDTaskWait(waittime);
        PMDprintf("MutexExample1 " );
        PMDTaskWait(waittime);
        PMDMutexUnlock(hMutex);
        PMDTaskWait(0);
        PMDPeriphOut(&hPeriphPIO, PMDPIO_LED_GREEN, PMDLEDBlinkRate_Off);
    }
    PMDprintf("\r\n");
    PMDprintf("MutexExample1 aborting\r\n" );
    PMDTaskAbort(tasknumber);
}

void MutexExample2( void* pvTaskParam )
{
    bool bSuccess;
    int tasknumber = PMDTaskGetNumber();

    PMDprintf("MutexExample2 is task #%d\r\n", tasknumber );

    for (int i=0; i<10; i++)
    {
        // if mutexes are disabled the output will be interspersed with MutexExample1 output
        bSuccess = PMDMutexLockEx(hMutex, 4000);
        PMDprintf("\r\n");
        if (bSuccess)
            PMDprintf("MutexExample2 locked\r\n" );
        else
            PMDprintf("MutexExample2 timeout\r\n" );
        // Set Red LED 
        PMDPeriphOut(&hPeriphPIO, PMDPIO_LED_RED, PMDLEDBlinkRate_On);
        PMDprintf("     MutexExample2 " );
        PMDTaskWait(waittime);
        PMDprintf("MutexExample2 " );
        PMDTaskWait(waittime);
        PMDprintf("MutexExample2 " );
        PMDTaskWait(waittime);
        PMDprintf("MutexExample2 " );
        PMDTaskWait(waittime);
        PMDMutexUnlock(hMutex);
        PMDTaskWait(0);
        PMDPeriphOut(&hPeriphPIO, PMDPIO_LED_RED, PMDLEDBlinkRate_Off);
    }
    PMDprintf("\r\n");
    PMDprintf("MutexExample2 aborting\r\n" );
    PMDTaskAbort(tasknumber);
}

void GPAnalogInput( void* pvTaskParam )
{
    float volts;
    PMDuint16 volts16;
    int tasknumber = PMDTaskGetNumber();

    PMDprintf("GPAnalogInput is task #%d\r\n", tasknumber );
    for (int i=0; i<1000; i++)
    {
        volts16 = PMDPeriphIn(&hPeriphPIO, PMDPIO_AICh1);
        volts = (float)volts16 * 10.0f / 32767.0f ;
        PMDprintf("GP ADC read %02.3lf volts\r\n", volts );
        PMDTaskWait(100);
    }
    PMDprintf("\r\n");
    PMDprintf("GPAnalogInput aborting\r\n" );
    PMDTaskAbort(tasknumber);
}

