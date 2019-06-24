/*
 * The function __low_level_init it called by the start-up code before
 * "main" is called, and before data segment initialization is
 * performed.
 *
 * This is a template file, modify to perform any initialization that
 * should take place early.
 *
 * The return value of this function controls if data segment
 * initialization should take place. If 0 is returned, it is bypassed.
 *
 * For the MSP430 microcontroller family, please consider disabling
 * the watchdog timer here, as it could time-out during the data
 * segment initialization.
 */

#include "Global.h"
#include <intrinsics.h>

int __low_level_init(void)
{
	WDTCTL = WDTPW+WDTHOLD;//Disable WDT
  /*
   * Return value:
   *
   *  1 - Perform data segment initialization.
   *  0 - Skip data segment initialization.
   */

  return 1;
}
