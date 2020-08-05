/*************************************
 * Lab 2 Exercise 3
 * Name: She Jiayu
 * Student No: A0188314B
 * Lab Group: 9
 *************************************
 Warning: Make sure your code works on
 lab machine (Linux on x86)
 *************************************/

#include <stdio.h>
#include <fcntl.h>      //For stat()
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>   //for waitpid()
#include <unistd.h>     //for fork(), wait()
#include <string.h>     //for string comparison etc
#include <stdlib.h>     //for malloc()

#define READ_END 0
#define WRITE_END 1
#define MAX_TOKEN_NUM 10 * 2
#define MAX_TOKEN_SIZE 19 * 2

int execute();
char* readInput();
char** extractCommand(char** tokenArr, int start, int end);
int parseCommand(char* command);
char** readTokens(int maxTokenNum, int maxTokenSize, int* readTokenNum, char* buffer);
void freeTokenArray(char** strArr, int size);

int main() {
    //TODO add your code
    int code = 1;
    while (code != 0) {
        code = execute();
    }
    return 0;
}

int execute() {
    int finalCode = 1;
    char* input = readInput();
    int readTokenNum = 0;
    char** tokenArr = readTokens(MAX_TOKEN_NUM * 10, MAX_TOKEN_SIZE, &readTokenNum, input);
    if (readTokenNum > 0) {
        char command[MAX_TOKEN_SIZE];
        //Initialize
        int pipeFd[10][2];
        for (int i = 0; i < 10; i++) {
            pipe(pipeFd[i]);
        }
        int pipeCounter = 0;
        int tokenIter = 0;
        int writeFd = dup(STDOUT_FILENO);
        int readFd = dup(STDIN_FILENO);
        dup2(pipeFd[0][WRITE_END], STDOUT_FILENO);
        //Loop
        for (int i = 0; i < readTokenNum; i++) {
            if (strcmp(tokenArr[i], "|") == 0 || i + 1 == readTokenNum) {
                char** curTokenArr = (i + 1 == readTokenNum)
                    ? extractCommand(tokenArr, tokenIter, i + 1)
                    : extractCommand(tokenArr, tokenIter, i);
                strcpy(command, curTokenArr[0]);
                int code = parseCommand(command);
                //Last command
                if (i + 1 == readTokenNum) {
                    dup2(writeFd, STDOUT_FILENO);
                    close(writeFd);
                }
                if (code == 0) {
                    finalCode = 0;
                    break;
                } else if (code == -1) {
                    finalCode = -1;
                    break;
                } else {
                    int pid = fork();
                    if (pid == 0) {
                        execv(command, curTokenArr);
                    } else {
                        waitpid(pid, NULL, 0);
                    }
                }
                //Clean up
                freeTokenArray(
                    curTokenArr,
                    (i + 1 == readTokenNum) ? i + 1 - tokenIter : i - tokenIter
                );
                curTokenArr = NULL;
                //Next pipe
                if (i != readTokenNum - 1) {
                    dup2(pipeFd[pipeCounter][READ_END], STDIN_FILENO);
                    dup2(pipeFd[pipeCounter + 1][WRITE_END], STDOUT_FILENO);
                    close(pipeFd[pipeCounter][READ_END]);
                    close(pipeFd[pipeCounter][WRITE_END]);
                    pipeCounter++;
                    tokenIter = i + 1;
                }
            }
        }
        if (finalCode == 0) {
            fprintf(stdout, "%s", "Goodbye!\n");
        } else if (finalCode == -1) {
            fprintf(stderr, "%s not found\n", command);
        }
        dup2(readFd, STDIN_FILENO);
        close(readFd);
    }
    //Final clean up
    freeTokenArray(tokenArr, readTokenNum);
    tokenArr = NULL;
    free(input);
    input = NULL;
    return finalCode;
}

char* readInput() {
    fprintf(stdout, "%s", "GENIE>");
    char* input = NULL;
    size_t size = 0;
    getline(&input, &size, stdin);
    return input;
}

char** extractCommand(char** tokenArr, int start, int end) {
    char** curTokenArr = (char **)malloc(sizeof(char *) * MAX_TOKEN_NUM);
    for (int i = 0; i < end - start; i++) {
        curTokenArr[i] = (char *)malloc(sizeof(char *) * MAX_TOKEN_SIZE);
        strcpy(curTokenArr[i], tokenArr[i + start]);
    }
    return curTokenArr;
}

int parseCommand(char* command) {
    struct stat sb;
    if (strcmp(command, "quit") == 0) {
        return 0;
    } else if (stat(command, &sb) == 0) {
        return 1;
    } else {
        char commandPath[MAX_TOKEN_SIZE];
        strcpy(commandPath, "/bin/");
        strcat(commandPath, command);
        strcpy(command, commandPath);
        if (stat(command, &sb) == 0) {
            return 1;
        } else {
            return -1;
        }
    }
}

char** readTokens(int maxTokenNum, int maxTokenSize, int* readTokenNum, char* buffer)
//Tokenize buffer
//Assumptions:
//  - the tokens are separated by " " and ended by newline
//Return: Tokenized substrings as array of strings
//        readTokenNum stores the total number of tokens
//Note:
//  - should use the freeTokenArray to free memory after use!
{
    char** tokenStrArr;
    char* token;
    int i;

    //allocate token array, each element is a char*
    tokenStrArr = (char**) malloc(sizeof(char*) * maxTokenNum);

    //Nullify all entries
    for (int i = 0; i < maxTokenNum; i++) {
        tokenStrArr[i] = NULL;
    }

    token = strtok(buffer, " \n");

    i = 0;
    while (i < maxTokenNum && (token != NULL)) {
        //Allocate space for token string
        tokenStrArr[i] = (char*) malloc(sizeof(char*) * maxTokenSize);

        //Ensure at most 19 + null characters are copied
        strncpy(tokenStrArr[i], token, maxTokenSize - 1);

        //Add NULL terminator in the worst case
        tokenStrArr[i][maxTokenSize-1] = '\0';

        i++;
        token = strtok(NULL, " \n");
    }

    *readTokenNum = i;

    return tokenStrArr;
}

void freeTokenArray(char** tokenStrArr, int size) {
    int i = 0;

    //Free every string stored in tokenStrArr
    for (i = 0; i < size; i++) {
        if (tokenStrArr[i] != NULL) {
            free(tokenStrArr[i]);
            tokenStrArr[i] = NULL;
        }
    }
    //Free entire tokenStrArr
    free(tokenStrArr);

    //Note: Caller still need to set the strArr parameter to NULL
    //      afterwards
}


