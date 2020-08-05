#include <stdio.h>
#include <stdlib.h>
// stdlib includes malloc() and free()

// define instructions
#define PREVIOUS -1
#define NEXT 1
#define DELETE 0
#define INSERTSUBNODE 2
#define COLLAPSE 3 // New instruction compared to ex2.

// Node declarations

typedef struct SUBNODE
{
	int data;
	struct SUBNODE* nextSubNode;
} subNode;

typedef struct NODE
{
	int data;
	struct NODE* previousNode;
	struct NODE* nextNode;
	subNode* subNodeHead;
} node;

// Type for function pointers
typedef void (*generic);
typedef void (*in) (int,node*);
typedef void (*iin) (int,int,node*);
typedef void (*iiin) (int,int,int,node*);

// Function prototypes
node* findNode(node*,int);
subNode* findSubNode(node*,int);
void insertNodeNext(int,int,node*); 
void insertNodePrevious(int,int,node*);
void deleteNode(int,node*);
void printList(node*);
void printSubNodes(node*);
void deleteList(node*);
void insertSubNode(int,int,int,node*);
void deleteAllSubNodes(node*);
void collapseSubNodes(int, node*); 

// Start of main
int main()
{
	int position;
	int instruction;
	int subPosition;
	int value;
	
	// Declaration of the origin Node
	node* originNode = (node*)malloc(sizeof(node));
	originNode->previousNode = originNode;
	originNode->nextNode = originNode;
	originNode->data = 0;
	originNode->subNodeHead = NULL;
	
    generic fptr_arr[] = {
        (generic)deleteNode,
        (generic)collapseSubNodes,
        (generic)insertNodeNext,
        (generic)insertNodePrevious,
        (generic)insertSubNode
    };

	while(scanf("%i%i",&instruction,&position) == 2)
		//scanf() returns an int that corresponds to the number of values scanned. 
	{ 
		if(instruction == DELETE)
		{
			((in)(fptr_arr[0]))(position,originNode);
		}
        else if(instruction == COLLAPSE)
        {
            ((in)(fptr_arr[1]))(position,originNode);
        }
		else if(instruction == NEXT)
		{
			scanf("%i",&value);
			((iin)(fptr_arr[2]))(position,value,originNode);
		}
		else if(instruction == PREVIOUS)
		{
			scanf("%i",&value);
			((iin)(fptr_arr[3]))(position,value,originNode);
        }
        else if(instruction == INSERTSUBNODE)
		{
			scanf("%i%i",&subPosition,&value);
			((iiin)(fptr_arr[4]))(position,subPosition,value,originNode);
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
void collapseSubNodes(int position,node* targetNode)
{
    node* currentNode = findNode(targetNode, position);
    int result = currentNode->data;
    subNode* iterator = currentNode->subNodeHead;
    while(iterator != NULL) {
        result += iterator->data;
        subNode* oldIterator = iterator;
        iterator = iterator->nextSubNode;
        free(oldIterator);
    }
    currentNode->subNodeHead = NULL;
    currentNode->data = result;
}

node* findNode(node* originNode,int position)
{
    node* resNode = originNode;
    for (int i = 0; i < position; i++) {
        resNode = resNode->nextNode;
    }
    return resNode;
}

subNode* findSubNode(node* baseNode,int subPosition)
{
    subNode* resSubNode = baseNode->subNodeHead;
    for (int i = 1; i < subPosition; i++) {
        resSubNode = resSubNode->nextSubNode;
    }
    return resSubNode;
}

void insertSubNode(int position,int subPosition,int value,node* originNode)
{
    node* currentNode = findNode(originNode, position);
    subNode* newSubNode = (subNode*)malloc(sizeof(subNode));    
    newSubNode->data = value;
    if (subPosition == 0) {
        newSubNode->nextSubNode = currentNode->subNodeHead;
        currentNode->subNodeHead = newSubNode;
    } else {
        subNode* currentSubNode = findSubNode(currentNode, subPosition);
        newSubNode->nextSubNode = currentSubNode->nextSubNode;
        currentSubNode->nextSubNode = newSubNode;
    }
}

void insertNodePrevious(int position,int value, node* originNode)
{ 
    node* currentNode = findNode(originNode, position);
    node* newNode = (node*)malloc(sizeof(node));
    newNode->data = value;
    newNode->subNodeHead = NULL;
    newNode->previousNode = currentNode->previousNode;
    newNode->previousNode->nextNode = newNode;
    newNode->nextNode = currentNode;
    currentNode->previousNode = newNode;
}

void insertNodeNext(int position, int value, node* originNode)
{
    node* currentNode = findNode(originNode, position);
    node* newNode = (node*)malloc(sizeof(node));
    newNode->data = value;
    newNode->subNodeHead = NULL;
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
    deleteAllSubNodes(currentNode);
    free(currentNode);
}

void deleteAllSubNodes (node* targetNode)
{
    subNode* iterator = targetNode->subNodeHead;
    while(iterator != NULL) {
        subNode* oldIterator = iterator;
        iterator = iterator->nextSubNode;
        free(oldIterator);
    }
    targetNode->subNodeHead = NULL;
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
    deleteAllSubNodes(originNode);
}

//Print list has been implemented for you. 
void printList(node* originNode)
{
	int count = 0;
	printf("Printing clockwise:\n");
	node* iterator = originNode->nextNode;
	printf("[Pos %i:%i]",0,originNode->data);
	// printing subNodes
	printSubNodes(originNode);
	printf("\n   |\n   v\n");
	while(iterator != originNode)
	{
		count++;
		printf("[Pos %i:%i]",count,iterator->data);
		// printing subNodes
		printSubNodes(iterator);
		printf("\n   |\n   v\n");
		iterator = iterator->nextNode;
	}
	printf("[Pos %i:%i]",0,originNode->data);
	// printing subNodes
	printSubNodes(originNode);		
	
	printf("\nPrinting counter-clockwise:\n");
	iterator = originNode->previousNode;
	printf("[Pos %i:%i]",0,originNode->data);
	// printing subNodes
	printSubNodes(originNode);
	printf("\n   |\n   v\n");
	while(iterator!= originNode)
	{
		printf("[Pos %i:%i]",count,iterator->data);
		// printing subNodes
		printSubNodes(iterator);
		printf("\n   |\n   v\n");
		iterator = iterator->previousNode;
		count--;
	}
	printf("[Pos %i:%i]",0,originNode->data);	
	// printing subNodes
	printSubNodes(originNode);
	printf("\n");

	return;
}

void printSubNodes(node* mainNode)
{
	int count = 0;
	subNode* iterator = mainNode->subNodeHead;
	while(iterator != NULL)
	{
		printf("->[subNode pos %i:%i]",count,iterator->data);
		iterator = iterator->nextSubNode;
		count++;
	}
}
