#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE 1024
#define MAX_COMMANDS 32
#define MAX_ARGS 64

int parseArgs(char *command, char *args[])
{
    int argc = 0;

    char *arg = strtok(command, " \t");

    while (arg != NULL && argc < MAX_ARGS - 1) {
        args[argc] = arg;
        argc++;

        arg = strtok(NULL, " \t");
    }

    args[argc] = NULL;

    return argc;
}


void runPipeline(char *commands[], int commandCount)
{
    int prevRead = -1;

    for (int i = 0; i < commandCount; i++) {
        int fd[2];

        if (i < commandCount - 1) { // последней команде следующий pipe не нужен
            if (pipe(fd) == -1) {
                perror("pipe");
                exit(1);
            }
        }

        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            exit(1);
        }

        if (pid == 0) {
            if (prevRead != -1) { // если команда не первая, читаем результат предыдущей команды
                if (dup2(prevRead, STDIN_FILENO) == -1) {
                    perror("dup2");
                    _exit(1);
                }
            }
            if (i < commandCount - 1) { // если команда не последняя, её stdout отправляем в следующий pipe
                if (dup2(fd[1], STDOUT_FILENO) == -1) {
                    perror("dup2");
                    _exit(1);
                }
            }
            if (prevRead != -1) {
                close(prevRead); // закрываем ребенка
            }

            if (i < commandCount - 1) {
                close(fd[0]); // fd[0] вообще не нужен
                close(fd[1]); // уже использовали
            }

            char *args[MAX_ARGS];

            if (parseArgs(commands[i], args) == 0) {
                _exit(0);
            }

            execvp(args[0], args);

            perror(args[0]); 
            _exit(1);
        }

        // здесь уже родитель
        if (prevRead != -1) {
            close(prevRead); // ребенок уже унаследовал fd
        }
        if (i < commandCount - 1) {
            close(fd[1]); // родитель писать не собирается
            prevRead = fd[0]; // откуда следующая команда должна читать
        }
    }

    for (int i = 0; i < commandCount; i++) {
        wait(NULL);
    }
}

int main(void)
{
    char line[MAX_LINE];

    while (1) {
        printf("myshell> ");
        fflush(stdout); // выталкиваем всё из буфера stdout

        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("\n");
            break;
        }

        line[strcspn(line, "\n")] = '\0';  // убираем \n

        if (strcmp(line, "myexit") == 0) {
            break;
        }

        if (line[0] == '\0') {
            continue;
        }

        char *commands[MAX_COMMANDS];
        int commandCount = 0;

        char *command = strtok(line, "|");

        while (command != NULL && commandCount < MAX_COMMANDS) {
            commands[commandCount] = command;
            commandCount++;

            command = strtok(NULL, "|");
        }

        runPipeline(commands, commandCount);
    }

    return 0;
}

