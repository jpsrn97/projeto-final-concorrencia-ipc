# Projeto Final – Programação Concorrente e IPC

Disciplina: **Sistemas Operacionais Embarcados e em Tempo Real**  
Tema: **Simulador de Fila de Processamento de Jobs com Concorrência e IPC**

## 🎯 Objetivo

Implementar uma aplicação que demonstra, na prática:

- Programação concorrente com **threads POSIX**;
- Criação e gerenciamento de **processos**;
- Uso de **mecanismos de sincronização** (mutex, condition variable, semáforo);
- Comunicação entre processos (**IPC**) usando:
  - **Pipe anônimo** (pai ↔ filho gerador);
  - **Memória compartilhada POSIX** (`shm_open`, `mmap`).

## 🧩 Arquitetura Geral

A aplicação é dividida em dois executáveis:

### 1. `manager` (processo gerente)

- Cria um **processo filho gerador de jobs** usando `fork()`;
- O gerador envia jobs para o manager através de um **pipe**;
- O manager mantém uma **fila limitada de jobs** em memória;
- Cria um pool de **threads worker** (`pthread_create`) que:
  - Retiram jobs da fila (modelo produtor–consumidor);
  - Processam jobs em paralelo;
- Utiliza:
  - **Mutex + Condition Variable** (`pthread_mutex_t`, `pthread_cond_t`)  
    para proteger a fila e coordenar produtores/consumidores;
  - **Semáforo POSIX** (`sem_t`, `sem_init`, `sem_wait`, `sem_post`)  
    para limitar quantos jobs podem estar em processamento simultaneamente;
  - **Memória compartilhada POSIX** (`shm_open`, `ftruncate`, `mmap`)  
    para expor estatísticas globais de execução.

### 2. `monitor` (processo monitor)

- Conecta-se à mesma **memória compartilhada POSIX** criada pelo `manager`;
- Lê periodicamente a struct `job_stats_t`;
- Exibe em tempo (quase) real:
  - Total de jobs criados;
  - Jobs em fila;
  - Jobs em processamento;
  - Jobs concluídos.

## 🛠️ Conceitos Usados (Resumo Técnico)

- **Processos**
  - `fork`, `waitpid`, `_exit`
- **Threads POSIX**
  - `pthread_create`, `pthread_join`
- **Sincronização**
  - `pthread_mutex_t`, `pthread_cond_t`
  - `sem_t` (semáforo POSIX)
- **Comunicação entre processos (IPC)**
  - Pipe anônimo: `pipe`, `read`, `write`
  - Memória compartilhada POSIX: `shm_open`, `ftruncate`, `mmap`
- **Padrão clássico**
  - Modelo **Produtor–Consumidor** com fila limitada

## 📦 Estrutura de Pastas

```text
projeto-final-concorrencia-ipc/
 ├── src/
 │   ├── manager.c   # processo gerente + threads + fila + pipe + shm
 │   └── monitor.c   # processo monitor, lê estatísticas via shm
 ├── include/
 │   └── common.h    # tipos compartilhados e configurações
 ├── docs/           # (slides e material da apresentação)
 ├── build/          # binários gerados pelo Makefile
 ├
  Makefile
 ├── 
 README.md
 └── 
 .gitignore
