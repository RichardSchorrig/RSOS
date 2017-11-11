/*
 * RSOSDivision.c
 *
 *  Created on: 27.03.2017
 *      Author: Richard
 */

#include "RSOSDivision.h"

#ifdef USE_RSOSDIVISION

#include "../Task.h"

static Task* task_divide = 0;

RSOSDivision rsosDivision_uint16_mem[MAXRSOSDIVISION];
int8_t rsosDivision_size;


void divideOperation()
{
    int8_t i = rsosDivision_size;
    RSOS_bool noDivisionOperation = RSOS_bool_true;
    for (; i>0; i-=1)
    {
    	RSOSDivision* current = &rsosDivision_uint16_mem[i-1];
    	if (0 != current->divisor)
    	{
    		noDivisionOperation = RSOS_bool_false;

    		uint16_t dividend_shifted = current->dividend;
    		uint8_t n = 0;
    		while (dividend_shifted >= current->divisor)
    		{
    			n += 1;
    			dividend_shifted >>= 1;
    		}

    		if (0 == n)
    		{
    			current->divisor = 0;	// end operation
    		}
    		else
    		{
    			current->result ^= 0x1 << (n - 1);
    			current->dividend -= current->divisor << (n - 1);
    		}
    	}
    }

    if (noDivisionOperation)
    {
        task_mem[currentRunningTask].currentCycle = 0;
    }
    else
    {
        task_mem[currentRunningTask].currentCycle = 2;
    }
}

void RSOSDivision_initOperation(uint8_t taskPriority)
{
    task_divide = addTask(taskPriority, divideOperation);
    setTaskCyclic(task_divide, 2);
}

RSOSDivision* RSOSDivision_init()
{
	rsosDivision_uint16_mem[rsosDivision_size].dividend = 0;
	rsosDivision_uint16_mem[rsosDivision_size].divisor = 0;
	rsosDivision_uint16_mem[rsosDivision_size].result = 0;

	rsosDivision_size += 1;
    return &rsosDivision_uint16_mem[rsosDivision_size - 1];
}

static RSOS_ret RSOSDivision_divide(RSOSDivision* division, rsosdivision_t dividend, rsosdivision_t divisor)
{
	division->dividend = dividend;
	division->divisor = divisor;
    if (0 == divisor || 0 == dividend)
    {
    	division->divisor = 0;
    	if (0 == divisor)
        {
    		division->result = RSOSDIVISION_MAX_UNSIGNED;
    		return RSOS_ret_ERROR;
        }
    }
    else
    {
    	scheduleTask(task_divide);
    }
    return RSOS_ret_OK;
}

RSOS_ret RSOSDivision_divide_unsigned(RSOSDivision* division, rsosdivision_t dividend, rsosdivision_t divisor)
{
	if (1 == divisor)
	{
		division->result = dividend;
		division->divisor = 0;
		division->dividend = 0;

	}
	else
	{
		division->result = 0;
		return RSOSDivision_divide(division, dividend, divisor);
	}
	return RSOS_ret_OK;
}

RSOS_ret RSOSDivision_divide_signed(RSOSDivision* division, signed_rsosdivision_t dividend, signed_rsosdivision_t divisor)
{
	rsosdivision_t isNegative = (dividend ^ divisor) & (RSOSDIVISION_SIGNBIT);
	if (isNegative)
	{
		division->result = RSOSDIVISION_MAX_UNSIGNED;
	}
	else
	{
		division->result = 0;
	}

	if (dividend & RSOSDIVISION_SIGNBIT)
	{
		dividend = ~dividend + 1;
	}
	if (divisor & RSOSDIVISION_SIGNBIT)
	{
		divisor = ~divisor + 1;
	}

	if (1 == divisor)
	{
		if (isNegative)
		{
			division->result = ~dividend;
		}
		else
		{
			division->result = dividend;
		}
		division->divisor = 0;
		division->dividend = 0;
	}
	else
	{
		return RSOSDivision_divide(division, dividend, divisor);
	}
	return RSOS_ret_OK;
}

#endif /* USE_RSOSDIVISION */

