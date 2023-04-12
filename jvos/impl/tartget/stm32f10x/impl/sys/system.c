// #include "rtl8710c.h"
// #include "sys_api.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"
#include "jos.h"

void mxos_sys_reboot(void)
{
    // BKUP_Set(BKUP_REG0, BIT_SYS_RESET_HAPPEN);
    sys_reset();
}

char *mxos_system_lib_version(void)
{
    // return LIB_VER;
}

void mxos_sys_standby(uint32_t secondsToWakeup)
{
    printf("\"%s\" is not implemented yet\r\n", __FUNCTION__);
}

void mxos_mcu_powersave_config(int enable)
{
    printf("\"%s\" is not implemented yet\r\n", __FUNCTION__);
}

/*
 * Macros used by vListTask to indicate which state a task is in.
 */
#define tskBLOCKED_CHAR ('B')
#define tskREADY_CHAR ('R')
#define tskDELETED_CHAR ('D')
#define tskSUSPENDED_CHAR ('S')

static char *prvWriteNameToBuffer(char *pcBuffer, const char *pcTaskName)
{
    long x;

    /* Start by copying the entire string. */
    strcpy(pcBuffer, pcTaskName);

    /* Pad the end of the string with spaces to ensure columns line up when
    printed out. */
    for (x = strlen(pcBuffer); x < (configMAX_TASK_NAME_LEN - 1); x++)
    {
        pcBuffer[x] = ' ';
    }

    /* Terminate. */
    pcBuffer[x] = 0x00;

    /* Return the new end of string. */
    return &(pcBuffer[x]);
}

/* Re-write vTaskList to add a buffer size parameter */
merr_t mxos_rtos_print_thread_status( void )
{
    TaskStatus_t *pxTaskStatusArray;
    unsigned portBASE_TYPE uxCurrentNumberOfTasks = uxTaskGetNumberOfTasks();
    volatile UBaseType_t uxArraySize, x;
    char cStatus;
    char pcTaskStatusStr[64];
    char *pcTaskStatusStrTmp;

    /* Take a snapshot of the number of tasks in case it changes while this
    function is executing. */
    uxArraySize = uxCurrentNumberOfTasks;

    /* Allocate an array index for each task.  NOTE!  if
     configSUPPORT_DYNAMIC_ALLOCATION is set to 0 then pvPortMalloc() will
     equate to NULL. */
    pxTaskStatusArray = pvPortMalloc(uxCurrentNumberOfTasks * sizeof(TaskStatus_t));

    mcli_printf("%-12s Status     Prio    Stack   TCB\r\n", "Name");
    mcli_printf("-------------------------------------------\r\n");

    if (pxTaskStatusArray != NULL)
    {
        /* Generate the (binary) data. */
        uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, NULL);

        /* Create a human readable table from the binary data. */
        for (x = 0; x < uxArraySize; x++)
        {
            switch (pxTaskStatusArray[x].eCurrentState)
            {
            case eReady:
                cStatus = tskREADY_CHAR;
                break;

            case eBlocked:
                cStatus = tskBLOCKED_CHAR;
                break;

            case eSuspended:
                cStatus = tskSUSPENDED_CHAR;
                break;

            case eDeleted:
                cStatus = tskDELETED_CHAR;
                break;

            default: /* Should not get here, but it is included
                 to prevent static checking errors. */
                cStatus = 0x00;
                break;
            }

            /* Write the task name to the string, padding with spaces so it
             can be printed in tabular form more easily. */
            pcTaskStatusStrTmp = pcTaskStatusStr;
            pcTaskStatusStrTmp = prvWriteNameToBuffer(pcTaskStatusStrTmp, pxTaskStatusArray[x].pcTaskName);

            /* Write the rest of the string. */
            sprintf(pcTaskStatusStrTmp, "\t%c\t%u\t%u\t%u\r\n", cStatus,
                    (unsigned int)pxTaskStatusArray[x].uxCurrentPriority,
                    (unsigned int)pxTaskStatusArray[x].usStackHighWaterMark,
                    (unsigned int)pxTaskStatusArray[x].xTaskNumber);
                    
            mcli_printf("%s", pcTaskStatusStr);
        }

        /* Free the array again.  NOTE!  If configSUPPORT_DYNAMIC_ALLOCATION
         is 0 then vPortFree() will be #defined to nothing. */
        vPortFree(pxTaskStatusArray);
    }
    else
    {
        mtCOVERAGE_TEST_MARKER();
    }

    return kNoErr;
}
