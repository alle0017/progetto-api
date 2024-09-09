#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "heap.h"
#include "hash.h"
#include "struct.h"
#include "functions.h"

unsigned long check = 0;



/**
 * add an ingredient to the store, if the ingredient already exists it sums the new qty with the overall qty
 * otherwise it adds the new ingredient to the store
 */
void store_add_ingredient( Heap* heap, unsigned int key, unsigned long int data ){
      int lvl_size = 1;

      for( int prev_lvl = 0; prev_lvl < heap->heap_size; lvl_size <<= 1 ){
            char flag = 1;

            for( int i = 0; i < lvl_size && prev_lvl + i < heap->heap_size; i++ ){
                  if( heap->array[prev_lvl + i]->key < key ){
                        flag = 0;
                  }

                  if( heap->array[prev_lvl + i]->key == key ){
                        heap->array[prev_lvl + i]->data = heap->array[prev_lvl + i]->data + data;
                        return;
                  }
            }

            if( flag )
                  break;

            prev_lvl += lvl_size;
      }

      heap_push_min( heap, key, (void*)data );
}
Bucket* new_ingredient( unsigned int key, unsigned long qty, unsigned int expire ){
      Bucket* bucket = (Bucket*)malloc( sizeof(Bucket) );

      bucket->next = NULL;
      bucket->data = (Data*)malloc( sizeof(Data) );
      bucket->data->data = (Ingredient*)malloc( sizeof(Ingredient) );
      bucket->data->key = key;

      ((Ingredient*)bucket->data->data)->tot = qty;
      ((Ingredient*)bucket->data->data)->lot = new_heap( 3 );


      //push ingredient inside heap
      ((Ingredient*)bucket->data->data)->lot->heap_size = 1;
      ((Ingredient*)bucket->data->data)->lot->array[0] = (Data*)malloc( sizeof(Data) );
      ((Ingredient*)bucket->data->data)->lot->array[0]->data = (void*)qty;
      ((Ingredient*)bucket->data->data)->lot->array[0]->key = expire;

      return bucket;
}
/**
 * store an amount of ingredients inside the store
 */
#if GAB

