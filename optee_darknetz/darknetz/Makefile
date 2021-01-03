export V?=0

# If _CA or _TA specific compilers are not specified, then use CROSS_COMPILE
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
