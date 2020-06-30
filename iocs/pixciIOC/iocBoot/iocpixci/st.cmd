#!../../bin/win32-x86/pixci

#- You may have to change pixci to something else
#- everywhere it appears in this file

< envPaths

cd "${TOP}"

## Register all support components
dbLoadDatabase "dbd/pixci.dbd"
pixci_registerRecordDeviceDriver pdbbase

# Prefix for all records
epicsEnvSet("PREFIX", "13PS1:")
# The port name for the detector
epicsEnvSet("PORT",   "PS1")
# The queue size for all plugins
epicsEnvSet("QSIZE",  "20")
# The maximim image width; used for row profiles in the NDPluginStats plugin
epicsEnvSet("XSIZE",  "1360")
# The maximim image height; used for column profiles in the NDPluginStats plugin
epicsEnvSet("YSIZE",  "1024")
# The maximum number of time series points in the NDPluginStats plugin
epicsEnvSet("NCHANS", "2048")
# The maximum number of frames buffered in the NDPluginCircularBuff plugin
epicsEnvSet("CBUFFS", "500")
# The search path for database files
epicsEnvSet("EPICS_DB_INCLUDE_PATH", "$(ADCORE)/db")


#pixciConfig(portName, maxBuffers,maxMemory,priority,stackSize)

pixciConfig("$(PORT)", 0,  0, 0, 0)

# Create a standard arrays plugin, set it to get data from Driver.
NDStdArraysConfigure("Image1", 3, 0, "$(PORT)", 0)
# Set NELEMENTS to at least the total number of pixels in the detector.  The following is a little larger than 4096 x 4096
dbLoadRecords("$(ADCORE)/db/NDStdArrays.template", "P=$(PREFIX),R=image1:,PORT=Image1,ADDR=0,TIMEOUT=1,NDARRAY_PORT=$(PORT),TYPE=Int16,SIZE=16,FTVL=SHORT,NELEMENTS=17000000")

## Load record instances
#dbLoadRecords("db/xxx.db","user=mii48756")

cd "${TOP}/iocBoot/${IOC}"
iocInit

## Start any sequence programs
#seq sncxxx,"user=mii48756"
