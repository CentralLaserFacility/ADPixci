#!../../bin/windows-x64/pixci

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
dbLoadRecords("ADBase.template","P=$(PREFIX),R=cam1:,PORT=$(PORT),ADDR=0,TIMEOUT=1")

#dbLoadRecords("db/xxx.db","user=mii48756")

#asynSetTraceIOMask("$(PORT)",0,2)
<<<<<<< HEAD
#asynSetTraceMask("$(PORT)",-1,0x9) 
#asynSetTraceMask("$(PORT)",0,ASYN_TRACE_ERROR+ASYN_TRACE_WARNING+ASYN_TRACE_FLOW)
asynSetTraceMask("$(PORT)",0,ASYN_TRACE_ERROR+ASYN_TRACE_WARNING)
=======
asynSetTraceMask("$(PORT)",-1,0x9) 
#asynSetTraceMask("$(PORT)",0,ASYN_TRACE_ERROR+ASYN_TRACE_WARNING+ASYN_TRACE_FLOW)
asynSetTraceMask("$(PORT)",0,ASYN_TRACE_ERROR)
>>>>>>> 3ed6c33b15d9c3449737acee801d49222449649b
cd "${TOP}/iocBoot/${IOC}"
iocInit

## Start any sequence programs
#seq sncxxx,"user=mii48756"
