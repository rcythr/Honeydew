
#include "rfus.hpp"
#include "helpers/task_wrapper.hpp"

namespace rfus
{

/**
* Helper function which appends a then task to the end of the given task which
*  sets a condition_variable. The calling thread blocks on this condition
*  variable and will not resume until the task at the end of the Task structure
*  sets the var.
* @arg rfus the RFUS scheduler to post the task to.
* @arg task the task to append the then relationship to.
*/    
void post_and_wait(RFUSInterface* rfus, Task& task);

}
