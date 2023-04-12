/**
 ******************************************************************************
 * @file    mos.h
 * @author  snow yang
 * @version v1.0.0
 * @date    2019/06/18
 * @brief   this file provides mxchip operation system apis.
 ******************************************************************************
 */

#ifndef __JOS_H__
#define __JOS_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "jerr.h"

/** 
 * @addtogroup os
 * @{
 */

/** 
 * @defgroup thread Thread
 * @brief Thread funcitons
 * @{
 */

#define MOS_RTOS_HIGEST_PRIORITY (0)     /**< highest priority */
#define MOS_NETWORK_WORKER_PRIORITY (3)  /**< network worker thread priority */
#define MOS_DEFAULT_WORKER_PRIORITY (5)  /**< default worker thread priority */
#define MOS_DEFAULT_LIBRARY_PRIORITY (5) /**< default library thread priority */
#define MOS_APPLICATION_PRIORITY (7)     /**< application thread priority */

#define MOS_NEVER_TIMEOUT (0xFFFFFFFF) /**< never timeout */
#define MOS_WAIT_FOREVER (0xFFFFFFFF)  /**< wait forever, same as "MOS_NEVER_TIMEOUT" */
#define MOS_NO_WAIT (0)                /**< no wait */

typedef void *mos_thread_id_t;                /**< thread id */
typedef void (*mos_thread_func_t)(void *arg); /**< thread function */

/**
 * @brief create a new thread
 * @param priority priority of thread
 * @param name name of thread
 * @param function function of thread
 * @param stack_size stack size of thread
 * @param arg argument of thread
 * 
 * @return id of thread
 * @retval non-null sucess
 * @retval null failure
 * 
 * example
 * @code
 * int led = 1;
 * mos_thread_id_t tid;
 * 
 * tid = mos_thread_new(3, "led blink thread", led_blink_thread, 512, &led);
 * 
 * if (tid == NULL)
 * {
 *      printf("create new thread failed!\r\n");
 * }
 * @endcode
 */
mos_thread_id_t mos_thread_new(uint8_t priority, const char *name, mos_thread_func_t function, uint32_t stack_size, void *arg);
/**
 * @brief delete a thread
 * @param id id of thread
 */
void mos_thread_delete(mos_thread_id_t id);
/**
 * @brief suspend a thread
 * @param id id of thread
 */
void mos_thread_suspend(mos_thread_id_t id);
/**
 * @brief resume a thread
 * @param id id of thread
 */
void mos_thread_resume(mos_thread_id_t id);
/**
 * @brief yield a thread
 */
void mos_thread_yield(void);
/**
 * @brief join a thread
 * 
 * @param id id of thread
 */
void mos_thread_join(mos_thread_id_t id);
/**
 * @brief force wake a thread
 * 
 * @param id id of thread
 */
void mos_thread_awake(mos_thread_id_t id);
/**
 * @brief get the id of current thread
 * 
 * @return id of current thread
 */
mos_thread_id_t mos_thread_get_id(void);
/**
 * @brief get the name of current thread
 * @param id id of thread
 * 
 * @return name of thread
 */
const char *mos_thread_get_name(mos_thread_id_t id);
/** 
 * @} 
 */

/** 
 * @defgroup semaphore Semaphore
 * @brief Semaphore funcitons
 * @{
 */
typedef void *mos_semphr_id_t; /**< semaphore id */
/**
 * @brief create a new semaphore
 * @param count maximum count of semaphore
 * 
 * @return id of semaphore
 * @retval non-null sucess
 * @retval null failure
 */
mos_semphr_id_t mos_semphr_new(uint32_t count);
/**
 * @brief acquire a semaphore
 * @param id id of semaphore
 * @param timeout timeout in ms
 * 
 * @return result
 * @retval kNoErr sucess
 * @retval others failure
 */
merr_t mos_semphr_acquire(mos_semphr_id_t id, uint32_t timeout);
/**
 * @brief realese a semaphore
 * @param id id of semaphore
 * 
 * @return result
 * @retval kNoErr sucess
 * @retval others failure
 */
merr_t mos_semphr_release(mos_semphr_id_t id);
/**
 * @brief delete a semaphore
 * @param id id of semaphore
 */
void mos_semphr_delete(mos_semphr_id_t id);
/** 
 * @} 
 */

/** 
 * @defgroup mutex Mutex
 * @brief Mutex funcitons
 * @{
 */
typedef void *mos_mutex_id_t; /**< mutex id */
/**
 * @brief create a new mutex
 * 
 * @return id of mutex
 * @retval non-null sucess
 * @retval null failure
 */
mos_mutex_id_t mos_mutex_new(void);
/**
 * @brief lock a mutex
 * @param id id of mutex
 */
void mos_mutex_lock(mos_mutex_id_t id);
/**
 * @brief unlock a mutex
 * @param id id of mutex
 */
void mos_mutex_unlock(mos_mutex_id_t id);
/**
 * @brief delete a mutex
 * @param id id of mutex
 */
void mos_mutex_delete(mos_mutex_id_t id);
/** 
 * @}
 */

/** 
 * @defgroup queue Queue
 * @brief Queue funcitons
 * @{
 */
typedef void *mos_queue_id_t; /**< queue id */
/**
 * @brief create a new queue
 * @param size size of item
 * @param number maximum number of items
 * 
 * @return id of queue
 * @retval non-null sucess
 * @retval null failure
 */
