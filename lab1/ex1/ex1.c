#include <stdio.h>
#include <stdlib.h>
// stdlib includes malloc() and free()

// define instructions 
#define PREVIOUS -1
#define NEXT 1
#define DELETE 0

// Node declaration ( Do not edit )
typedef struct NODE
{
    int data;
    struct NODE* previousNode;
    struct NODE* nextNode;
} node;

// Function prototypes
node* findNode(node*,int);
void insertNodeNext(int,int,node*);
void insertNodePrevious(int,int,node*);
void deleteNode(int,node*);
void deleteList(node*);
void printList(node*); // Implemented for you

// Start of main
int main()
{
    int position;
    int instruction;
    int value;
    
    // Declaration of the origin Node ( do not edit )
    node* originNode = (node*)malloc(sizeof(node));
    originNode->previousNode = originNode;
    originNode->nextNode = originNode;
    originNode->data = 0;
    
    // input and processing has been done for you. 
    
    while(scanf("%i%i",&instruction,&position) == 2)
        //scanf() returns an int that corresponds to the number of values scanned. 
    {
        if(instruction == DELETE)
        {
            deleteNode(position,originNode);
        }
        else if(instruction == NEXT)
        {
            scanf("%i",&value);
            insertNodeNext(position,value,originNode);
        }
        else if(instruction == PREVIOUS)
        {
            scanf("%i",&value);
            insertNodePrevious(position,value,originNode);
        }
    }
    printList(originNode);
    deleteList(originNode);
    

    printf("Circular List after delete\n");
    printList(originNode);
    
    free(originNode);
    
    return 0;
}


//Function Implementations 
node* findNode(node* originNode,int position)
{
    node* resNode = originNode;
    for (int i = 0; i < position; i++) {
        resNode = resNode->nextNode;
    }
    return resNode;
}

void insertNodePrevious(int position,int value,node* originNode)
{
    node* currentNode = findNode(originNode, position);
    node* newNode = (node*)malloc(sizeof(node));
    newNode->data = value;
    newNode->previousNode = currentNode->previousNode;
    newNode->previousNode->nextNode = newNode;
    newNode->nextNode = currentNode;
    currentNode->previousNode = newNode;
}

void insertNodeNext(int position,int value,node* originNode)
{
    node* currentNode = findNode(originNode, position);
    node* newNode = (node*)malloc(sizeof(node));
    newNode->data = value;
    newNode->nextNode = currentNode->nextNode;
    newNode->nextNode->previousNode = newNode;
    newNode->previousNode = currentNode;
    currentNode->nextNode = newNode;
}

void deleteNode (int position,node* originNode)
{
    node* currentNode = findNode(originNode, position);
    currentNode->previousNode->nextNode = currentNode->nextNode;
    currentNode->nextNode->previousNode = currentNode->previousNode;
    free(currentNode);
}

void deleteList(node* originNode)
{
    node* iterator = originNode->nextNode;
    while(iterator != originNode)
    {
        node* oldIterator = iterator;
        iterator = iterator->nextNode;
        deleteNode(0, oldIterator);
    }
}

// printList has been implemented for you.
// You may study the functions below to help you implement your own functions. 
void printList(node* originNode)
{
    int count = 0;

    printf("Printing clockwise:\n");
    node* iterator = originNode->nextNode;
    printf("[Pos %i:%i]",0,originNode->data);
    printf("\n   |\n   v\n");
    while(iterator != originNode)
    {
        count++;
        printf("[Pos %i:%i]",count,iterator->data);
        printf("\n   |\n   v\n");
        iterator = iterator->nextNode;
    }
    printf("[Pos %i:%i]",0,originNode->data);   
    
    printf("\nPrinting counter-clockwise:\n");
    iterator = originNode->previousNode;
    printf("[Pos %i:%i]",0,originNode->data);
    printf("\n   |\n   v\n");
    while(iterator!= originNode)
    {
        printf("[Pos %i:%i]",count,iterator->data);
        printf("\n   |\n   v\n");
        iterator = iterator->previousNode;
        count--;
    }
    printf("[Pos %i:%i]\n",0,originNode->data);     

    return;
}

