/*
 * RSOSDivision.h
 *
 * a component for dividing integer values
 * the result is also integer
 * not suitable for divisions that result in values smaller than 1
 * the residual of the operation is saved, but both the dividend and divisor
 * are invalid at the end of each operation.
 *
 * Uses a task to control the division operation
 *
 * the division operation is divided into steps,
 * so that the controller is not blocked by the division algorythm
 * this results in a long division time but the cpu can react to other tasks in between
 *
 * operation:
 * 		1. 	dividend and divisor is set by the user in the structure returned by init,
 * 			the function divide() is called
 * 		2.	the algorithm shifts the divisor by n as long as it is smaller than dividend
 * 		3.	when divisor << n is bigger than the dividend, dividend is subtracted by divisor << n
 * 			the result n'th bit is set, the operation yields here
 * 		4.	the steps are repeated as long as n is not null
 * 		5.	when n is null, divisor is set to null, the operation is ended.
 *
 *  Created on: 27.03.2017
 *      Author: Richard
 */

#ifndef RSOSDIVISION_H_
#define RSOSDIVISION_H_

#include <RSOSDefines.h>
#include "../RSOS_BasicInclude.h"
#include <stdint.h>

#ifdef USE_RSOSDIVISION

#ifdef RSOSDIVISION_64BIT
#ifdef BITLENGTH_SET
#error "More than one bit length for RSOSDivision set"
#else
#define BITLENGTH_SET
typedef uint64_t rsosdivision_t;
typedef int64_t signed_rsosdivision_t;
#define RSOSDIVISION_MAX_UNSIGNED 0xFFFFFFFFFFFFFFFF
#define RSOSDIVISION_MAX_SIGNED 0x7FFFFFFFFFFFFFFF
#define RSOSDIVISION_SIGNBIT 0x8000000000000000
#endif /* BITLENGTH_SET */
#endif /* RSOSDIVISION_64BIT */

#ifdef RSOSDIVISION_32BIT
#ifdef BITLENGTH_SET
#error "More than one bit length for RSOSDivision set"
#else
#define BITLENGTH_SET
typedef uint32_t rsosdivision_t;
typedef int32_t signed_rsosdivision_t;
#define RSOSDIVISION_MAX_UNSIGNED 0xFFFFFFFF
#define RSOSDIVISION_MAX_SIGNED 0x7FFFFFFF
#define RSOSDIVISION_SIGNBIT 0x80000000
#endif /* BITLENGTH_SET */
#endif /* RSOSDIVISION_32BIT */

#ifdef RSOSDIVISION_16BIT
#ifdef BITLENGTH_SET
#error "More than one bit length for RSOSDivision set"
#else
#define BITLENGTH_SET
typedef uint16_t rsosdivision_t;
typedef int16_t signed_rsosdivision_t;
#define RSOSDIVISION_MAX_UNSIGNED 0xFFFF
#define RSOSDIVISION_MAX_SIGNED 0x7FFF
#define RSOSDIVISION_SIGNBIT 0x8000
#endif /* BITLENGTH_SET */
#endif /* RSOSDIVISION_16BIT */

#ifdef RSOSDIVISION_8BIT
#ifdef BITLENGTH_SET
#error "More than one bit length for RSOSDivision set"
#else
#define BITLENGTH_SET
typedef uint8_t rsosdivision_t;
typedef int8_t signed_rsosdivision_t;
#define RSOSDIVISION_MAX_UNSIGNED 0xFF
#define RSOSDIVISION_MAX_SIGNED 0x7F
#define RSOSDIVISION_SIGNBIT 0x80
#endif /* BITLENGTH_SET */
#endif /* RSOSDIVISION_8BIT */

#ifndef BITLENGTH_SET
#error "No bit length set for RSOSDivision"
#endif /* BITLENGTH_SET */

/**
 * structure for a division operation
 * the structure is active as long as denum_buffer is smaller than numerator
 * Fields:
 *      dividend: the value to divide, on end it is the residual
 *      divisor: the value the numerator is divided by, when operation is ended it is set to 0
 *      result: the result buffer
 *
 * There are different structures for each bit length and signed / unsigned operation
 */

/**
 * unsigned int 16 bit wide:
 * Memory: this structure takes up 6 Bytes
 */
typedef struct RSOSDivision_t {
    rsosdivision_t dividend;
    rsosdivision_t divisor;
    rsosdivision_t result;
} RSOSDivision;

extern RSOSDivision rsosDivision_mem[MAXRSOSDIVISION];
extern int8_t rsosDivision_size;

/**
 * initializes the division operation
 * 1x Task is needed, consider one extra task for the RSOS
 * @param taskPriority the priority of the division task, can be 0 to divide in the background
 * or a higher number to make division faster (value 0..15)
 */
__EXTERN_C
void RSOSDivision_initOperation(uint8_t taskPriority);

/**
 * initializes a new division structure
 * @return a pointer to the new structure
 */
__EXTERN_C
RSOSDivision* RSOSDivision_init();

/**
 * initiates a new unsigned division operation
 * @param dividend the value to be divided
 * @param divisor the value to divide by
 * @return OK if division is possible, ERROR if division by 0 (the result is RSOSDIVISION_MAX_UNSIGNED)
 */
__EXTERN_C
RSOS_ret RSOSDivision_divide_unsigned(RSOSDivision* division, rsosdivision_t dividend, rsosdivision_t divisor);

/**
 * initiates a new signed division operation
 * @param dividend the value to be divided
 * @param divisor the value to divide by
 * @return OK if division is possible, ERROR if division by 0 (the result is RSOSDIVISION_MAX_UNSIGNED)
 */
__EXTERN_C
RSOS_ret RSOSDivision_divide_signed(RSOSDivision* division, signed_rsosdivision_t dividend, signed_rsosdivision_t divisor);

/**
 * returns if the division operation is done (field result is valid)
 * @param _X a pointer to a RSOSDivision_intX structure
 * @return 0 if not ready, 1 if ready
 */
#define RSOSDivision_isReady(_X) (_X->divisor ? RSOS_bool_false : RSOS_bool_true)

/**
 * returns the result of an unsigned division operation.
 * Check RSOSDivision_isReady() first, else the result might be erroneous.
 * @param _X a pointer to a RSOSDivision_intX structure
 * @return the result of the division operation
 */
#define RSOSDivision_getResult_unsigned(_X) (_X->result)

/**
 * returns the result of a signed division operation.
 * Check RSOSDivision_isReady() first, else the result might be erroneous.
 * @param _X a pointer to a RSOSDivision_intX structure
 * @return the result of the division operation
 */
#define RSOSDivision_getResult_signed(_X) \
	(_X->result) & RSOSDIVISION_SIGNBIT ? ((int16_t)_X->result + 1) : ((int16_t)_X->result)

/**
 * returns the residual of a division operation.
 * Check RSOSDivision_isReady() first, else the residual might be erroneous.
 * @param _X a pointer to a RSOSDivision_intX structure
 * @return the residual of the division operation
 */
#define RSOSDivision_getResidual(_X) (_X->dividend)


#endif /* USE_RSOSDIVISION */
#endif /* RSOSDIVISION_H_ */
