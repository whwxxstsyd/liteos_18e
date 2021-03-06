#include "hi_osal.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include "los_event.h"

int osal_wait_init(osal_wait_t* wait)
{
    wait_queue_head_t* wq;
    if (wait == NULL)
    {
        osal_printk("%s - parameter invalid!\n", __FUNCTION__);
        return -1;
    }
    wq = (wait_queue_head_t*)kmalloc(sizeof(wait_queue_head_t), GFP_KERNEL);
    if (wq == NULL)
    {
        osal_printk("%s - kmalloc error!\n", __FUNCTION__);
        return -1;
    }
    init_waitqueue_head(wq);
    wait->wait = wq;
    return 0;
}

int osal_wait(osal_wait_t* wait)
{
    wait_queue_head_t* wq;

    wq = (wait_queue_head_t*)(wait->wait);

    if(wq->stEvent.stEventList.pstPrev == (struct LOS_DL_LIST *)0xFFFFFFFF) 
    { 
        LOS_EventInit(&wq->stEvent); 
    } 

    LOS_EventRead(&wq->stEvent,0x1,LOS_WAITMODE_AND|LOS_WAITMODE_CLR, LOS_WAIT_FOREVER);

    return 0;
}
int osal_wait_uninterruptible(osal_wait_t* wait)
{
    /*LiteOS do not support signal*/
    return osal_wait(wait);
}
int osal_wait_timeout(osal_wait_t* wait, unsigned long timeout)
{
    int ret;
    unsigned long tm;
    wait_queue_head_t* wq;
    UINT32 ticks_now, ticks_gap;

    tm = msecs_to_jiffies(timeout) ;
    wq = (wait_queue_head_t*)(wait->wait);

    if(wq->stEvent.stEventList.pstPrev == (struct LOS_DL_LIST *)0xFFFFFFFF) 
    { 
        LOS_EventInit(&wq->stEvent); 
    } 

    ticks_now = LOS_TickCountGet();//osKernelSysTick();

    ret = LOS_EventRead(&wq->stEvent,0x1,LOS_WAITMODE_AND|LOS_WAITMODE_CLR,tm);

    ticks_gap = (LOS_TickCountGet() - ticks_now);
    if(tm <= ticks_gap)
    { 
        return 0;   //timeout
    } 
    else
    {
        tm -= ticks_gap;
    }

    if (ret == LOS_ERRNO_EVENT_READ_TIMEOUT)
    {
        return 0;   //timeout
    }
    else
    {
        return jiffies_to_msecs(tm);  //read wq event  ok
    }

}

void osal_wakeup(osal_wait_t* wait)
{
    wait_queue_head_t* wq;

    wq = (wait_queue_head_t*)(wait->wait);
    wake_up(wq);
}
void osal_wait_destory(osal_wait_t* wait)
{
    wait_queue_head_t* wq;

    wq = (wait_queue_head_t*)(wait->wait);
    kfree(wq);
    wait->wait = NULL;
}

