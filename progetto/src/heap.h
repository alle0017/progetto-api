#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "struct.h"

#define LEFT(x) (2 * (x) + 1)
#define RIGHT(x) (2 * (x + 1))
#define PARENT(x) ((x - 1) / 2)
#define heap_top( heap ) (heap->array[0]) 

Heap* new_heap( unsigned int height ){
      Heap* h = (Heap*)malloc( sizeof(Heap) );

      h->length = 1 << height;
      h->heap_size = 0;
      h->array = (Data**)malloc( sizeof(Data*)*h->length );

      return h;
}


void max_heapify( Heap* heap, unsigned int n ){

      unsigned int l = LEFT(n);
      unsigned int r = RIGHT(n);

      while( 1 ){
            unsigned int posmax = n;
            if( l < heap->heap_size && heap->array[l]->key > heap->array[n]->key )
                  posmax = l;
            if( r < heap->heap_size && heap->array[r]->key > heap->array[posmax]->key )
                  posmax = r;
            
            if( posmax != n ){
                  Data* tmp = heap->array[posmax];

                  heap->array[posmax] = heap->array[n];
                  heap->array[n] = tmp;

                  n = posmax;
                  l = LEFT(n);
                  r = RIGHT(n);
            }else{
                  return;
            }
      }
}

void max_heapify_ordered( Heap* heap, unsigned int n ){

      unsigned int l = LEFT(n);
      unsigned int r = RIGHT(n);

      while( 1 ){
            unsigned int posmax = n;
            if( l < heap->heap_size && (heap->array[l]->key > heap->array[n]->key || ( heap->array[l]->key == heap->array[n]->key && ((OrderedRecipe*)heap->array[l]->data)->t < ((OrderedRecipe*)heap->array[n]->data)->t )) )
                  posmax = l;
            if( r < heap->heap_size && (heap->array[r]->key > heap->array[posmax]->key || ( heap->array[r]->key == heap->array[posmax]->key && ((OrderedRecipe*)heap->array[r]->data)->t < ((OrderedRecipe*)heap->array[posmax]->data)->t ))  )
                  posmax = r;
            
            if( posmax != n ){
                  Data* tmp = heap->array[posmax];

                  heap->array[posmax] = heap->array[n];
                  heap->array[n] = tmp;

                  n = posmax;
                  l = LEFT(n);
                  r = RIGHT(n);
            }else{
                  return;
            }
      }
}

void min_heapify( Heap* heap, unsigned int n ){

      unsigned int l = LEFT(n);
      unsigned int r = RIGHT(n);

      while( 1 ){
            unsigned int posmax = n;
            if( l < heap->heap_size && heap->array[l]->key < heap->array[n]->key )
                  posmax = l;
            if( r < heap->heap_size && heap->array[r]->key < heap->array[posmax]->key )
                  posmax = r;
            
            if( posmax != n ){
                  Data* tmp = heap->array[posmax];

                  heap->array[posmax] = heap->array[n];
                  heap->array[n] = tmp;

                  n = posmax;
                  l = LEFT(n);
                  r = RIGHT(n);
            }else{
                  return;
            }
      }
}

Data* heap_pop_max_ordered( Heap* heap ){
      if( heap->heap_size < 1 )
            return NULL;
      Data* max = heap->array[0];

      heap->array[0] = heap->array[heap->heap_size - 1];
      heap->heap_size--;
      max_heapify_ordered( heap, 0 );
      return max;
}


Data* heap_pop_max( Heap* heap ){
      if( heap->heap_size < 1 )
            return NULL;
      Data* max = heap->array[0];

      heap->array[0] = heap->array[heap->heap_size - 1];
      heap->heap_size--;
      max_heapify( heap, 0 );
      return max;
}

Data* heap_pop_min( Heap* heap ){
      if( heap->heap_size < 1 )
            return NULL;
      Data* min = heap->array[0];

      heap->array[0] = heap->array[heap->heap_size - 1];
      heap->heap_size--;
      min_heapify( heap, 0 );
      return min;
}

void expand_heap( Heap* heap ){
      heap->length *= 2;
      heap->array = (Data**)realloc(heap->array, sizeof(Data*) * heap->length);
}

void heap_push_max( Heap* heap, unsigned int key, void* data ){
      heap->heap_size++;
      if( heap->heap_size > heap->length )
            expand_heap( heap );
      heap->array[ heap->heap_size - 1 ] = (Data*)malloc(sizeof(Data));
      ( heap->array[ heap->heap_size - 1 ] )->data = data;
      ( heap->array[ heap->heap_size - 1 ] )->key = key;

      unsigned short i = heap->heap_size - 1;

      while( i > 0 && heap->array[PARENT(i)]->key < heap->array[i]->key ){

            Data* tmp = heap->array[PARENT(i)];

            heap->array[PARENT(i)] = heap->array[i];
            heap->array[i] = tmp;
            i = PARENT(i);
      } 
}
void heap_push_min( Heap* heap, unsigned int key, void* data ){
      heap->heap_size++;
      if( heap->heap_size > heap->length )
            expand_heap( heap );
      heap->array[ heap->heap_size - 1 ] = (Data*)malloc(sizeof(Data));
      ( heap->array[ heap->heap_size - 1 ] )->data = data;
      ( heap->array[ heap->heap_size - 1 ] )->key = key;

      unsigned short i = heap->heap_size - 1;

      while( i > 0 && heap->array[PARENT(i)]->key > heap->array[i]->key ){
            
            Data* tmp = heap->array[PARENT(i)];

            heap->array[PARENT(i)] = heap->array[i];
            heap->array[i] = tmp;
            i = PARENT(i);
      } 
}

void order_max_heap( Heap* heap ){
      for( unsigned int i = (unsigned int)heap->heap_size/2; i > 0; i-- ){
            max_heapify( heap, i );
      }
}

void order_min_heap( Heap* heap ){
      for( unsigned int i = (unsigned int)heap->heap_size/2; i > 0; i-- ){
            min_heapify( heap, i );
      }
}

void max_heapify_array( Data* array[], unsigned int n, unsigned int size ){

      unsigned int l = LEFT(n);
      unsigned int r = RIGHT(n);

      while( l < size ){
            if( array[l]->key > array[n]->key && ( r >= size || array[l]->key >= array[r]->key ) ){
                  Data* tmp = array[l];

                  array[l] = array[n];
                  array[n] = tmp;

                  n = l;
                  l = LEFT(n);
                  r = RIGHT(n);

                  continue;
            }else if( r < size && array[r]->key >= array[n]->key ){
                  Data* tmp = array[r];

                  array[r] = array[n];
                  array[n] = tmp;

                  n = r;
                  l = LEFT(n);
                  r = RIGHT(n);

                  continue;
            }
            return;
      }
}
