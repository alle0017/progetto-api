#include <stdio.h>
#include <stdlib.h>
#include "pheap.h"
#include <time.h>


void completeness( int seed ){
      Heap* heap = new_heap();
      int last = 20;
      int flag = 0;
      int array[10];
      int insert[10];
      srand(seed + time(0));
      
      for( int i = 10; i > 0; i--){
            int t = rand() % 16;
            int q = 1 + rand() % 30;
            insert[10 - i] = t;
            max_heap_insert( heap, t, &q );
      }

      for( int i = 10; i > 0; i--){
            Data* max =  max_heap_pop_max( heap );
            array[ 10 - i ] = max->key;
            if(last < max->key){
                  printf("❌ERRORE\n");
                  flag = 1;
            }
            //printf("data: %d key: %d\n", *(int*)(max->data), max->key );
            last = max->key;
      }
      if( flag ){
            printf("OUTPUT\n");
            for(  int i = 0; i < 10; i++ )
                  printf("%d\n", array[i] );
            printf("INPUTS\n");
            for(  int i = 0; i < 10; i++ )
                  printf("%d\n", insert[i] );
      }else{
            printf("✅test passed\n");
      }
}

void correctness( int seed ){
      Heap* heap = new_heap();
      int count[20];

      for( int i = 0; i < 20; i++ ){
            count[i] = 0;
      }

      srand( time(0) + seed );
      for( int i = 0; i < 20; i++ ){
            int t = rand()%20;
            count[t]++;
            max_heap_insert( heap, t, NULL );
      }
      for( int i = 0; i < 20; i++ ){
            count[max_heap_pop_max( heap )->key]--;
      }
      for( int i = 0; i < 20; i++ ){
            if( count[i] ){
                  printf("❌errore");
                  return;
            }
      }
}
int main( int argc, char** argv ){
      for( int i = 0; i < 200000; i++ ){
            correctness( i );
            completeness( i );
      }
      printf("✅test passed\n");

      return 0;
}