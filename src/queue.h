#pragma once
#include <stdlib.h>
typedef struct QueueNode {
      struct QueueNode *next;
      void* data;
} QueueNode;

typedef struct Queue{
      QueueNode* head;
      QueueNode* tail;
} Queue;


static QueueNode* new_q_node( void* data ){
      QueueNode* node = (QueueNode*)malloc(sizeof(QueueNode));
      
      node->next = NULL;
      node->data = data;
      return node;
}

Queue* q_push( Queue* queue, void* data ){
      QueueNode* node = new_q_node( data );

      if( !queue->head ){
            queue->head = node;
            queue->tail = node;
      }else{
            queue->head->next = node;
      }
      return queue;
}

void* q_pop( Queue* queue ){
      if( !queue->tail )
            return NULL;

      void* data = queue->tail->data;
      QueueNode* tmp = queue->tail;

      queue->tail->data = NULL;
      queue->tail = queue->tail->next;
      free(tmp);

      return data;
}

Queue* new_queue(){
      Queue* queue = (Queue*)malloc(sizeof(Queue));

      queue->head = NULL;
      queue->tail = NULL;

      return queue;
}