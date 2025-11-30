# Makefile inicial do projeto final - Concorrência & IPC
# Nesta primeira versão, vamos buildar apenas o executável "manager".

CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -ggdb3 -pthread
LDFLAGS = -pthread -lrt

SRC_DIR   = src
INC_DIR   = include
BUILD_DIR = build

# Fontes desta versão inicial
MANAGER_SRC = $(SRC_DIR)/manager.c

# Objetos
MANAGER_OBJ = $(BUILD_DIR)/manager.o

# Binário
MANAGER_BIN = $(BUILD_DIR)/manager

# Alvo padrão: compilar o manager
all: $(MANAGER_BIN)

# Garante que o diretório build existe
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compila o manager.c em manager.o
$(MANAGER_OBJ): $(MANAGER_SRC) $(INC_DIR)/common.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

# Linka o objeto e gera o executável
$(MANAGER_BIN): $(MANAGER_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Atalho para rodar o manager
run-manager: $(MANAGER_BIN)
	$(MANAGER_BIN)

# Limpeza
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean run-manager
