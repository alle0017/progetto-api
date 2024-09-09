#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pheap.h"
#include "warehouse.h"
#include "hash.h"
#include "queue.h"

typedef struct IngredientList {
      struct IngredientList* next;
      int qty;
      int ing_key;
} IngredientList;

typedef struct Camion {
      int max_weight;
      int period;
} Camion;

typedef struct Recipe {
      char* name;
      int flag;
      IngredientList* ingredients;
} Recipe;
typedef struct OrderedRecipe {
      int key;
      int qty;
      int t;
} OrderedRecipe;

enum Cmd { 
      ADD_R, 
      REMOVE_R, 
      ORDER, 
      SUPPLY,
      END
};

OrderedRecipe* new_order( int qty, int t, int key ){
      OrderedRecipe* order = (OrderedRecipe*)malloc(sizeof(OrderedRecipe));
      order->qty = qty;
      order->t = t;
      order->key = key;
      return order;
}
IngredientList* ing_push( IngredientList* head, char* ing_name, int qty ){
      IngredientList* list = (IngredientList*)malloc(sizeof(IngredientList));

      list->ing_key = str_hash( ing_name );
      list->qty = qty;
      list->next = head;

      return list;
}

Recipe* new_recipe( char* name, IngredientList* list ){
      Recipe* recipe = (Recipe*)malloc(sizeof(Recipe));
      int i = 0;
      recipe->name = (char*)malloc(sizeof(char)*256);
      while( name[i] != '\0' ){
            recipe->name[i] = name[i];
            i++;
      }
      recipe->ingredients = list;
      recipe->flag = 0;

      return recipe;
}

void fill_camion( Heap* rq, Hash* recipes, int max_weight ){
      Data* curr;

      if( !rq->root ){
            printf("camioncino vuoto\n");
            return;
      }

      while( (curr = heap_get_top( rq )) && curr->key < max_weight ){
            OrderedRecipe* order = (OrderedRecipe*)curr->data;
            Recipe* recipe = (Recipe*)hash_get( recipes, order->key );

            printf("%d %s %d\n", order->t, recipe->name, order->qty );

            recipe->flag--;
            max_weight -= curr->key;

            free( max_heap_pop_max( rq ) );
      }
}

int can_be_prepared( Hash* recipes, Hash* wh, int qty, int recipe_key ){
      Recipe* recipe = (Recipe*)hash_get( recipes, recipe_key );
      IngredientList* list = recipe->ingredients;
      
      while( list ){
            if( !check_ingredient( wh, list->ing_key, (list->qty) * qty ) )
                  return 0;
            list = list->next;
      }
      return 1;
}

int prepare(  Hash* recipes, Hash* wh, Heap* rq, OrderedRecipe* order  ){
      Recipe* recipe = (Recipe*)hash_get( recipes, order->key );
      IngredientList* list = recipe->ingredients;
      int total_weight = 0;

      while( list ){
            use_ingredient( wh, list->ing_key, order->qty * list->qty );
            total_weight += list->qty * order->qty;
            list = list->next;
      }
      max_heap_insert( rq, total_weight, order );
      return total_weight;
}

void check_waiting( Hash* recipes, Hash* wh, Queue* nrq, Heap* rq ){
      QueueNode* curr = nrq->tail;
      QueueNode* prev = nrq->tail;
      while( curr ){
            OrderedRecipe* recipe = (OrderedRecipe*)curr->data;
            if( can_be_prepared( recipes, wh, recipe->qty, recipe->key ) ){
                  prepare( recipes, wh, rq, recipe );
                  prev->next = curr->next;

                  if( curr == nrq->head ){
                        nrq->head = prev;
                  }
                  if( curr == nrq->tail ){
                        nrq->tail = curr->next;
                  }
            }
            prev = curr;
            curr = curr->next;
      }
}

/**
 * the only letter that must be checked 
 * is the first and the third. they can 
 * identify the command
 */
