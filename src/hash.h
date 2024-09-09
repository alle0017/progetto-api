#pragma once
#include <stdio.h>
#include <stdlib.h>

#define INIT_LEN 20
#define INIT_HASH 23
#define LOAD_FACT 0.75
#define IDEAL_FACT 0.2
#define BASE 10

typedef struct Bucket {
      struct Bucket* next;
      void* data;
      int key;
} Bucket;

typedef struct {
      Bucket** map;
      int size;
      int length;
} Hash;


static Bucket* append_to_bucket( Bucket* bucket, int key, void* data ) {
      Bucket* new_node = (Bucket*)malloc( sizeof( Bucket ) );
      new_node->key = key;
      new_node->data = data;
      new_node->next = bucket;
      return new_node;
}
static inline int fermat_hash( int hash_size ){
      int i = 1;
      while( i < hash_size ){
            i*=2;
      }
      i++;
      return i;
}


int str_hash( const char* str ){
      int i = 0;
      int hash = 0;

      while( str[i] != '\0' ){
            hash = hash * BASE + ((str[i] - (int)'0')% 32);
            i++;
      } 

      return hash > 0 ? hash: -hash;
}

Hash* new_hash(){
      Hash* hash = (Hash*)malloc(sizeof(Hash));
      hash->size = 0;
      hash->length = INIT_LEN;
      hash->map = (Bucket**)malloc(sizeof(Bucket*)*INIT_LEN);
      for( int i = 0; i < INIT_LEN; i++ ){
            hash->map[i] = NULL;
      }
      return hash;
}

static void resize_hash( Hash* hash ){
      int len = hash->length;

      printf("resize");

      while( (float)hash->size/hash->length >= IDEAL_FACT )
            hash->length *= 2;

      Bucket** map = (Bucket**)malloc(sizeof(Bucket*)*hash->length);


      for( int i = 0; i < len; i++ ){
            Bucket* next = hash->map[i];
            while( next ){
                  Bucket* prev = next;
                  int key = next->key%hash->length;
                  printf("%d >", key);
                  
                  map[key] = append_to_bucket( map[key], next->key, next->data );
                  next = next->next;
                  free(prev);
            }
      }
      free( hash->map );
      hash->map = map;
}

void hash_set( Hash* hash, int key, void* data ){
      int hashed = key%hash->length;
      hash->size++;

      if( (float)hash->size/hash->length >= LOAD_FACT ){
            resize_hash( hash );
      }
      hash->map[hashed] = append_to_bucket( hash->map[hashed], key, data );
}

void* hash_get( Hash* hash, int key ){
      //int hashed = key%20;;
      Bucket* next = hash->map[key%hash->length];
      


      while( next ){
            if( next->key == key )
                  return next->data;
            next = next->next;
      }
      return NULL;
}

int hash_has( Hash* hash, int key ){
      int hashed = key%hash->length;
      Bucket* next = hash->map[hashed];

      while( next ){
            if( next->key == key )
                  return 1;
            next = next->next;
      }
      return 0;
}

void* hash_remove( Hash* hash, int key ){
      int hashed = key%hash->length;
      Bucket* next = hash->map[hashed];
      Bucket* prev = next;

      if( next && next->key == key ){
            void* data = next->data;
            hash->map[hashed] = next->next;
            hash->size--;
            free( next );
            return data;
      }

      while( next ){
            if( next->key == key ){
                  void* data = next->data;
                  prev->next = next->next;
                  hash->size--;
                  free( next );
                  return data;
            }     
            prev = next;
            next = next->next;
      }
      return NULL;
}