#pragma once 

#include <stdio.h>
#include <stdlib.h>

#include "struct.h"
#include "hash.h"
#include "heap.h"
#include "store.h"
#include "recipes.h"
#include "queue.h"

#if GAB 
unsigned int supply( HashMap* store, unsigned int t, unsigned int next_exp, HashMap* domain ){
#else 
unsigned int supply( HashMap* store, unsigned int t, unsigned int next_exp ){
#endif
      char buffer[256];
      unsigned int qty;
      unsigned int exp_time;

      while( get_string( buffer ) == CONTINUE && get_number( &qty ) == CONTINUE && get_number( &exp_time ) == CONTINUE ){
            
            if( exp_time <= t )
                  continue;

            #if DEBUG_SUPPLY_NAME
                  printf("%s,", buffer);
            #endif

            
            #if GAB
                  add_ingredient( store, buffer, qty, exp_time, domain );
            #else
                  add_ingredient( store, buffer, qty, exp_time );
            #endif


            if( next_exp > exp_time ){
                  next_exp = exp_time;
            }

            
      }
      if( exp_time > t ){
            
            #if GAB
                  add_ingredient( store, buffer, qty, exp_time, domain );
            #else
                  add_ingredient( store, buffer, qty, exp_time );
            #endif

            if( next_exp > exp_time ){
                  next_exp = exp_time;
            }
      }
      #if DEPLOY
            fputs( SUPPLY_SUCCESS, stdout );
      #endif
      return next_exp;
}
void add_recipe( RHash* recipes ){

      char buffer[256];
      unsigned int qty;

      #if DEBUG_ING_OCC
            HashMap* map = new_hash( 10 );
      #endif

      get_string( buffer );

      if( recipes_get( recipes, buffer ) ){

            while( get_string( buffer ) == CONTINUE && get_number( &qty ) == CONTINUE );
            #if DEPLOY
                  fputs( RECIPE_ADDED_FAILED, stdout );
            #endif
            return;
      }
      #if DEPLOY
            fputs( RECIPE_ADDED_SUCCESS, stdout );
      #endif
      
      Recipe* recipe = (Recipe*)malloc( sizeof( Recipe ) );

      recipe->ingredients = NULL;

      recipe->compressed = compress( buffer, &recipe->length );
      recipe->weight = 0;
      recipe->waiting_orders = 0;
      #if DEBUG_WEIGHT
            printf(" RECIPE %s",buffer );
      #endif
            



      while( get_string( buffer ) == CONTINUE && get_number( &qty ) == CONTINUE ){

            #if DEBUG_RECIPE
                  printf("%d,",qty );
            #endif
            RNeededIngredient* ingredient = (RNeededIngredient*)malloc( sizeof( RNeededIngredient ) );

            ingredient->key = hash_string( buffer );
            ingredient->qty = qty;
            recipe->weight += qty;

            #if DEBUG_ING_OCC
                  if( hash_get( map, ingredient->key ) ){
                        puts("ERRORE");
                        exit(1);
                  }else{
                        printf("%u,", ingredient->key);
                  }
                  hash_set( map, ingredient->key, NULL );
            #endif

            List* node = (List*)malloc( sizeof( List ) );
            node->next = recipe->ingredients;
            node->data = ingredient;
            recipe->ingredients = node;
      }
      #if DEBUG_RECIPE
            List* node_tmp = recipe->ingredients;
            printf("\n[");
            while( node_tmp ){
                  printf("%d,", ((RNeededIngredient*)node_tmp->data)->qty );
                  node_tmp = node_tmp->next;
            }
            printf("]\n");
      #endif
      #if DEBUG_WEIGHT
            printf(" WEIGHT %d", recipe->weight);
      #endif

      RNeededIngredient* ingredient = (RNeededIngredient*)malloc( sizeof( RNeededIngredient ) );

      ingredient->key = hash_string( buffer );
      ingredient->qty = qty;
      recipe->weight += qty;

      List* node = (List*)malloc( sizeof( List ) );
      node->next = recipe->ingredients;
      node->data = ingredient;
      recipe->ingredients = node;

      recipes_set( recipes, recipe );
}
void remove_recipe( RHash* recipes ){
      char buff[256];

      get_string( buff );

      Recipe* recipe = recipes_get( recipes, buff );

      if( !recipe ){
            #if DEPLOY
                  fputs( RECIPE_REMOVED_NOT_FOUND, stdout );
            #endif
            return;
      }

      if( recipe->waiting_orders > 0 ){
            #if DEPLOY
                  fputs( RECIPE_REMOVED_WAITING_ORDERS, stdout );
            #endif
            return;
      }
      #if DEPLOY
            fputs( RECIPE_REMOVED_SUCCESS, stdout );
      #endif
      recipes_delete( recipes, buff );
}
void order( Queue* waiting, Heap* ready, RHash* recipes, HashMap* store, unsigned int t, unsigned int camion_max_w ){
      unsigned int qty;
      char buff[256];

      get_string( buff );
      get_number( &qty );

      Recipe* recipe = recipes_get( recipes, buff );

      if( !recipe ){
            #if DEPLOY
                  fputs( ORDER_FAILED, stdout );
            #endif
            return;
      }

      List* ingredients = recipe->ingredients;
      OrderedRecipe* order = (OrderedRecipe*)malloc( sizeof( OrderedRecipe ) );

      order->recipe = recipe;
      order->t = t;
      order->qty = qty;

      recipe->waiting_orders++;
      #if DEPLOY
            fputs( ORDER_SUCCESS, stdout );
      #endif

      if( !recipes_can_be_prepared( recipe, store, qty ) ){
            #if DEBUG_PROCESSING_ORDERS
                  printf("stored for later processing\n");
            #endif
            queue_push( waiting, order );
      }else{
            #if DEBUG_PROCESSING_ORDERS
                  printf("immediate processing\n");
            #endif
            #if GAB 
                  printf("Cooking (time %d): %d of %s ordered at %d\n", t, qty, buff, t );
            #endif
            while( ingredients ){
                  use_ingredient( store, ((RNeededIngredient*)ingredients->data)->key, ((RNeededIngredient*)ingredients->data)->qty * qty );
                  ingredients = ingredients->next;
            }
            if( camion_max_w >= recipe->weight * qty ){
                  heap_push_min( ready, t, order );
            }else{
                  free( order );
            }
      }
}
void fill_camion( Heap* ready, unsigned int camion_max_w ){

      if( !ready->heap_size ){
            #if DEPLOY
                  fputs( CAMION_EMPTY, stdout );
            #endif
            return;
      }

      Heap* output = new_heap( 5 );
      HashMap* names = new_hash( 10 );

      #if DEBUG_READY_RECIPE
            for( int i = 0; i < ready->heap_size; i++ ){
                  printf("%u > ", ready->array[i]->key);
            }
      #endif
      
      while( camion_max_w && ready->heap_size ){
            if( camion_max_w >= ((OrderedRecipe*)heap_top( ready )->data)->qty * ((OrderedRecipe*)heap_top( ready )->data)->recipe->weight ){
                  camion_max_w -= ((OrderedRecipe*)heap_top( ready )->data)->qty * ((OrderedRecipe*)heap_top( ready )->data)->recipe->weight;
                  
                  heap_push_max( 
                        output, 
                        ((OrderedRecipe*)heap_top( ready )->data)->qty * ((OrderedRecipe*)heap_top( ready )->data)->recipe->weight,
                        heap_pop_min( ready )->data 
                  );
                  
            }else{
                  break;
            }
      }

      Data* tmp;
      while( ( tmp = heap_pop_max_ordered( output ) ) ){

            Data* data = hash_get( names, (unsigned int)(unsigned long)((OrderedRecipe*)tmp->data)->recipe );
            char* name;

            if( !data ){
                  name = decompress( ((OrderedRecipe*)tmp->data)->recipe->compressed, ((OrderedRecipe*)tmp->data)->recipe->length );
                  hash_set( 
                        names, 
                        (unsigned int)(unsigned long)((OrderedRecipe*)tmp->data)->recipe, 
                        name 
                  );
            }else{
                  name = (char*)data->data;
            }
            #if DEPLOY
                  printf("%u %s %u\n", ((OrderedRecipe*)tmp->data)->t, name, ((OrderedRecipe*)tmp->data)->qty );
            #endif

            
            ((OrderedRecipe*)tmp->data)->recipe->waiting_orders--;

            free( tmp->data );
            free( tmp );
      }

      free( output->array );
      free( output );

      //free names map
      for( int i = 0; i < names->length; i++ ){
            Bucket* bucket = names->map[i];
            
            while( bucket ){
                  Bucket* tmp = bucket;

                  bucket = bucket->next;
                  free( tmp->data );
                  free( tmp );
                  names->size--;
            }

            if( !names->size ){
                  free( names->map );
                  free( names );
                  return;
            }
      }

      free( names->map );
      free( names );

}

void check_waiting( Queue* waiting, Heap* ready, RHash* recipes, HashMap* store, unsigned int camion_max_w, unsigned int t ){
      List* node = waiting->head;
      List* prev = node;

      while( node ){

            
            if( recipes_can_be_prepared( ((OrderedRecipe*)node->data)->recipe, store, ((OrderedRecipe*)node->data)->qty ) ){
                  #if GAB 
                        printf("Cooking (time %d): %d of %s ordered at %d\n", t, ((OrderedRecipe*)node->data)->qty, decompress( ((OrderedRecipe*)node->data)->recipe->compressed, ((OrderedRecipe*)node->data)->recipe->length ), ((OrderedRecipe*)node->data)->t );
                  #endif
                  List* ingredients = ((OrderedRecipe*)node->data)->recipe->ingredients;
                  
                  while( ingredients ){
                        use_ingredient( store, ((RNeededIngredient*)ingredients->data)->key, ((RNeededIngredient*)ingredients->data)->qty * ((OrderedRecipe*)node->data)->qty );
                        ingredients = ingredients->next;
                  }
                  
                  if( camion_max_w >= ((OrderedRecipe*)node->data)->qty * ((OrderedRecipe*)node->data)->recipe->weight )
                        heap_push_min( ready, ((OrderedRecipe*)node->data)->t, node->data );
                  else{
                        free( node->data );
                  }

                  if( node == prev ){
                        prev = node->next;

                        if( waiting->tail == waiting->head ){
                              waiting->tail = NULL;
                              waiting->head = NULL;
                              free( node );
                              return;
                        }else{
                              waiting->head = prev;
                        }
                        free( node );
                        node = prev;

                        continue;
                  }else if( !node->next ){
                        waiting->tail = prev;
                        prev->next = NULL;
                        free( node );
                        node = prev;
                  }else{
                        prev->next = node->next;
                        free( node );
                        node = prev;
                  }
            }
            #if DEBUG_PROCESSING_ORDERS
                  else{
                        printf("discarded %d\n",((OrderedRecipe*)node->data)->t );
                  }
                       
            #endif
            prev = node;     
            node = node->next;
      }
}