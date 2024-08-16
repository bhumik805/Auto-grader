#include "structures.h"

extern pthread_mutex_t queue_lock;
extern pthread_cond_t empty_cond;
extern struct Queue* queue;


/* queue utilities */
struct Queue* createQueue();
int isEmpty(struct Queue* queue);
void enqueue(struct Queue* queue, char data[]);
char * dequeue(struct Queue* queue);
char * front(struct Queue* queue);
int sizeOf(struct Queue* queue);
void freeQueue(struct Queue* queue);
int find(struct Queue* queue, char req_id[]);


/* thread utilities */
void *gradeSubmission(void *td);

/* command utilities */
struct commands* create_commands(char id[]);

// /* recieve file utilities */
// int recieve_file(struct commands* cmds,int sockfd);

// /* process on compilation error */
// void process_compile_error(struct commands* cmds,int sockfd,int res_id);

// /* process on runtime error */
// void process_runtime_error(struct commands* cmds, int sockfd, int res_id);

// /* process on difference in output */
// void process_diff(struct commands* cmds,int sockfd, int res_id);
