#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "pheap.h"
#include "hash.h"


typedef struct {
      Heap* lot;
      int quantity;
} StoredIngredient;

/**
 * @heap usage: 
 * the key is the expire time of the ingredient
 * the data contains the quantity
 */

/**
 * @hashmap usage:
 * the key is the ingredient
 * the data contains the heap defined before
 */

StoredIngredient* new_ingredient(){
      StoredIngredient* ingredient = (StoredIngredient*)malloc( sizeof(StoredIngredient) );
      ingredient->quantity = 0;
      ingredient->lot = new_heap();
      return ingredient;
}
void trash_expired( Hash* wh, int t ){
      for( int i = 0; i < wh->length; i++ ){
            Bucket* curr = wh->map[i];
            Bucket* prev = wh->map[i];

            while( curr ){
                  StoredIngredient* stored = (StoredIngredient*)curr->data;
                  Heap* heap = stored->lot;
                  int exp_t;

                  while( ( exp_t = heap_get_top(heap)->key ) && exp_t >= t ){
                        Data* tmp = max_heap_pop_max( heap );

                        stored->quantity -= *(int*)tmp->data;
                        free( tmp );
                  }
                  if( !exp_t ){
                        prev->next = curr->next;
                        free( curr );
                        curr = prev;
                        wh->size--;
                  }
                  prev = curr;
                  curr = curr->next;
            }

      }
}


int use_ingredient( Hash* wh, int key, int quantity ){
      
      StoredIngredient* stored = (StoredIngredient*)hash_get( wh, key );

      if( !stored )
            return 0;
      if( stored->quantity < quantity )
            return 0;
      if( stored->quantity == quantity ){
            free( hash_remove( wh, key ) );
            return 1;
      }
      
      stored->quantity -= quantity;


      while( quantity > 0 ){
            Data* top = heap_get_top( stored->lot );

            if( *(int*)top->data > quantity ){
                  *(int*)top->data -= quantity;
                  return 1;
            }else{
                  quantity -= *(int*)top->data;
                  free( min_heap_pop_min(stored->lot) );
            }
      }
      return 1;
}

int check_ingredient( Hash* wh, int key, int qty ){
      StoredIngredient* stored = (StoredIngredient*)hash_get( wh, key );
      return stored && qty <= stored->quantity;
}

void add_ingredient( Hash* wh, int key, int quantity, int exp_t ){

      int* qty = (int*)malloc(sizeof(int));
      StoredIngredient* stored = (StoredIngredient*)hash_get( wh, key );

      *qty = quantity;

      if( !stored ){
            stored = new_ingredient();
            hash_set( wh, key, stored );
      }

      stored->quantity += quantity;
      min_heap_insert( stored->lot, exp_t, qty );
}