/*************************************
 * Lab 2 Exercise 2
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

#define MAX_TOKEN_NUM 10 * 2
#define MAX_TOKEN_SIZE 19 * 2

int execute();
char* readInput();
int parseCommand(char* command);
void removeQuote(char** tokenArr, int readTokenNum);
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
    int code = 1;
    char* input = readInput();
    int readTokenNum = 0;
    char** tokenArr = readTokens(MAX_TOKEN_NUM, MAX_TOKEN_SIZE, &readTokenNum, input);
    if (readTokenNum > 0) {
        char command[MAX_TOKEN_SIZE];
        strcpy(command, tokenArr[0]);
        code = parseCommand(command);
        if (code == 0) {
            fprintf(stdout, "%s", "Goodbye!\n");
        } else if (code == -1) {
            fprintf(stderr, "%s not found\n", command);
        } else {
            int pid = fork();
            if (pid == 0) {
                removeQuote(tokenArr, readTokenNum);
                execv(command, tokenArr);
            } else {
                waitpid(pid, NULL, 0);
            }
        }
    }
    //Clean up
    freeTokenArray(tokenArr, readTokenNum);
    tokenArr = NULL;
    free(input);
    input = NULL;
    return code;
}

char* readInput() {
    fprintf(stdout, "%s", "GENIE>");
    char* input = NULL;
    size_t size = 0;
    getline(&input, &size, stdin);
    return input;
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

void removeQuote(char** tokenArr, int readTokenNum) {
    for (int i = 0; i < readTokenNum; i++) {
        int counter = 0;
        int len = strlen(tokenArr[i]);
        for (int j = 0; j < len; j++) {
            if (tokenArr[i][j] != '"') tokenArr[i][counter++] = tokenArr[i][j];
        }
        tokenArr[i][counter] = '\0';
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