void add_ingredient( HashMap* store, char* ingredient, unsigned long qty, unsigned short expire, HashMap* domain ){

#else
void add_ingredient( HashMap* store, char* ingredient, unsigned long qty, unsigned int expire ){
#endif
      unsigned int key = hash_string( ingredient );

      #if GAB
            if( !hash_get( domain, key ) ){
                  char* buff = (char*)malloc( sizeof(char)*256 );
                  int i;

                  for( i = 0; ingredient[i] != '\0'; i++ ){
                        buff[i] = ingredient[i];
                  }
                  buff[i] = '\0';
                  hash_set( domain, key, buff );
            }
            
      #endif 


      Bucket* bucket = GET_BUCKET(store, key );

      

      if( !bucket ){
            #if DEBUG_SUPPLY
                  printf("bucket doesn't exists\n");
            #endif    
            store->size++;
            if( store->size/store->length >= LOAD_FACT ){
                  resize_hash( store );
            }

            GET_BUCKET( store, key ) = new_ingredient( key, qty, expire );

            #if TRACK_ING
                  if( key == TRACKED ){
                        printf("added %u\n", ((Ingredient*)GET_BUCKET( store, key )->data)->tot);
                  }
            #endif
            return;
      }

      while( bucket ){
            if( bucket->data->key == key ){
                  #if DEBUG_SUPPLY
                        printf("ingredient found\n");
                  #endif      
                  ((Ingredient*)bucket->data->data)->tot += qty;
                  #if TRACK_ING
                        if( key == TRACKED ){
                              printf("added %u\n", ((Ingredient*)bucket->data->data)->tot);
                        }
                  #endif
                  store_add_ingredient( ((Ingredient*)bucket->data->data)->lot, expire, qty );
                  return;
            }

            if( !bucket->next ){
                  #if DEBUG_SUPPLY
                        printf("last of bucket\n");
                  #endif    
                  store->size++;
                  if( store->size/store->length >= LOAD_FACT ){
                        //printf("%u\n", store->length);
                        resize_hash( store );
                  }
                  Bucket* new = new_ingredient( key, qty, expire );
                  #if TRACK_ING
                        if( key == TRACKED ){
                              printf("added %u\n", ((Ingredient*)new->data->data)->tot);
                        }
                  #endif
                  new->next = GET_BUCKET( store, key );
                  GET_BUCKET( store, key ) = new;

                  return;
            }
            bucket = bucket->next;
      }      
}

void use_ingredient( HashMap* store, unsigned int key, unsigned long qty ){
      

      Ingredient* ing = (Ingredient*)(hash_get( store, key )->data);
      #if DEBUG_USE_ING
            printf("[%u needed %lu]", ing->tot, qty );
      #endif
      ing->tot -= qty;
      #if TRACK_ING
            if( key == TRACKED ){
                  printf("removed %u\n", ing->tot);
            }
      #endif

      if( !ing->tot ){
            #if DEBUG_USE_ING
                  for( int i = 0; i < ing->lot->heap_size; i++ )
                        printf("expire at %u,\n", ing->lot->array[i]->key);
            #endif
            free( ing->lot->array );
            free( ing->lot );
            free( ing );
            free( hash_delete( store, key ) );
            
            return;  
      }
      
      while( qty > 0 ){
            if( (unsigned long)heap_top(ing->lot)->data <= qty ){
                  qty -= (unsigned long)heap_top(ing->lot)->data;
                  #if DEBUG_USE_ING
                        printf("expire at %u, ", heap_top(ing->lot)->key);
                  #endif
                  
                  free( heap_pop_min( ing->lot ) );
            }else{
                  #if DEBUG_USE_ING
                        printf("expire at %u,\n", heap_top(ing->lot)->key);
                  #endif
                  heap_top(ing->lot)->data = heap_top(ing->lot)->data - (unsigned long)qty;
                  break;
            }
      }

      /**
       * perform multiple delete operations at once, by exchanging 
       * the deleted item with the last item in the heap, then 
       * call min_heapify once to fix all problems
       *
      int i;

      
      for( i = 0; i < ing->lot->heap_size; i++ ){
            
            #if DEBUG_USE_ING
                  printf("%u qty = %lu,", ing->lot->array[i]->key, (unsigned long)ing->lot->array[i]->data );
            #endif
            if( (unsigned long)ing->lot->array[i]->data <= qty ){
                  qty -= (unsigned long)ing->lot->array[i]->data;
            }else{
                  ing->lot->array[i]->data = ing->lot->array[i]->data - (unsigned long)qty;
                  break;
            }
      }

      #if DEBUG_HEAP_SIZE_PREPARE_RECIPE
            printf(" *%u %d* ", key, ing->lot->heap_size );
      #endif


      for( int j = 0; j < i; j++ ){
            ing->lot->array[j] = ing->lot->array[ing->lot->heap_size - 1];
            ing->lot->heap_size--;
      }

      for( int i = ing->lot->heap_size/2 - 1; i >= 0; i-- )
            min_heapify( ing->lot, i );*/
}

unsigned int get_ingredient_qty( HashMap* store, unsigned int key ){
      
      
      Data* data = hash_get( store, key );

      if( data ){
            //printf(" expires = %d", ((Ingredient*)(data->data))->lot->array[0]->key );
            return ((Ingredient*)(data->data))->tot;
      }

      return 0;
}

/**
 * delete all the elements that are expired and return the next expire time
 */
unsigned int trash_expired_ingredients( HashMap* store, unsigned int t ){

      #if DEBUG_TRASH
            puts("[trash]");
      #endif
      unsigned int size = 0;
      unsigned int exp_t = MAX_U32_SIZE; 


      unsigned int i;
      
      for( i = 0; i < store->length; i++ ){
            if( !store->map[i] ){
                  continue;
            }
            //1925
            Bucket* bucket = store->map[i];
            Bucket* prev = bucket;

            while( bucket ){
                  
                  Heap* heap = ((Ingredient*)bucket->data->data)->lot;

                  #if DEBUG_EXPIRE_TIME
                        for( int i = 0; i < heap->heap_size; i++ ){
                              if( heap->array[i]->key < t ){
                                    printf("\n[ERROR LOG] key: %d index: %d\n", heap->array[i]->key, i );
                                    printf("[");
                                    for( int i = 0; i < heap->heap_size; i++ )
                                          printf("%d,", heap->array[i]->key);
                                    printf("]");
                                    exit(1);
                              }
                        }
                  #endif
                  if( heap_top( heap )->key <= t ){

                        #if DEBUG_TRASH
                              printf("%u %d ", bucket->data->key, ((Ingredient*)bucket->data->data)->tot );
                        #endif
                        ((Ingredient*)bucket->data->data)->tot -= (unsigned long int)heap_top( heap )->data;
                        #if DEBUG_EXPIRE_TIME
                              int debug_t = heap_top( heap )->key;
                        #endif
                        free( heap_pop_min( heap ) );

                        if( !heap->heap_size ){
                              //free( hash_delete( store, bucket->data->key ) );
                              free( heap->array );
                              free( heap );
                              store->size--;


                              if( bucket != prev ){
                                    prev->next = bucket->next;
                                    free( bucket->data->data );
                                    free( bucket->data );
                                    free( bucket );
                                    bucket = prev;

                              }else if( prev->next ){

                                    store->map[i] = prev->next;
                                    prev = prev->next;
                                    free( bucket->data->data );
                                    free( bucket->data );
                                    free( bucket );
                                    bucket = prev;
                                    
                              }else{
                                    store->map[i] = NULL;
                                    free( bucket->data->data );
                                    free( bucket->data );
                                    free( bucket );
                                    bucket = NULL;
                                    break;
                              }
                        }else if( heap_top( heap )->key < exp_t ){
                              exp_t = heap_top( heap )->key;
                              #if DEBUG_EXPIRE_TIME
                                    printf(" %d [trashed %d], ", exp_t, debug_t);
                              #endif
                        }
                        
                  }else if( heap_top( heap )->key < exp_t ){
                        exp_t = heap_top( heap )->key;
                        #if DEBUG_EXPIRE_TIME
                              printf(" %d [non trashed], ", exp_t);
                        #endif
                  }
                  bucket = bucket->next;
                 
            }

            size++;
            if( size == store->size ){
                  return exp_t;
            }
      }
      return exp_t;
      //order_min_heap();
}

