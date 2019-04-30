#include "queue.h"

//******************************************************************************
//      STRUCTS
//******************************************************************************
typedef struct Node* node_t;
struct Node {
    void* data;
    node_t next;
    node_t prev;
};

struct Queue {
    int size;
    node_t head;
    node_t tail;
};

//******************************************************************************
//      PRIVATE FUNCTIONS
//******************************************************************************
static node_t node_create (void* new_data)
{
    node_t N = (node_t)malloc(sizeof(struct Node));
    N->data = new_data;
    N->next = NULL;
    N->prev = NULL;
    return N;
}

//******************************************************************************
//      PUBLIC FUNCTIONS
//******************************************************************************
queue_t queue_create()
{
    queue_t Q = (queue_t)malloc(sizeof(struct Queue));
    Q->size = 0;
    Q->head = NULL;
    Q->tail = NULL;
    return Q;
}

int queue_destroy(queue_t Q)
{
    if(!Q)
        return -1;

    while(Q->head) {
        node_t next_node = Q->head->next;
        free(Q->head);
        Q->head = next_node;
    }
    return 0;
    free(Q);
}

int queue_enqueue(queue_t Q, void* data)
{
    if(!Q || !data)
        return -1;

    node_t N = node_create(data);

    if(!queue_count(Q))
        Q->tail = N;
    else
        Q->head->prev = N;

    N->next = Q->head;
    Q->head = N;

    Q->size++;
    return 0;
}

int queue_dequeue(queue_t Q, void** return_data)
{
    if(!Q || !queue_count(Q))
        return -1;

    if(return_data)
        *return_data = Q->tail->data;
    node_t new_tail = Q->tail->prev;

    if(queue_count(Q) > 1)
    {
        new_tail->next = NULL;
        free(Q->tail);
        Q->tail = new_tail;
    }
    else
        Q->head = Q->tail = NULL;

    Q->size--;
    return 0;
}

int queue_count(queue_t Q)
{
    if(!Q)
        return -1;

    return Q->size;
}

int queue_test()
{
    queue_t Q = queue_create();

    int* front;
    int a = 1;
    assert(!queue_enqueue(Q, &a));
    assert(!queue_dequeue(Q, (void*)&front));
    assert(*front == a);
    assert(queue_count(Q) == 0);

    int b = 2;
    int c = 3;
    assert(!queue_enqueue(Q, &a));
    assert(!queue_enqueue(Q, &b));
    assert(!queue_enqueue(Q, &c));

    assert(queue_count(Q) == 3);
    assert(!queue_dequeue(Q, (void*)&front));
    assert(*front == a);
    assert(queue_count(Q) == 2);
    assert(!queue_dequeue(Q, (void*)&front));
    assert(*front == b);
    assert(!queue_dequeue(Q, (void*)&front));
    assert(*front == c);
    assert(queue_count(Q) == 0);

    assert(!queue_enqueue(Q, &c));
    assert(!queue_dequeue(Q, NULL));
    assert(queue_dequeue(Q, NULL));
    assert(!queue_enqueue(Q, &c));
    assert(queue_dequeue(NULL, (void*)&front));
    assert(queue_dequeue(Q, (void*)&front));
    assert(queue_count(Q) == 0);

    assert(!queue_destroy(Q));
    return 0;
}
