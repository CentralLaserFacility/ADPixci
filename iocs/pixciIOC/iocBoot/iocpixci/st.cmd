#!../../bin/windows-x64/pixci

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
epicsEnvSet("XSIZE",  "600")
# The maximim image height; used for column profiles in the NDPluginStats plugin
epicsEnvSet("YSIZE",  "600")
# The maximum number of time series points in the NDPluginStats plugin
epicsEnvSet("NCHANS", "512")
# The search path for database files
epicsEnvSet("EPICS_DB_INCLUDE_PATH", "$(ADCORE)/db")
epicsEnvSet("CAMERA_MODEL","4710")
epicsEnvSet("RAPTOR_SETTINGS_FILE","$(ADPIXCI)/formatFiles/Raptor_Photonics_EagleXV_$(CAMERA_MODEL).fmt")


#pixciConfig(portName, maxBuffers,maxMemory,priority,stackSize,formatfile)
pixciConfig("$(PORT)", 0,  0, 0, 0, $(CAMERA_MODEL))
dbLoadRecords("$(ADPIXCI)/db/Pixci.template","P=$(PREFIX),R=cam1:,PORT=$(PORT),ADDR=0,TIMEOUT=1")

NDStdArraysConfigure("Image1", 20, 0, "$(PORT)", 0, 0, 0, 0, 0, 5)
dbLoadRecords("NDStdArrays.template", "P=$(PREFIX),R=image1:,PORT=Image1,ADDR=0,TIMEOUT=1,NDARRAY_PORT=$(PORT),TYPE=Int16,FTVL=USHORT,NELEMENTS=12000000")
#asynSetTraceIOMask("$(PORT)",0,0x2)
#asynSetTraceMask("$(PORT)",0,0x9) 
#asynSetTraceMask("$(PORT)",0,ASYN_TRACE_ERROR+ASYN_TRACE_WARNING+ASYN_TRACE_FLOW)
asynSetTraceMask("$(PORT)",0,ASYN_TRACE_ERROR)
cd "${TOP}/iocBoot/${IOC}"
iocInit
dbpf $(PREFIX)image1:EnableCallbacks 1

