TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

#----------------------------------------------------
# Create and install (or just install) into <top>/db
# databases, templates, substitutions like this
#DB += xxx.db

#----------------------------------------------------
# If <anyname>.db template is not named <anyname>*.template add
# <anyname>_template = <templatename>
DB_INSTALLS += $(filter %.db %.req %.template, $(wildcard $(AREA_DETECTOR)/ADCore/ADApp/Db/*))
DB_INSTALLS += $(filter %.db %.req %.template, $(wildcard $(ADPIXCI)/db/*))

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

