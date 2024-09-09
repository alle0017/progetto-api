#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "store.h"
#include "functions.h"
#include "heap.h"
#include "hash.h"
#include "struct.h"
#include "commands.h"
#include "recipes.h"

int _compare(const void *a, const void *b) {
    // Cast the pointers to strings
    const char* str_a = *(const char**)a;
    const char* str_b = *(const char**)b;

    // Compare the entire strings
    return strcmp(str_a, str_b);
}

void print_hash( HashMap* store, HashMap* domain ){
      char* ordered[store->size];
      int j = 0;

      for( int i = 0; i < store->length; i++ ){
            if( !store->map[i] ){
                  continue;
            }
            //1925
            Bucket* bucket = store->map[i];

            while( bucket ){
                  ordered[j] = (char*)malloc( sizeof( char ) * 267 );
                  sprintf(ordered[j], "  %s %d", (char*)hash_get( domain, bucket->data->key )->data, ((Ingredient*)bucket->data->data)->tot);
                  j++;
                  bucket = bucket->next;
            }
            
            if( j == store->size )
                  break;
      }
      qsort( ordered, store->size, sizeof(char*), _compare  );
      for( int i = 0; i < store->size; i++ ){
            puts( ordered[i] );
            free( ordered[i] );
      }
}

int main( int argc, char **argv ){
      HashMap* store = new_hash( 10 );
      #if GAB
            HashMap* domain = new_hash( 10 );
      #endif

      //printf("%u\n", hash_string( "8RblXk7G" ) );
      RHash* recipes = new_recipes_store( 10 );

      Heap* ready = new_heap( 4 );
      Queue* waiting = (Queue*)calloc( 1, sizeof(Queue) );


      char buff[17];
      unsigned int camion_t;
      unsigned int camion_max_w;
      unsigned int t = 0;
      unsigned int next_exp = MAX_U32_SIZE;

      setvbuf(stdout, NULL, _IOFBF, 8192);

      get_number( &camion_t );
      get_number( &camion_max_w );

      

      while( get_string( buff ) != END_OF_FILE ){
            
            #if DEBUG
                  printf("%d - ", t );
            #endif
            if( t == next_exp ){
                  next_exp = trash_expired_ingredients( store, t );
                  #if DEBUG_EXPIRE_TIME
                        printf("next exp %d [trash]\n", next_exp);
                  #endif
            }
            #if GAB
                  printf("Printing depot (time %d): %d ingredients\n", t, store->size );
                  print_hash( store, domain );
            #endif
            if( !(t%camion_t) && t > 0 ){
                  fill_camion( ready, camion_max_w );
            }
            switch( buff[2] ){
                  case REMOVE: {
                        remove_recipe( recipes );
                  }break;
                  case ADD: {
                        add_recipe( recipes );
                  }break;
                  case ORDER: { 
                        order( waiting, ready, recipes, store, t, camion_max_w );
                  }break;
                  case SUPPLY: {
                        #if GAB
                              next_exp = supply( store, t, next_exp, domain );
                        #else
                              next_exp = supply( store, t, next_exp );
                        #endif
                        #if DEBUG_EXPIRE_TIME
                              printf("next exp %d [supply]\n", next_exp);
                        #endif
                        check_waiting( waiting, ready, recipes, store, camion_max_w, t );
                  }break;
                  default: {
                        if( !(t%camion_t) && t > 0 ){
                              fill_camion( ready, camion_max_w );
                        }
                        return 0;
                  }
                  
            }
            t++;
      }
      if( !(t%camion_t) && t > 0 ){
            fill_camion( ready, camion_max_w );
      }
      return 0;
}