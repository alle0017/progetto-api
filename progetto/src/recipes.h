#pragma once 

#include <stdio.h>
#include <stdlib.h>

#include "hash.h"
#include "struct.h"
#include "functions.h"
#include "store.h"

#define COMPRESSED_LEN(x) (((int)(x) + ROUNDING)/MAX_STORED_DIGITS)



RHash* new_recipes_store( unsigned int size ){
      RHash* hash = (RHash*)malloc(sizeof(RHash));

      hash->length = size;
      hash->size = 0;
      hash->map = (RBucket**)calloc(size, sizeof(RBucket*));

      return hash;
}


Recipe* recipes_get( RHash* recipes, char* recipe ){
      char len;
      unsigned long* compressed = compress( recipe, &len );
      int length = COMPRESSED_LEN( len );

      RBucket* bucket = GET_BUCKET( recipes, compressed[ length - 1 ] );
      

      while( bucket ){
            if(  len == bucket->recipe->length && compare_compressed_strings( bucket->recipe->compressed, compressed, length ) ){
                  return bucket->recipe;
            }
            bucket = bucket->next;
      }
      return NULL;
}
void resize_recipes_hash( RHash* hash ){
      RBucket** new_array = (RBucket**)calloc( hash->length* 2, sizeof(RBucket*) );

      for( unsigned short i = 0; i < hash->length; i++ ){
            new_array[i] = NULL;
            new_array[i + hash->length] = NULL;
      }

      for( unsigned short i = 0; i < hash->length; i++ ){
            RBucket* bucket = hash->map[i];

            while( bucket ){
                  RBucket* tmp = bucket;

                  bucket = bucket->next;
                  tmp->next = new_array[tmp->recipe->compressed[ COMPRESSED_LEN( tmp->recipe->length ) - 1 ]%(hash->length*2)];;
                  new_array[tmp->recipe->compressed[ COMPRESSED_LEN( tmp->recipe->length ) - 1 ]%(hash->length*2)] = tmp;
            }
      }
      hash->length *= 2;
      free( hash->map );
      hash->map = new_array;
}
void recipes_set( RHash* hash, Recipe* recipe ){

      hash->size++;

      if( hash->size/hash->length >= LOAD_FACT ){
            resize_recipes_hash( hash );
      }

     
      RBucket* new_node = (RBucket*)malloc( sizeof( RBucket ) );
      

      new_node->next = GET_BUCKET( hash, recipe->compressed[ COMPRESSED_LEN(recipe->length) - 1 ] );
      new_node->recipe = recipe;
      GET_BUCKET( hash, recipe->compressed[ COMPRESSED_LEN(recipe->length) - 1 ] ) = new_node;
}

void recipes_delete( RHash* hash, char* recipe ){
      char len;
      unsigned long* compressed = compress( recipe, &len );
      RBucket* bucket = GET_BUCKET( hash, compressed[ COMPRESSED_LEN( len ) - 1 ] );
      RBucket* prev = bucket;

      while( bucket ){

            if( bucket->recipe->length == len && compare_compressed_strings( compressed, bucket->recipe->compressed, COMPRESSED_LEN( len ) ) ){
                  hash->size--;

                  if( prev == bucket ){
                        hash->map[compressed[ COMPRESSED_LEN( len ) - 1 ]%hash->length] = bucket->next;
                  }else{
                        prev->next = bucket->next;
                  }
                  
                  List* node = bucket->recipe->ingredients;

                  while( node ){

                        List* tmp = node;

                        node = node->next;

                        free( tmp->data );
                        free( tmp );
                  }

                  free( bucket->recipe->compressed );
                  free( bucket->recipe );
                  free( bucket );
                  return;
            }
            prev = bucket;
            bucket = bucket->next;
      }
}

unsigned short recipes_can_be_prepared( Recipe* recipe, HashMap* store, unsigned int qty ){
      
      List* node = recipe->ingredients;

      #if DEBUG_PROCESSING_ORDERS
            printf(" recipe qty %d [", qty);
      #endif

      while( node ){
            unsigned int tmp = get_ingredient_qty( store, ((RNeededIngredient*)(node->data))->key );
            
            #if DEBUG_PROCESSING_ORDERS
                  printf("( key = %u qty = %d, stored = %d)", ((RNeededIngredient*)(node->data))->key, ((RNeededIngredient*)(node->data))->qty,tmp );
            #endif

            
            if( tmp < ((RNeededIngredient*)(node->data))->qty * qty ){
                  #if DEBUG_ING_QTY
                        printf("key = %u qty = %d needed = %d\n", ((RNeededIngredient*)(node->data))->key, tmp, ((RNeededIngredient*)(node->data))->qty * qty );
                  #endif
                  #if DEBUG_PROCESSING_ORDERS
                        printf("]");
                  #endif
                  return 0;
            }
            node = node->next;
      }
      #if DEBUG_PROCESSING_ORDERS
            printf("]");
      #endif
      return 1;
}