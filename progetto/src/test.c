#include <stdio.h>
#include <stdlib.h>
#include "heap.h"
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
      int input[10];
      int output[10];

      for( int i = 0; i < 10000000; i++ ){
            Heap* heap = new_heap( 8 );
            unsigned int max_key = 18;
            char flag = 0;

            srand(i + time(0));

            for( int j = 0; j < 10; j++ ){
                  input[j] = rand() % 16;
                  heap_push_max( heap, input[j], NULL );
            }


            for( int j = 0; j < 10; j++ ){
                  Data* max = heap_pop_max( heap );

                  output[j] = max->key;

                  if( !flag && max->key > max_key ){
                        printf("❌ test failed\n");
                        flag = 1;
                  }
                  max_key = max->key;
            }

            if( flag ){
                  printf("INPUT\n");
                  for( int j = 0; j < 10; j++ ){
                        printf("%d - ", input[j] );
                  }
                  printf("\nOUTPUT\n");
                  for( int j = 0; j < 10; j++ ){
                        printf("%d - ", output[j] );
                  }
                  return 0;
            }
      }
      printf("✅ test passed\n");
      return 0;
}
