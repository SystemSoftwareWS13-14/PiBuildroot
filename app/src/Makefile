CROSS_COMPILE=/opt/toolchains/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf-

CC=$(CROSS_COMPILE)gcc
CFLAGS=-Wall -g

INSTALL_DIR=../../rootfs_files/usr/bin
BIN_DIR=../bin

RM=rm -rf
MKDIR=mkdir -p
CP=cp

SOURCES=$(wildcard *.c)
BIN_FILES_SAMEDIR=$(patsubst %.c,%,$(SOURCES))
BIN_FILES=$(patsubst %.c,$(BIN_DIR)/%,$(SOURCES))

RM=rm -rf
MKDIR=mkdir -p
CP=cp

.PHONY: all clean install

all: $(BIN_DIR) $(BIN_FILES)
sameDir: $(BIN_FILES_SAMEDIR)

install: all $(INSTALL_DIR)
	$(CP) $(BIN_FILES) $(INSTALL_DIR)

$(BIN_DIR)/%: %.c
	$(CC) $(CFLAGS) -pthread -lrt $^ -o $@

%: %.c
	$(CC) $(CFLAGS) -pthread -lrt $^ -o $@	

#$(BIN_DIR)/led5: led5.c
#	$(CC) $(CFLAGS) -pthread -lrt $^ -o $@

#$(BIN_DIR)/led250: led250.c
#	$(CC) $(CFLAGS) -pthread -lrt $^ -o $@

#$(BIN_DIR)/ledmax: ledmax.c
#	$(CC) $(CFLAGS) -pthread -lrt $^ -o $@

#$(BIN_DIR)/ledflip: ledflip.c
#	$(CC) $(CFLAGS) -pthread -lrt $^ -o $@

$(BIN_DIR):
	$(MKDIR) $(BIN_DIR)

$(INSTALL_DIR):
	$(MKDIR) $(INSTALL_DIR)

clean:
	$(RM) $(BIN_FILES) $(BIN_FILES_SAMEDIR)
