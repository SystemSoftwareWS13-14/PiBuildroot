################################################################################
#
# jefa_web
#
################################################################################

JEFA_WEB_VERSION = 1.0
JEFA_WEB_SOURCE = jefa_web.tar.gz
JEFA_WEB_SITE_METHOD = file
JEFA_WEB_SITE = ~/systemarm/PiBuildroot/myPackages

define JEFA_WEB_INSTALL_TARGET_CMDS
  mkdir -p $(TARGET_DIR)/www
  cp $(@D)/index.html $(TARGET_DIR)/www
  cp -r $(@D)/cgi-bin $(TARGET_DIR)/www
  cp -r $(@D)/jquery-ui $(TARGET_DIR)/www
  $(INSTALL) -D -m 0755 $(@D)/serverStartup $(TARGET_DIR)/etc/init.d/S99ServerStartup
  $(INSTALL) -D -m 0755 $(@D)/simple.script $(TARGET_DIR)/etc/jefa_web/simple.script
endef

$(eval $(generic-package))
