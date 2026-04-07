#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/wait.h>
#include <readline/history.h>
#include <readline/readline.h>

#include <imesh.h>

void run_simulador(char *input){
    char *args[64];
    int i = 0;

    char *token = strtok(input, " ");
    while(token!=NULL && i<63){
        args[i++]= token;
        token=strtok(NULL, " "); // tipo o String.strip() do python/javascript
    }
    args[i] =NULL;

    if(execvp(args[0], args)== -1){ // aqui ja rodo o processo com execvp e checo se deu ruim
        perror("imesh: erro ao executar o simulador");
        exit(1);
    }
}

void run_pwd(){
    char cwd[1024];
    if(getcwd(cwd, sizeof(cwd))!= NULL){
        printf("%s\n", cwd);
    }else{
        perror("imesh");
    }
}

void run_date(){
    time_t seconds = time(NULL);

    if(seconds ==(time_t)-1){
        perror("imesh: date");
    }else{
        // %ld é usado pois aqui está alertando que time_t é um long int
        printf("%ld\n", (long)seconds);
    }
}

void run_kill(char *input){
    int pid;
    int sinal;

    if(sscanf(input, "kill -%d %d", &sinal, &pid)== 2){
        if(kill(pid, sinal)== -1){
            fprintf(stderr, "imesh: kill: %d - %s\n", pid, strerror(errno));
        }
    } 
    else {  // nao sei se deveria ter isso
         fprintf(stderr, "imesh: kill: usage: kill -<sinal> <pid>\n");
     }
}

int main(){
    char *input;
    char user[_POSIX_LOGIN_NAME_MAX];
    char host[_POSIX_HOST_NAME_MAX];
    char dir[PATH_MAX];
    char prompt[2048];
    getlogin_r(user, sizeof(user));
    gethostname(host, sizeof(host));
    getcwd(dir, sizeof(dir));
    sprintf(prompt, "[%s@%s:%s]$ ", user, host, dir);

    while((input = readline(prompt))!= NULL){
        if(strlen(input)> 0){
            add_history(input);

            if(strcmp(input, "pwd")==0){
                run_pwd();
            }else if(strcmp(input, "date +%s")== 0){  
                run_date();
            }else if(strcmp(input, "exit")==0){
                free(input);
                break;
            }else if(strncmp(input, "kill", 4)==0){
                run_kill(input);
            }else{
                pid_t pid = fork(); // retorna 0 se for o filho (no linux)
                if(pid == 0){
                    if(strncmp(input, "/bin/ls", 7)== 0){
                        char *args[] ={"/bin/ls", "-laF", "--color=never", NULL};
                        execve(args[0], args, NULL);
                    }
                    else if(strncmp(input, "/bin/top", 8)== 0){
                        char *args[] = {"/bin/top", "-b", "-n", "1", "-p", "1", NULL};
                        execve(args[0], args, NULL);
                    }
                    else if(strncmp(input, "./ep1", 5)== 0){
                        run_simulador(input); 
                    }
                    exit(1);
                }else{
                    wait(NULL);
                }
            }
        }
    }
    free(input);

    return 0;
}
