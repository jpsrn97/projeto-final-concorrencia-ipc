Título do Projeto: Sistema Completo de Processamento Concorrente com IPC em Linux
Disciplina: Programação Concorrente e Sistemas Operacionais
Professor: Fabio de Sousa Cardoso – UEA
Alunos: João Paulo Santa Rita Neves 
        Warley Matheus Nogueira

📌 1. Objetivo do Projeto

Este projeto demonstra uma aplicação completa que utiliza todos os principais conceitos de concorrência e IPC (Inter-Process Communication) estudados na disciplina:

Processos (fork, exec, wait)

Threads POSIX (pthread)

IPC: PIPE nomeado (FIFO), memória compartilhada POSIX, sinais

Sincronização: mutexes, semáforos binários

Arquitetura Produtor → Gerente → Trabalhadores → Monitor

Projeto funcional, executável e modular

O sistema simula um pipeline real de processamento de “jobs” com múltiplas entidades concorrentes e comunicação interprocessual.

🏗️ 2. Arquitetura do Sistema
                        ┌────────────────────────────────┐
                        │          PROCESSO MANAGER       │
                        │  - Recebe jobs do gerador       │
                        │  - Distribui para workers       │
                        └───────────────┬─────────────────┘
                                        │ PIPE
                                        ▼
      ┌─────────────────┐         ┌───────────────┐
      │ PROCESSO        │  FIFO   │ THREAD WORKER │ x N
      │  GERADOR        │─────────► (pthread)     │
      │  (produtor)     │         │ - executa job │
      └─────────────────┘         └──────┬────────┘
                                         │ Mutex + ...
                                         │
                                         ▼
                            Memória Compartilhada (SHM)
                            total_criados | em_fila | em_execução |
                            total_concluídos


✔ O que cada módulo faz:
Componente	Função
generator	Gera jobs e envia ao Manager via PIPE
manager	Recebe jobs, controla fila, cria workers
workers	Threads que processam tarefas concorrentes
monitor	Processo externo que lê o estado pela SHM

🚀 3. Como Executar o Projeto
1️⃣ Compilar todos os módulos
make clean
make


Isso gera:

build/generator
build/manager
build/monitor

2️⃣ Abrir dois terminais
Terminal 1 → Rodar Manager

O Manager automaticamente inicia as threads e recebe jobs do Generator:

make run-manager

Terminal 2 → Rodar Monitor

Mostra estatísticas de processamento em tempo real:

make run-monitor

📦 4. Estrutura do Repositório
projeto-final-concorrencia-ipc/
│
├── src/
│   ├── generator.c     # Processo produtor
│   ├── manager.c       # Processo gerente
│   ├── monitor.c       # Processo monitor via SHM
│   ├── ipc.h           # Constantes e interface
│   └── common.h        # Estruturas compartilhadas
│
├── build/              # Arquivos compilados
│
├── Makefile            # Build profissional
└── README.md           # ESTE ARQUIVO

🔧 5. Tecnologias e Mecanismos Utilizados
✔ Processos POSIX

fork(), execve()

wait() para sincronização entre pai/filho

✔ Threads POSIX

pthread_create()

pthread_join()

pthread_mutex_t

✔ IPC — Comunicação Entre Processos
Tecnologia	Onde usamos
PIPE nomeado (FIFO)	Comunicação Generator → Manager
Memória Compartilhada POSIX (shm_open + mmap)	Monitor lê estatísticas em tempo real
Sinais POSIX (SIGINT)	Finalização limpa dos processos
✔ Sincronização

Semáforo controla tamanho da fila de jobs

Mutex protege a memória compartilhada

Mutex + condition variables gerenciam workers no Manager

📊 6. Funcionamento do Sistema

O Manager cria FIFO e aguarda jobs.

O Generator começa a escrever jobs no FIFO.

O Manager distribui esses jobs para as threads.

Ao final de cada job:

Workers atualizam a SHM com mutex.

O Monitor lê a SHM e imprime:

[MONITOR] total_criados=20 | em_fila=0 | em_proc=0 | concluídos=20

🧪 7. Demonstração (Exemplo de Execução)
Terminal 1 (Manager + Workers)
[GENERATOR] Criando job 12
[MANAGER] Recebeu job 12
[WORKER 2] Processando job 12
[WORKER 2] Finalizou job 12
...

Terminal 2 (Monitor)
[MONITOR] total_criados=20 | em_fila=0 | em_proc=1 | concluídos=19
[MONITOR] total_criados=20 | em_fila=0 | em_proc=0 | concluídos=20

📝 8. Pontos Fortes do Projeto

Este projeto demonstra claramente:

✔ Processos se comunicando via FIFO

✔ Threads executando tarefas concorrentes

✔ Controle rigoroso via mutex + semáforos

✔ Memória compartilhada como canal de monitoramento

✔ Arquitetura modular e escalável

✔ Código organizado e padrão profissional

✔ Makefile limpo e reprodutível

🎓 9. Conclusão

Este projeto integra os principais pilares da Programação Concorrente e IPC, simulando um sistema real de processamento distribuído.
A solução implementa:

Comunicação robusta entre processos

Múltiplos workers concorrentes

Sincronização eficiente

Monitoramento externo em tempo real

Arquitetura escalável e modular

O conjunto demonstra domínio completo dos conteúdos da disciplina e segue padrões profissionais de desenvolvimento.