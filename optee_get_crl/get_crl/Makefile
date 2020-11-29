export V?=0

OUTPUT_DIR := $(CURDIR)/out

CA_CROSS_COMPILE ?= $(CROSS_COMPILE)
TA_CROSS_COMPILE ?= $(CROSS_COMPILE)

.PHONY: all
all:
	$(MAKE) -C ca CROSS_COMPILE="$(CA_CROSS_COMPILE)" --no-builtin-variables
	$(MAKE) -C ta CROSS_COMPILE="$(TA_CROSS_COMPILE)" LDFLAGS=""

.PHONY: clean
clean:
	$(MAKE) -C ca clean
	$(MAKE) -C ta clean
