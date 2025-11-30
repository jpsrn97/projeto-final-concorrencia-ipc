# Makefile do projeto final - Concorrência & IPC

CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -ggdb3 -pthread
LDFLAGS = -pthread -lrt

SRC_DIR   = src
INC_DIR   = include
BUILD_DIR = build

# Fontes
MANAGER_SRC = $(SRC_DIR)/manager.c
MONITOR_SRC = $(SRC_DIR)/monitor.c

# Objetos
MANAGER_OBJ = $(BUILD_DIR)/manager.o
MONITOR_OBJ = $(BUILD_DIR)/monitor.o

# Binários
MANAGER_BIN = $(BUILD_DIR)/manager
MONITOR_BIN = $(BUILD_DIR)/monitor

# Alvo padrão
all: $(MANAGER_BIN) $(MONITOR_BIN)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Manager
$(MANAGER_OBJ): $(MANAGER_SRC) $(INC_DIR)/common.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(MANAGER_BIN): $(MANAGER_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Monitor
$(MONITOR_OBJ): $(MONITOR_SRC) $(INC_DIR)/common.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(MONITOR_BIN): $(MONITOR_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Execução rápida
run-manager: $(MANAGER_BIN)
	$(MANAGER_BIN)

run-monitor: $(MONITOR_BIN)
	$(MONITOR_BIN)

# limpeza
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean run-manager run-monitor
