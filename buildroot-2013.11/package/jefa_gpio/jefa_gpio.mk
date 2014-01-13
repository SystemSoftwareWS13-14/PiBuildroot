################################################################################
#
# jefa_gpio
#
################################################################################

JEFA_GPIO_VERSION = 1.0
JEFA_GPIO_SOURCE = jefa_gpio.tar.gz
JEFA_GPIO_SITE_METHOD = file
JEFA_GPIO_SITE = ~/systemarm/PiBuildroot/gpio

define JEFA_GPIO_BUILD_CMDS
  $(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D) sameDir
endef

define JEFA_GPIO_INSTALL_TARGET_CMDS
  mkdir -p $(TARGET_DIR)/lib/modules/linux
  cp $(@D)/index.html $(TARGET_DIR)/www
  cp -r $(@D)/cgi-bin $(TARGET_DIR)/www
  cp -r $(@D)/jquery-ui $(TARGET_DIR)/www
  $(INSTALL) -D -m 0755 $(@D)/serverStartup $(TARGET_DIR)/etc/init.d/S99ServerStartup
  $(INSTALL) -D -m 0755 $(@D)/simple.script $(TARGET_DIR)/etc/jefa_web/simple.script
endef

$(eval $(generic-package))
