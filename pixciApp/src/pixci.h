
/*
 * This is a driver for PIXCI frame grabber
 * 
 *  Author: Subindev D
 *
 * Created:  23/06/2020
*/

#include "ADDriver.h"

/*___________________________________________________________________________*/

class pixci;

class pixci: public ADDriver {

public:
    pixci(const char *portName, int IDType, const char *IDValue, 
              int maxBuffers, size_t maxMemory,
              int priority, int stackSize);



    /* These are the methods that we override from ADDriver */
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    virtual asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value);

    int connectCamera(void);



};