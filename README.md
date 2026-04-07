# escalonador-de-processos
Pequeno projeto contendo um Imesh (mini-bash) e um escalonador de processos para a atividade EP.1 da disciplina MAC0422 - Sistemas Operacionais do IME/USP

A idéia é simular 3 algoritmos de escalonagem de processo: Shortest Job First, Round Robin e Priority Scheduler (priorizando terminar o máximo de processos possíveis dentro do prazo). 

To Do List:
- <s>atualizar o executar_processo() para usar tempo real de processamento ao invés de usar sleep()</s>
- Implementar afinidade de CPU (funções Afinitty), mas preciso testar nos PCs do IME com Linux
- <s>atualizar os escalonadores para medir tempo real de execução ao invés de gerir um relógio virtual. </s>
- <s>adicionar a identificação de erros no comando Kill no imesh</s>
- <s>Adicionar Header (arquivos .h)</s>
- <s>Criar o arquivo Makefile.</s>
- <s>Terminar os slides para o envio da atividade pelo moodle</s>
- Usar Markdown para embelezar este README (sejemos sinceros isso provavelmente não vai ser feito)

Para usar este projeto, primeiro compile os arquivos ep1.c e imesh.c, especificamente com o comando:
- $ make all

E depois executá-los:
-  $ ./imesh
-  $ ./ep1 <id_do_escalonador> <path_input.txt> <path_output.txt>

Na função main do arquivo ep1.c está definido os ids dos escalonadores, a seguir:
  - Shortest Job First : 1
  - Round Robin : 2
  - Priority Scheduler : 3

Os algoritmos implementados nesse projeto foram amplamente baseados no conteúdo visto nas aulas de MAC0422 de 2026.1 no IME/USP, pelo professor Daniel Macedo e nos catálogos de Linux online (www.opengroup.org; man7.org; e en.cppreference.com) , assim como na playlist de Teoria de Sistemas Operacionais do canal Core Dumped (https://www.youtube.com/@CoreDumpped) no youtube e foram usadas algumas soluções apresentadas por usuários do [StackOverflow](https://stackoverflow.com/questions) (por exemplo, para o clock_gettime()). Foi feito uso de inteligências artificiais (Gemini) para gerar os arquivos de Trace e corrigir alguns bugs de implementação do relógio virtual (mas posteriormente isso foi trocado por um relógio real + busy wait nas threads).

Obs: durante o desenvolvimento deste programa, principalmente do ep1.c (que implementa o simulador de processos), eu optei por deixar todos os comentários que usei para me guiar durante o desenvolvimento e por apenas comentar diversos trechos que foram abandonados depois, para que quem estiver lendo possa "acompanhar" como foi o processo de desenvolvimento, detecção de erros e implementação de soluções, assim como as idéias iniciais e os motivos por elas terem sido abandonadas.