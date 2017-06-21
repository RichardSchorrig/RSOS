/*
 * RSOS_BasicInclude.h
 *
 * File to be included in the project that
 * makes use of the RSOS.
 *
 * In this File, some types are defined that
 * are used as return values, boolean values, etc
 *
 *  Created on: 12.06.2017
 *      Author: Richard
 */

#ifndef RSOS_BASICINCLUDE_H_
#define RSOS_BASICINCLUDE_H_

#include <stdint.h>

/**
 * boolean define for bool like variables
 */
typedef uint8_t RSOS_bool;

/**
 * RSOS_bool: true
 */
#define RSOS_bool_true 1

/**
 * RSOS_bool: false
 */
#define RSOS_bool_false 0

/**
 * return value define that identifies success / error
 */
typedef int8_t RSOS_ret;

/**
 * RSOS_ret: error
 */
#define RSOS_ret_ERROR -1

/**
 * RSOS_ret: ok
 */
#define RSOS_ret_OK 0


#endif /* RSOS_BASICINCLUDE_H_ */
