
/*
 * This is a driver for PIXCI frame grabber
 * 
 *  Author: Subindev D
 *
 * Created:  23/06/2020
*/

#include "ADDriver.h"


static const char *driverName = "pixci";





/*___________________________________________________________________________*/

class pixci;

class pixci: public ADDriver {

public:
    pixci(const char *portName, 
              int maxBuffers, size_t maxMemory,
              int priority, int stackSize);



    /* These are the methods that we override from ADDriver */
    virtual asynStatus connect(asynUser* pasynUser);
    virtual asynStatus disconnect(asynUser* pasynUser);


    // virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    // virtual asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value);

    private:

    asynStatus connectCamera();
    asynStatus disconnectCamera();

};