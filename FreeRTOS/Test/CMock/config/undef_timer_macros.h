/* xTimerDelete is conditionally a macro when portUSING_MPU_WRAPPERS=0.
 * Undef it so CMock-generated weak declarations compile correctly. */
#undef xTimerDelete
