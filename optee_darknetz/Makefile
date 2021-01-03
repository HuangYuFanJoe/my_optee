export V?=0

OUTPUT_DIR := $(CURDIR)/out

PROJECT_LIST := $(subst /,,$(dir $(wildcard */Makefile)))

.PHONY: all
all: projects prepare-for-rootfs

.PHONY: clean
clean: projects-clean prepare-for-rootfs-clean

projects:
	@for project in $(PROJECT_LIST); do \
		$(MAKE) -C $$project CROSS_COMPILE="$(HOST_CROSS_COMPILE)" || exit -1; \
	done

projects-clean:
	@for project in $(PROJECT_LIST); do \
		$(MAKE) -C $$project clean || exit -1; \
	done

prepare-for-rootfs: projects
	@echo "Copying project CA and TA binaries to $(OUTPUT_DIR)..."
	@mkdir -p $(OUTPUT_DIR)
	@mkdir -p $(OUTPUT_DIR)/ta
	@mkdir -p $(OUTPUT_DIR)/ca
	@for project in $(PROJECT_LIST); do \
		if [ -e $$project/ca/optee_$$project ]; then \
			cp -p $$project/ca/optee_$$project $(OUTPUT_DIR)/ca/; \
		fi; \
		cp -pr $$project/ta/*.ta $(OUTPUT_DIR)/ta/; \
	done

prepare-for-rootfs-clean:
	@rm -rf $(OUTPUT_DIR)/ca
	@rm -rf $(OUTPUT_DIR)/ta
	@rmdir --ignore-fail-on-non-empty $(OUTPUT_DIR) || test ! -e $(OUTPUT_DIR)