enum Cmd decode_instruction_code( char* cmd ){
      if( !cmd )
            return END;
      switch( cmd[0] ){
            case 'a':
                  return ADD_R;
            case 'o': 
                  return ORDER;
            default: {
                  if( cmd[2] == 'm' )
                        return REMOVE_R;
                  else if( cmd[2] == 'f' )
                        return SUPPLY;
                  return END;
            }
      }
}
enum Cmd get_instruction_code(){
      char cmd[256];
      
      scanf( "%s", cmd );
      return decode_instruction_code( cmd );
}




//return the next command
void add_recipe( Hash* recipes, char* cmd_buffer, char* name ){
      int qty;
      int key;
      IngredientList* head = NULL;

      scanf("%s", name);
      key = str_hash(name);

      if( hash_has( recipes, key ) ){
            printf("ignorato\n");
            while( scanf("%s %d", cmd_buffer, &qty ) == 2 );
            return;
      } 
      
      while( scanf("%s %d", cmd_buffer, &qty ) == 2 ){
            head = ing_push( head, cmd_buffer, qty );
      }
      
      hash_set( recipes, key, new_recipe( name, head ) );
      printf("aggiunta\n");
}


void remove_recipe( Hash* recipes, char* name ){
      Recipe* recipe;
      int key;

      scanf("%s", name);
      key = str_hash( name );
      recipe = (Recipe*)hash_get( recipes, key );

      if( !recipe ){
            printf("non presente\n");
            return;
      }
      if( recipe->flag ){
            printf("ordini in sospeso\n");
            return;
      }

      free( hash_remove( recipes, key ) );
      printf("rimossa\n");
}

void order( Hash* wh, Hash* recipes, Heap* rq, Queue* nrq, char*name, int t ){
      OrderedRecipe* order;
      Recipe* recipe;
      int qty;
      int key;
      
      scanf( "%s %d", name, &qty );
      key = str_hash( name );
      
      recipe = (Recipe*)hash_get( recipes, key );
      
      if( !recipe ){
            printf("rifiutato\n");
            return;
      }

      order = new_order( qty, t, key );
      recipe->flag++;

      if( !can_be_prepared( recipes, wh, qty, key ) ){
            q_push( nrq, order );
      }else{
            prepare( recipes, wh, rq, order );
      }

      printf("accettato\n");
}
//return the next command
void supply( Hash* wh, char* cmd_buffer ){
      int qty;
      int exp_time;

      while( scanf( "%s %d %d", cmd_buffer, &qty, &exp_time ) == 3 ){
            int key = str_hash( cmd_buffer );
            add_ingredient( wh, key, qty, exp_time );
      }
      printf("rifornito\n");
}


void main_loop(){
      Queue* not_ready_queue = new_queue();
      Hash* wh = new_hash();
      Hash* recipes = new_hash();
      Heap* ready_queue = new_heap();
      Camion* camion = (Camion*)malloc(sizeof(Camion));
      char cmd_buffer[256];
      char str_buffer[256];
      int t = 0;
      enum Cmd cmd;

      scanf( "%d %d", &camion->period, &camion->max_weight );

      cmd = get_instruction_code();

      while( 1 ){
            printf("%d ", t);
            
            if( !(t%camion->period) && t > 0 )
                  fill_camion( ready_queue, recipes, camion->max_weight );
            switch( cmd ){
                  case ADD_R: {
                        add_recipe( recipes, cmd_buffer, str_buffer );
                        cmd = decode_instruction_code( cmd_buffer );
                  }break;
                  case REMOVE_R: {
                        remove_recipe( recipes, str_buffer );
                        cmd = get_instruction_code();
                  }break;
                  case SUPPLY: {
                        supply( wh, cmd_buffer );
                        cmd = decode_instruction_code( cmd_buffer );
                        check_waiting( recipes, wh, not_ready_queue, ready_queue );
                  }break;
                  case ORDER: {
                        order( wh, recipes, ready_queue, not_ready_queue, str_buffer, t );
                        cmd = get_instruction_code();
                  }break;
                  default: 
                        return;
            }
            t++;
      }
}


int main( int argc, char** argv ){
      main_loop();
      return 0;
}