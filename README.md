# 🧵 Sistema Completo de Processamento Concorrente com IPC em Linux  
**Disciplina:** Programação Concorrente e Sistemas Operacionais  
**Professor:** Fabio de Sousa Cardoso – UEA  
**Alunos:** João Paulo Santa Rita Neves · Warley Matheus Nogueira  

---

## 🎯 1. Objetivo do Projeto

Este projeto demonstra uma aplicação completa que utiliza os principais conceitos de **concorrência** e **comunicação entre processos (IPC)**:

- Processos POSIX (`fork`, `exec`, `wait`)
- Threads POSIX (`pthread_create`, `pthread_join`)
- IPC:  
  - **PIPE nomeado (FIFO)**  
  - **Memória compartilhada POSIX (`shm_open`, `mmap`)**  
  - Sinais (opcional)
- Sincronização:  
  - **Mutex**  
  - **Semáforos binários**

O sistema simula um pipeline real de processamento de *jobs*, com múltiplas entidades concorrentes trocando mensagens e compartilhando estado.

---

## 🧱 2. Arquitetura do Sistema

```text
                          ┌──────────────────────────────┐
                          │        PROCESSO MANAGER       │
                          │  - Lê jobs do FIFO            │
                          │  - Enfileira tarefas          │
                          │  - Cria threads worker        │
                          └───────────────┬──────────────┘
                                          │
                                  FIFO    │
┌─────────────────────┐                  ▼
│   PROCESSO          │        ┌───────────────────┐
│    GENERATOR        │ -----> │ THREAD WORKER (N) │
│  (produtor de jobs) │        │ - executa job     │
└─────────────────────┘        │ - atualiza SHM    │
                               └─────────┬─────────┘
                                         │ mutex
                                         ▼
                          ┌───────────────────────────────┐
                          │      MEMÓRIA COMPARTILHADA     │
                          │ total_criados                 │
                          │ em_fila                       │
                          │ em_exec                       │
                          │ concluidos                    │
                          └───────────────────────────────┘
                                         ▲
                                         │ leitura periódica
                          ┌───────────────────────────────┐
                          │           MONITOR              │
                          │ - lê SHM a cada 1s             │
                          │ - imprime estatísticas         │
                          └───────────────────────────────┘

---

## 📂 3. Estrutura do Repositório

projeto-final-concorrencia-ipc/
│
├── src/
│   ├── generator.c
│   ├── manager.c
│   ├── monitor.c
│   ├── common.h
│   ├── ipc.h
│
├── build/
├── .gitignore
├── Makefile
└── README.md


---

## ⚙️ 4. Como compilar

make clean
make

Executáveis gerados em:

build/generator
build/manager
build/monitor

---

## 🚀 5. Como Executar

🟦 Terminal 1 — Rodar Manager
make run-manager

🟩 Terminal 2 — Rodar Monitor
make run-monitor

O processo Generator é iniciado automaticamente pelo Manager.

---

## 🔧 6. Tecnologias e Conceitos Aplicados
✔ Processos POSIX

fork(), wait(), exec()

Comunicação via FIFO

✔ Threads POSIX

pthread_create

pthread_join

Semáforos e mutex para sincronização

✔ IPC
| Mecanismo | Uso                        |
| --------- | -------------------------- |
| FIFO      | Generator → Manager        |
| SHM       | Manager/Workers → Monitor  |
| Mutex     | Proteção de escrita na SHM |

---

## 📈 7. Estrutura da Memória Compartilhada
typedef struct {
    int total_criados;
    int em_fila;
    int em_exec;
    int concluidos;
} shm_status_t;