mos_queue_id_t mos_queue_new(uint32_t size, uint32_t number);
/**
 * @brief push a item to queue
 * @param id id of queue
 * @param data buffer to store item data
 * @param timeout timeout in ms
 * 
 * @return result
 * @retval kNoErr sucess
 * @retval others failure
 */
merr_t mos_queue_push(mos_queue_id_t id, void *data, uint32_t timeout);
/**
 * @brief pop a item from queue
 * @param id id of queue
 * @param data buffer to store item data
 * @param timeout timeout in ms
 * 
 * @return result
 * @retval kNoErr sucess
 * @retval others failure
 */
merr_t mos_queue_pop(mos_queue_id_t id, void *data, uint32_t timeout);
/**
 * @brief delete a queue
 * @param id id of queue
 */
void mos_queue_delete(mos_queue_id_t id);
/**
 * @brief get total number of a queue
 * @param id id of queue
 * 
 * @return total number of a queue
 */
uint32_t mos_queue_get_total(mos_queue_id_t id);
/**
 * @brief get free number of a queue
 * @param id id of queue
 * 
 * @return free number of a queue
 */
uint32_t mos_queue_get_free(mos_queue_id_t id);
/** 
 * @}
 */

/** 
 * @defgroup timer Timer
 * @brief Timer funcitons
 * @{
 */
typedef void *mos_timer_id_t;                   /**< timer id */
typedef void (*mos_timer_handler_t)(void *arg); /**< timer function */
/**
 * @brief create a new timer
 * @param timeout timer expired time in ms
 * @param function function of timer
 * @param repeat true repeat, false one shot
 * @param arg argument of thread
 * 
 * @return id of timer
 * @retval non-null sucess
 * @retval null failure
 */
mos_timer_id_t mos_timer_new(uint32_t timeout, mos_timer_handler_t function, bool repeat, void *arg);
/**
 * @brief delete a timer
 * @param id id of timer
 */
void mos_timer_delete(mos_timer_id_t id);
/**
 * @brief start a timer
 * @param id id of timer
 */
void mos_timer_start(mos_timer_id_t id);
/**
 * @brief stop a timer
 * @param id id of timer
 */
void mos_timer_stop(mos_timer_id_t id);
/**
 * @brief change a timer's timeout
 * @param id id of timer
 * @param timeout timeout in ms
 */
void mos_timer_change(mos_timer_id_t id, uint32_t timeout);
/**
 * @brief return timer runing status
 * @param id id of timer
 * 
 * @retval true timer is runing
 * @retval false timer is stoped
 */
bool mos_timer_is_runing(mos_timer_id_t id);
/**
 * @brief reset a timer
 * @param id id of timer
 */
void mos_timer_reset(mos_timer_id_t id);
/** 
 * @}
 */

/** 
 * @defgroup Eventgroup Eventgroup
 * @brief Eventgroup funcitons
 * @{
 */
typedef void *mos_eventgroup_id_t;

mos_eventgroup_id_t mos_eventgroup_init(void);

void mos_eventgroup_deinit(mos_eventgroup_id_t id);

uint32_t mos_eventgroup_set_bits(mos_eventgroup_id_t id, int bits);

uint32_t mos_eventgroup_clear_bits(mos_eventgroup_id_t id, int bits);

uint32_t mos_eventgroup_wait_bits(mos_eventgroup_id_t id, int bits, 
            int isClearOnExit, int isWaitForAllBits, int waitMs);
/** 
 * @}
 */


/** 
 * @defgroup eventfd Eventfd
 * @brief Eventfd funcitons
 * @{
 */
typedef void *mos_event_id_t; /**< event id */
/**
 * @brief create a event fd
 * @param handle event id
 * 
 * @return fd of event
 * @retval >= 0 sucess
 * @retval <0 failure
 */
int mos_event_fd_new(mos_event_id_t handle);
/**
 * @brief delete a event fd
 * @param fd event fd
 */
int mos_event_fd_delete(int fd);
/** 
 * @}
 */

/** 
 * @defgroup os_utils Utils
 * @brief Utils funcitons
 * @{
 */
/**
 * @brief memory heap information structure
 */
typedef struct
{
    int total;    /**< total space */
    int free;     /**< total free space */
    int chunks;   /**< number of free chunks */
    int min_free; /**< maximum allocated space */
} mos_mallinfo_t;
/**
 * @brief malloc record item structure
 */
typedef struct
{
    mos_thread_id_t thread; /**< caller thread id */
    void *caller;           /**< caller function address */
    uint32_t size;          /**< allocate size */
    uint32_t addr;          /**< allocated space address */
} mos_mallrecord_t;
/**
 * @brief get the timestaps in ms from system bootup
 * 
 * @return timestamps in ms
 */
uint32_t mos_time(void);
/**
 * @brief put current thread in sleep state in seconds
 * @param seconds sleep time in seconds
 */
void mos_sleep(float seconds);
/**
 * @brief put current thread in sleep state in ms
 * @param ms sleep time in ms
 */
void mos_msleep(uint32_t ms);
/**
 * @brief get memory heap information
 * @return memory heap information pointer
 */
mos_mallinfo_t *mos_mallinfo(void);
/**
 * @brief show the malloc record list
 */
void mos_mallrecord_show(void);
/** 
 * @}
 */
/** 
 * @}
 */

#endif
