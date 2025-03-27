#include "pixci.h"


/**
 * @brief C Function prototypes to tie in with EPICS
 * run acquire task
 * @param drvPvt
 */
static void acquireTaskC(void *drvPvt);

// static void serialTaskC(void *drvPvt);
static void paramTaskC(void *drvPvt);