# escalonador-de-processos
Pequeno projeto contendo um Imesh (mini-bash) e um escalonador de processos para a atividade EP.1 da disciplina MAC0422 - Sistemas Operacionais do IME/USP

A idéia é simular 3 algoritmos de escalonagem de processo: Shortest Job First, Round Robin e Priority Scheduler (priorizando terminar o máximo de processos possíveis dentro do prazo). 

To Do List:
- atualizar o executar_processo() para usar tempo real de processamento ao invés de usar sleep()
- Implementar afinidade de CPU (funções Afinitty), mas preciso testas nos PCs do IME com Linux
- atualizar os escalonadores para medir tempo real de execução ao invés de gerir um relógio virtual. 
- adicionar a identificação de erros no comando Kill no imesh
- Criar o arquivo Makefile.
- Terminar os slides para o envio da atividade pelo moodle
- Usar Markdown para embelezar este README

Para usar este projeto, primeiro compile os arquivos ep1.c e imesh.c, utilizando seu compilador escolhido, como:
-  $ make imesh
-  $ make ep1

E depois executá-los:
-  $ ./imesh
-  $ ./ep1 <id_do_escalonador> <path_input.txt> <path_output.txt>

Na função main do arquivo ep1.c está definido os ids dos escalonadores, a seguir:
  - Shortest Job First : 1
  - Round Robin : 2
  - Priority Scheduler : 3
