/* structure for commands */
struct commands {
    char rm_out[200];
    char rm_diff[200];
    char rm_comp_err[200];
    char rm_run_err[200];
    char rm_test[200];
    char rm_binaries[200];
    char open_test[200];
    char open_comp_err[200];
    char open_run_err[200];
    char open_diff[200];
    char compile[200];
    char run[200];
    char diff[200];
    char create_queue_size[200];
    char rm_queue_size[200];
    char append_queue_size[200];
};

/* structures for Queue */
struct Node {
    char data[70];
    struct Node* next;
};

struct Queue {
    struct Node* front;
    struct Node* rear;
};