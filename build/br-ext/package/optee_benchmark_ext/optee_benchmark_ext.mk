OPTEE_BENCHMARK_EXT_VERSION = 1.0
OPTEE_BENCHMARK_EXT_SOURCE = local
OPTEE_BENCHMARK_EXT_SITE = $(BR2_PACKAGE_OPTEE_BENCHMARK_EXT_SITE)
OPTEE_BENCHMARK_EXT_SITE_METHOD = local
OPTEE_BENCHMARK_EXT_INSTALL_STAGING = YES
OPTEE_BENCHMARK_EXT_DEPENDENCIES = optee_client_ext libyaml

$(eval $(cmake-package))
