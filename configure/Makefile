TOP=..

include $(TOP)/configure/CONFIG

TARGETS = $(CONFIG_TARGETS)
CONFIGS += $(subst ../,,$(wildcard $(CONFIG_INSTALLS)))
CFG += CONFIG_FMT RULES_FMT

include $(TOP)/configure/RULES
