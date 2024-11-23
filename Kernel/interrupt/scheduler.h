#pragma once
#include <core/process.h>

void schedule();
void switch_to_task(uint8_t, ExecutionMode);
