// This file is part of Honeydew
// Honeydew is licensed under the MIT LICENSE. See the LICENSE file for more info.

#include <honeydew/honeydew.hpp>
#include <honeydew/helpers/task_wrapper.hpp>

namespace honeydew
{

/**
* Helper function which appends a then task to the end of the given task which
*  sets a condition_variable. The calling thread blocks on this condition
*  variable and will not resume until the task at the end of the Task structure
*  sets the var.
* @arg honeydew the Honeydew scheduler to post the task to.
* @arg task the task to append the then relationship to.
*/    
void post_and_wait(Honeydew* honeydew, Task& task);

}
