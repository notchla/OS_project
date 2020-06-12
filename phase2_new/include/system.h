#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#ifdef TARGET_UMPS
#include "umps/libumps.h"
#include "umps/arch.h"
#include "umps/types.h"
#endif
#ifdef TARGET_UARM
#include "uarm/libuarm.h"
#include "uarm/arch.h"
#include "uarm/uARMtypes.h"
#endif

#include "const_bikaya.h"

#endif
