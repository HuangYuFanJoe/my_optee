OPTEE_DARKNETZ_VERSION = 1.0
OPTEE_DARKNETZ_SOURCE = local
OPTEE_DARKNETZ_SITE = $(BR2_PACKAGE_OPTEE_DARKNETZ_SITE)
OPTEE_DARKNETZ_SITE_METHOD = local
OPTEE_DARKNETZ_INSTALL_STAGING = YES
OPTEE_DARKNETZ_DEPENDENCIES = optee_client_ext host-python-pycrypto
OPTEE_DARKNETZ_SDK = $(BR2_PACKAGE_OPTEE_DARKNETZ_SDK)
OPTEE_DARKNETZ_CONF_OPTS = -DOPTEE_DARKNETZ_SDK=$(OPTEE_DARKNETZ_SDK)

define OPTEE_DARKNETZ_BUILD_TAS
	@$(foreach f,$(wildcard $(@D)/*/ta/Makefile), \
		echo Building $f && \
			$(MAKE) CROSS_COMPILE="$(shell echo $(BR2_PACKAGE_OPTEE_DARKNETZ_CROSS_COMPILE))" \
			O=out TA_DEV_KIT_DIR=$(OPTEE_DARKNETZ_SDK) \
			$(TARGET_CONFIGURE_OPTS) -C $(dir $f) all &&) true
endef

define OPTEE_DARKNETZ_INSTALL_TAS
	@$(foreach f,$(wildcard $(@D)/*/ta/out/*.ta), \
		mkdir -p $(TARGET_DIR)/lib/optee_armtz && \
		$(INSTALL) -v -p  --mode=444 \
			--target-directory=$(TARGET_DIR)/lib/optee_armtz $f \
			&&) true
endef

OPTEE_DARKNETZ_POST_BUILD_HOOKS += OPTEE_DARKNETZ_BUILD_TAS
OPTEE_DARKNETZ_POST_INSTALL_TARGET_HOOKS += OPTEE_DARKNETZ_INSTALL_TAS

$(eval $(cmake-package))
