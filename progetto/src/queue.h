#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "struct.h"

void queue_push( Queue* queue, void* data ){
      List* node = (List*)malloc( sizeof( List ) );

      node->data = data;
      node->next = NULL;
      
      if( !queue->head ){
            queue->head = node;
            queue->tail = node;
      }else{
            queue->tail->next = node;
            queue->tail = node;
      }
}

void* queue_pop( Queue* queue ){
      void* data;

      if( !queue->head )
            return NULL;

      data = queue->head->data;

      if( queue->head == queue->tail ){

            free( queue->tail );

            queue->head = NULL;
            queue->tail = NULL;

            return data;
      }

      List* node = queue->head;

      queue->head = queue->head->next;
      free( node );

      return data;
}