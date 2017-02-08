/**
 * Path.h
 *
 * Created on: 03.02.2017
 * by Richard
 */
 
#ifndef PATH_H_
#define PATH_H_

#define DISPLAY_TEST

#ifdef DISPLAY_COMMANDRECEIVER
#define PATH_RSOSDEFINES_H "I:/CCS_ProjectsV6/Display_CommandReceiver/RSOSDefines.h"
#define PATH_DEFINES_H "I:/CCS_ProjectsV6/Display_CommandReceiver/defines.h"
#ifndef __MSP430G2553__
#define __MSP430G2553__
#endif /* __MSP430G2553__ */
#endif /* DISPLAY_COMMANDRECEIVER */

#ifdef DISPLAY_TEST
#define PATH_RSOSDEFINES_H "I:/CCS_ProjectsV6/F5529_DisplayTest/RSOSDefines.h"
#define PATH_DEFINES_H "I:/CCS_ProjectsV6/F5529_DisplayTest/defines.h"
#ifndef __MSP430F5529__
#define __MSP430F5529__
#endif /* __MSP430F5529__ */
#endif /* DISPLAY_TEST */

#endif /* PATH_H_ */
