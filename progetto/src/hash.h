#pragma once 

#include <stdio.h>
#include <stdlib.h>

#include "struct.h"

#define GET_BUCKET( hash, key ) hash->map[(key)%hash->length]



HashMap* new_hash( unsigned int size ){
      HashMap* hash = (HashMap*)malloc(sizeof(HashMap));

      hash->length = size;
      hash->size = 0;
      hash->map = (Bucket**)calloc(size, sizeof(Bucket*));

      return hash;
}


Data* hash_get( HashMap* hash, unsigned int key ){

      Bucket* bucket = GET_BUCKET( hash, key );


      while( bucket ){
            if( bucket->data->key == key ){
                  return bucket->data;
            }
            bucket = bucket->next;
      }
      return NULL;
}
void resize_hash( HashMap* hash ){

      Bucket** new_array = (Bucket**)calloc( hash->length* 2, sizeof(Bucket*) );

      for( unsigned short i = 0; i < hash->length; i++ ){
            new_array[i] = NULL;
            new_array[i + hash->length] = NULL;
      }
      for( unsigned short i = 0; i < hash->length; i++ ){
            Bucket* bucket = hash->map[i];

            while( bucket ){

                  Bucket* tmp = bucket;

                  bucket = bucket->next;
                  tmp->next = new_array[tmp->data->key%(hash->length*2)];
                  new_array[tmp->data->key%(hash->length*2)] = tmp;

                  /*
                  Bucket* prev = bucket;
                  Bucket* new_bucket = (Bucket*)malloc(sizeof(Bucket));

                  new_bucket->next = new_array[prev->data->key%(hash->length*2)];
                  new_bucket->data = prev->data;

                  new_array[prev->data->key%(hash->length*2)] = new_bucket;
                  bucket = bucket->next;

                  free( prev );*/
            }
      }
      hash->length *= 2;
      free( hash->map );
      hash->map = new_array;
}
void hash_set( HashMap* hash, unsigned int key, void* data ){

      hash->size++;

      if( hash->size/hash->length >= LOAD_FACT ){
            resize_hash( hash );
      }
      Bucket* bucket = GET_BUCKET( hash, key );
      Bucket* new_bucket_node = (Bucket*)malloc( sizeof( Bucket ) );

      new_bucket_node->data = (Data*)malloc( sizeof( Data ) );
      new_bucket_node->data->key = key;
      new_bucket_node->data->data = data;
      new_bucket_node->next = bucket;

      hash->map[key%hash->length] = new_bucket_node;
}

Data* hash_delete( HashMap* hash, unsigned int key ){
      Bucket* bucket = GET_BUCKET( hash, key );
      Bucket* prev = bucket;

      while( bucket ){
            if( bucket->data->key == key ){

                  Data* data = bucket->data;
                  hash->size--;

                  if( prev == bucket ){
                        GET_BUCKET( hash, key ) = bucket->next;
                  }else{
                        prev->next = bucket->next;
                        
                  }

                  free( bucket );
                  return data;
            }
            prev = bucket;
            bucket = bucket->next;
      }
      return NULL;
}