#ifndef __PRIVILEGE_APIS_H__
#define __PRIVILEGE_APIS_H__

#include "FreeRTOS.h"

BaseType_t portIS_PRIVILEGED( void );
// void portRAISE_PRIVILEGE( void );
void portRESET_PRIVILEGE( void );
void portSWITCH_TO_USER_MODE( void );

#endif

