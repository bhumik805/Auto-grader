#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 
#define MAX_SIZE 3000

// Define the structure for the queue node
struct Node {
    char data[30];
    struct Node* next;
};

// Define the structure for the queue
struct Queue {
    struct Node* front;
    struct Node* rear;
};

// Function to create an empty queue
struct Queue* createQueue() {
    struct Queue* queue = (struct Queue*)malloc(sizeof(struct Queue));
    if (!queue) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    queue->front = queue->rear = NULL;
    return queue;
}

// Function to check if the queue is empty
int isEmpty(struct Queue* queue) {
    return (queue->front == NULL);
}

// Function to get the size of queue
int sizeOf(struct Queue* queue) {
    if (isEmpty(queue)) {
        return 0;
    }
    
    struct Node* temp = queue->front;
    int count = 1;
    while(temp != queue->rear){
        temp = temp->next;
        count++;
    }
    return count;
}

// Function to enqueue an element into the queue
void enqueue(struct Queue* queue, char data[]) {
    
    if(sizeOf(queue) == MAX_SIZE){
        perror("Maximum Queue Size Reached");
        return;
    }

    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    if (!newNode) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    strcpy(newNode->data, data);
    newNode->next = NULL;

    if (isEmpty(queue)) {
        queue->front = queue->rear = newNode;
    } else {
        queue->rear->next = newNode;
        queue->rear = newNode;
    }
}

// Function to dequeue an element from the queue
char * dequeue(struct Queue* queue) {
    if (isEmpty(queue)) {
        printf("Queue is empty\n");
        exit(EXIT_FAILURE);
    }

    char *data = (char *)malloc(sizeof(char)*30) ;
    bzero(data,sizeof data);
    struct Node* temp = queue->front;
    strcpy(data,temp->data);
    queue->front = temp->next;

    free(temp);
    return data;
}

// Function to get the front element of the queue
char * front(struct Queue* queue) {
    if (isEmpty(queue)) {
        printf("Queue is empty\n");
        exit(EXIT_FAILURE);
    }
    return queue->front->data;
}

// Function to free the memory used by the queue
void freeQueue(struct Queue* queue) {
    while (!isEmpty(queue)) {
        dequeue(queue);
    }
    free(queue);
}   


//Function to return position of request or -1 if request DNE
int find(struct Queue* queue,char req_id[]){
    struct Node *temp;
    temp = queue->front;
    
    int foundat=-1;
    int position = 1;
    while (temp)
    {
        if(!strcmp(temp->data,req_id)){
            foundat=position;
            break;
        }
        position+=1;
        temp=temp->next;
    }
    return foundat;
}