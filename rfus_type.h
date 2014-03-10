// This file is part of RFUS (Rcythr's Fast Userspace Scheduling)
// RFUS is licensed under the MIT LICENSE. See the LICENSE file for more info.

#pragma once

namespace rcythr
{

/**
* Defines how the implementation of the RFUS behaves when a task is not
*  associated with a resource.
*/
enum RFUSType
{
    /**
    * In this implementation tasks are simply cycled through the different
    *  available queues.
    * Advantages:
    *   * Predictable
    *   * Fast
    *   * Easy to Understand
    * Disadvantages:
    *   * Tasks may be pushed onto very active queues.
    */
    ROUND_ROBIN = 0,

    /**
    * In this implementation tasks are simply cycled through the different
    *  available queues.
    * Advantages:
    *   * Tasks pushed onto active queues will be reordered based on priority (closest deadline).
    * Disadvantages:
    *   * If a low priority task is pushed onto a queue of mostly high priority tasks
    *       it may take a long time for the low priority task to be executed.
    *   * Extra work reordering queue.
    */
    ROUND_ROBIN_WITH_PRIORITY,

    /**
    * In this implementation tasks are pushed to the least busy queue at the time.
    * Advantages:
    *   * Assuming very short tasks, fastest performance.
    * Disadvantages:
    *   * If tasks take a long time to complete other, shorter tasks may be delayed.
    *   * Bookeeping of the length of each queue.
    */
    LEAST_BUSY,

    /**
    * In this implementation tasks are pushed to the least busy queue at the time.
    *   The tasks on that queue are then sorted based upon their priority.
    * Advantages:
    *   * Assuming very short tasks, fastest performance.
    *   * Reorders tasks based upon their priority (closest deadline).
    * Disadvantages:
    *   * Bookeeping of the length of each queue.
    *   * Extra work reordering queue.
    */
    LEAST_BUSY_WITH_PRIORITY,

};

}
