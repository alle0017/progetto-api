#include <stdio.h>
#include <stdlib.h>
#include "pheap.h"
#include <time.h>
/**
 * test sugli HEAP
 * PRINT
14
11
8
13
8
4
4
3
3
0
*INSERT
11
4
3
8
4
14
3
0
8
13
 */

int main( int argc, char** argv ){
      int supercont = 0;
      while(supercont < 10000000){
            Heap* heap = new_heap();
            int last = 20;
            int flag = 0;
            int array[10];
            int insert[10];
            srand(supercont + time(0));
            
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
                  break;
            }else{
                  printf("✅test passed\n");
            }
            supercont++;
      }
}