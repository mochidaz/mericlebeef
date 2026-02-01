PLUGIN_NAME = ddb_mericlebeef
TARGET = $(PLUGIN_NAME).so

CC ?= gcc
CFLAGS ?= -Wall -O2 -fPIC -std=c99
LDFLAGS ?= -shared -lpthread

ifdef DEADBEEF_INC
DEADBEEF_CFLAGS = -I$(DEADBEEF_INC)

else
DEADBEEF_CFLAGS := $(shell pkg-config --cflags deadbeef 2>/dev/null)

ifeq ($(DEADBEEF_CFLAGS),)
DEADBEEF_CFLAGS = -I/usr/include/deadbeef -I/usr/local/include/deadbeef
endif
endif

INSTALL_DIR = /opt/deadbeef/lib/deadbeef

SRC = mericlebeef.c
OBJ = $(SRC:.c=.o)

.PHONY: all clean install check

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(DEADBEEF_CFLAGS) -c $< -o $@

install: $(TARGET)
	sudo mkdir -p $(INSTALL_DIR)
	sudo cp $(TARGET) $(INSTALL_DIR)/
	sudo chmod 755 $(INSTALL_DIR)/$(TARGET)
	@echo "Installed to $(INSTALL_DIR)/$(TARGET)"
	@echo "Restart Deadbeef"

clean:
	rm -f $(OBJ) $(TARGET)

check:
	@echo "Using include flags: $(DEADBEEF_CFLAGS)"
	@if [ -f $(INSTALL_DIR)/$(TARGET) ]; then \
		echo "✓ Plugin installed in $(INSTALL_DIR)"; \
		ls -lh $(INSTALL_DIR)/$(TARGET); \
	else \
		echo "✗ Plugin not found in $(INSTALL_DIR)"; \
	fi
