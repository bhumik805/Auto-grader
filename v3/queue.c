#include <stdio.h>
#include <stdlib.h>  
#define MAX_SIZE 3000

// Define the structure for the queue node
struct Node {
    int data;
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
void enqueue(struct Queue* queue, int data) {
    
    if(sizeOf(queue) == MAX_SIZE){
        perror("Maximum Queue Size Reached");
        return;
    }

    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    if (!newNode) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    newNode->data = data;
    newNode->next = NULL;

    if (isEmpty(queue)) {
        queue->front = queue->rear = newNode;
    } else {
        queue->rear->next = newNode;
        queue->rear = newNode;
    }
}

// Function to dequeue an element from the queue
int dequeue(struct Queue* queue) {
    if (isEmpty(queue)) {
        printf("Queue is empty\n");
        exit(EXIT_FAILURE);
    }

    struct Node* temp = queue->front;
    int data = temp->data;
    queue->front = temp->next;

    free(temp);
    return data;
}

// Function to get the front element of the queue
int front(struct Queue* queue) {
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


