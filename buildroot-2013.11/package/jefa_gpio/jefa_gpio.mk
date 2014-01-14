################################################################################
#
# jefa_gpio
#
################################################################################

JEFA_GPIO_VERSION = 1.0
JEFA_GPIO_SOURCE = jefa_gpio.tar.gz
JEFA_GPIO_SITE_METHOD = file
JEFA_GPIO_SITE = ~/systemarm/PiBuildroot/myPackages
JEFA_GPIO_DEPENDENCIES = linux

define JEFA_GPIO_BUILD_CMDS
  $(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D)/app/src sameDir
  $(MAKE) ARCH=arm CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(LINUX_DIR) M=$(@D)/driver/src
endef

define JEFA_GPIO_INSTALL_TARGET_CMDS
  mkdir -p $(TARGET_DIR)/lib/modules/
  $(INSTALL) -D -m 0755 $(@D)/driver/src/gpio.ko $(TARGET_DIR)/lib/modules
  $(INSTALL) -D -m 0755 $(@D)/app/src/* $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))
