OPTEE_GET_CRL_VERSION = 1.0
OPTEE_GET_CRL_SOURCE = local
OPTEE_GET_CRL_SITE = $(BR2_PACKAGE_OPTEE_GET_CRL_SITE)
OPTEE_GET_CRL_SITE_METHOD = local
OPTEE_GET_CRL_INSTALL_STAGING = YES
OPTEE_GET_CRL_DEPENDENCIES = optee_client_ext host-python-pycrypto
OPTEE_GET_CRL_SDK = $(BR2_PACKAGE_OPTEE_GET_CRL_SDK)
OPTEE_GET_CRL_CONF_OPTS = -DOPTEE_GET_CRL_SDK=$(OPTEE_GET_CRL_SDK)

define OPTEE_GET_CRL_BUILD_TAS
	@$(foreach f,$(wildcard $(@D)/*/ta/Makefile), \
		echo Building $f && \
			$(MAKE) CROSS_COMPILE="$(shell echo $(BR2_PACKAGE_OPTEE_GET_CRL_CROSS_COMPILE))" \
			O=out TA_DEV_KIT_DIR=$(OPTEE_GET_CRL_SDK) \
			$(TARGET_CONFIGURE_OPTS) -C $(dir $f) all &&) true
endef

define OPTEE_GET_CRL_INSTALL_TAS
	@$(foreach f,$(wildcard $(@D)/*/ta/out/*.ta), \
		mkdir -p $(TARGET_DIR)/lib/optee_armtz && \
		$(INSTALL) -v -p  --mode=444 \
			--target-directory=$(TARGET_DIR)/lib/optee_armtz $f \
			&&) true
endef

OPTEE_GET_CRL_POST_BUILD_HOOKS += OPTEE_GET_CRL_BUILD_TAS
OPTEE_GET_CRL_POST_INSTALL_TARGET_HOOKS += OPTEE_GET_CRL_INSTALL_TAS

$(eval $(cmake-package))
