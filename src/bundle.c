#include <stdio.h>
#include <stdlib.h>

#define INIT_LEN 20
#define LOAD_FACT 0.75
#define IDEAL_FACT 0.2
#define BASE 10
#define MAX 1
#define MIN 2

typedef struct QueueNode {
      struct QueueNode *next;
      void* data;
} QueueNode;

typedef struct Queue{
      QueueNode* head;
      QueueNode* tail;
} Queue;

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

typedef struct IngredientList {
      struct IngredientList* next;
      int qty;
      int ing_key;
} IngredientList;

typedef struct Camion {
      int max_weight;
      int period;
} Camion;

typedef struct Data {
      int key;
      void* data;
} Data;
typedef struct HNode {
      struct HNode* left;
      struct HNode* right;
      Data* data;
} HNode;

typedef struct Heap {
      int height;
      int leaf_i;
      HNode* root;
} Heap;

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

typedef struct {
      Heap* lot;
      int quantity;
} StoredIngredient;

enum Cmd { 
      ADD_R, 
      REMOVE_R, 
      ORDER, 
      SUPPLY,
      END
};


//----------------------------------------------------------------
// Heap 
//----------------------------------------------------------------



static inline HNode* new_node( int key, void* data ){
      HNode* node = (HNode*)malloc(sizeof(HNode));
      node->data = (Data*)malloc(sizeof(Data));

      node->data->key = key;
      node->data->data = data;
      node->left = NULL;      
      node->right = NULL;
      return node;
}

Heap* new_heap(){
      Heap* heap = (Heap*)malloc(sizeof(Heap));

      heap->height = 0;
      heap->root = NULL;
      heap->leaf_i = 0;
      return heap;
}

static void insert( Heap* heap, int key, void* data, const int min_max ){

      HNode* curr = heap->root;
      int level_increment = 1 << heap->height; //2^height
      int root = (level_increment << 1) - 1; //2^height - 1
      int hook_index = heap->leaf_i << 1; // index-of-last-leaf * 2

      if( heap->leaf_i == root ){
            heap->leaf_i = 0;
            heap->height++;
      }else{
            heap->leaf_i++;
      }

      while( curr ){
            if( ( min_max == MAX && curr->data->key < key ) || ( min_max == MIN && curr->data->key > key ) ){
                  void* tmp_data = curr->data->data;
                  int tmp_key = curr->data->key;

                  curr->data->key = key;
                  curr->data->data = data;
                  key = tmp_key;
                  data = tmp_data;
            }

            if( hook_index == root - level_increment ){ 
                  //is left node
                  curr->left = new_node( key, data );
                  break;
            }else if( hook_index == root + level_increment ){
                  curr->right = new_node( key, data );
                  break;
            }

            if( hook_index > root ){
                  root += level_increment;
                  curr = curr->right;
            }else{
                  root -= level_increment;
                  curr = curr->left;
            }
            level_increment >>= 1;
      }
}


static void cq_insert( Heap* heap, int key, OrderedRecipe* data ){

      HNode* curr = heap->root;
      int level_increment = 1 << heap->height; //2^height
      int root = (level_increment << 1) - 1; //2^height - 1
      int hook_index = heap->leaf_i << 1; // index-of-last-leaf * 2

      if( heap->leaf_i == root ){
            heap->leaf_i = 0;
            heap->height++;
      }else{
            heap->leaf_i++;
      }

      while( curr ){
            if( curr->data->key < key || ( curr->data->key == key && ((OrderedRecipe*)curr->data->data)->t < data->t ) ){
                  OrderedRecipe* tmp_data = (OrderedRecipe*)curr->data->data;
                  int tmp_key = curr->data->key;

                  curr->data->key = key;
                  curr->data->data = data;
                  key = tmp_key;
                  data = tmp_data;
            }

            if( hook_index == root - level_increment ){ 
                  //is left node
                  curr->left = new_node( key, data );
                  break;
            }else if( hook_index == root + level_increment ){
                  curr->right = new_node( key, data );
                  break;
            }

            if( hook_index > root ){
                  root += level_increment;
                  curr = curr->right;
            }else{
                  root -= level_increment;
                  curr = curr->left;
            }
            level_increment >>= 1;
      }
}

static void switch_top_last( Heap* heap ){

      HNode* curr = heap->root;
      int level_increment = 1 << heap->height; //2^height
      int root = (level_increment << 1) - 1; //2^height - 1
      int hook_index = (heap->leaf_i - 1) << 1; // index-of-last-leaf * 2

      if( heap->leaf_i == 0 ){
            heap->leaf_i = (root - 1)>>1;
            heap->height--;
            level_increment >>= 1;
            hook_index = root - 1;
            root = (level_increment << 1) - 1;
      }else{
            heap->leaf_i--;
      }

      while( curr ){

            if( hook_index == root - level_increment ){ 
                  heap->root->data = curr->left->data;
                  free( curr->left );
                  curr->left = NULL;
                  break;
            }else if( hook_index == root + level_increment ){
                  heap->root->data = curr->right->data;
                  free( curr->right );
                  curr->right = NULL;
                  break;
            }

            if( hook_index > root ){
                  root += level_increment;
                  curr = curr->right;
            }else{
                  root -= level_increment;
                  curr = curr->left;
            }
            level_increment >>= 1;
      }
}
     
static void max_heap_fix( HNode* root ){
      HNode* curr = root;
      Data* tmp;

      while( curr && curr->right && curr->left && ( curr->data->key <= curr->left->data->key || curr->data->key <= curr->right->data->key ) ){
            if( curr->right->data->key >= curr->left->data->key ){
                  tmp = curr->right->data;
                  curr->right->data = curr->data;
                  curr->data = tmp;
                  curr = curr->right;
            }else{
                  tmp = curr->left->data;
                  curr->left->data = curr->data;
                  curr->data = tmp;
                  curr = curr->left;
            }
      }
      if( curr->left && !curr->right && curr->data->key <= curr->left->data->key ){

            tmp = curr->left->data;
            curr->left->data = curr->data;
            curr->data = tmp;
            curr = curr->left;
      }
}
void camion_queue_fix( HNode* root ){
      HNode* curr = root;
      Data* tmp;

      while( curr && curr->right && curr->left && ( curr->data->key <= curr->left->data->key || curr->data->key <= curr->right->data->key ) ){
            OrderedRecipe* ct = (OrderedRecipe*)curr->data->data;
            OrderedRecipe* left = (OrderedRecipe*)curr->left->data->data;
            OrderedRecipe* right = (OrderedRecipe*)curr->right->data->data;

            if( curr->right->data->key == curr->left->data->key ){
                  if( ct->t < left->t && ct->t < right->t )
                        return;


                  if( left->t < right->t ){
                        tmp = curr->left->data;
                        curr->left->data = curr->data;
                        curr->data = tmp;
                        curr = curr->left;
                  }else{
                        tmp = curr->right->data;
                        curr->right->data = curr->data;
                        curr->data = tmp;
                        curr = curr->right;
                  }
            }else if( curr->right->data->key > curr->left->data->key ){
                  tmp = curr->right->data;
                  curr->right->data = curr->data;
                  curr->data = tmp;
                  curr = curr->right;
            }else{
                  tmp = curr->left->data;
                  curr->left->data = curr->data;
                  curr->data = tmp;
                  curr = curr->left;
            }
      }
      if( curr->left && !curr->right && (( curr->data->key == curr->left->data->key && ((OrderedRecipe*)curr->data->data)->t > ((OrderedRecipe*)curr->left->data->data)->t )|| curr->data->key < curr->left->data->key )){
            tmp = curr->left->data;
            curr->left->data = curr->data;
            curr->data = tmp;
            curr = curr->left;
      }
}

static void min_heap_fix( HNode* root ){
      HNode* curr = root;
      Data* tmp;

      while( curr && curr->right && curr->left && ( curr->data->key >= curr->left->data->key || curr->data->key >= curr->right->data->key ) ){
            if( curr->right->data->key <= curr->left->data->key ){
                  tmp = curr->right->data;
                  curr->right->data = curr->data;
                  curr->data = tmp;
                  curr = curr->right;
            }else{
                  tmp = curr->left->data;
                  curr->left->data = curr->data;
                  curr->data = tmp;
                  curr = curr->left;
            }
      }
      if( curr->left && !curr->right && curr->data->key > curr->left->data->key ){

            tmp = curr->left->data;
            curr->left->data = curr->data;
            curr->data = tmp;
            curr = curr->left;
      }
}

void min_heap_insert( Heap* heap, int key, void* data ){
      if( !heap->root ){
            heap->root = new_node( key, data );
            return;
      }
      insert( heap, key, data, MIN );
}
void max_heap_insert( Heap* heap, int key, void* data ){
      if( !heap->root ){
            heap->root = new_node( key, data );
            return;
      }
      insert( heap, key, data, MAX );
}
void camion_queue_insert( Heap* heap, int key, OrderedRecipe* data ){
      if( !heap->root ){
            heap->root = new_node( key, data );
            return;
      }
      cq_insert( heap, key, data );
}
Data* min_heap_pop_min( Heap* heap ){
      if( !heap->root )
            return NULL;
      Data* tmp = heap->root->data;

      if( !heap->root->left ){
            
            heap->root = NULL;
            return tmp;
      }
      switch_top_last( heap );
      min_heap_fix( heap->root );
      return tmp;
}
Data* max_heap_pop_max( Heap* heap ){
      if( !heap->root )
            return NULL;
      
      Data* tmp = heap->root->data;

      if( !heap->root->left ){
            heap->root = NULL;
            return tmp;
      }
      switch_top_last( heap );
      max_heap_fix( heap->root );
      return tmp;
}
Data* camion_queue_pop_max( Heap* heap ){
      if( !heap->root )
            return NULL;
      
      Data* tmp = heap->root->data;

      if( !heap->root->left ){
            heap->root = NULL;
            return tmp;
      }
      switch_top_last( heap );
      camion_queue_fix( heap->root );
      return tmp;
}
Data* heap_get_top( Heap* heap ){
      return heap->root? heap->root->data: NULL;
}




//----------------------------------------------------------------
// Hash
//----------------------------------------------------------------



static Bucket* append_to_bucket( Bucket* bucket, int key, void* data ) {
      Bucket* new_node = (Bucket*)malloc( sizeof( Bucket ) );
      new_node->key = key;
      new_node->data = data;
      new_node->next = bucket;
      return new_node;
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

      while( (float)hash->size/hash->length >= IDEAL_FACT )
            hash->length *= 2;

      Bucket** map = (Bucket**)malloc(sizeof(Bucket*)*hash->length);


      for( int i = 0; i < len; i++ ){
            Bucket* next = hash->map[i];
            while( next ){
                  Bucket* prev = next;
                  int key = next->key%hash->length;
                  
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




//----------------------------------------------------------------
// Queue
//----------------------------------------------------------------



static QueueNode* new_q_node( void* data ){
      QueueNode* node = (QueueNode*)malloc(sizeof(QueueNode));
      
      node->next = NULL;
      node->data = data;
      return node;
}

Queue* q_push( Queue* queue, void* data ){
      QueueNode* node = new_q_node( data );
      QueueNode* next = NULL;

      if( !queue->head ){
            queue->head = node;
            queue->tail = node;
      }else{
            queue->head->next = node;
            queue->head = node;
      }

      next = queue->tail;
      while( next ){
            //printf("%d >", ((OrderedRecipe*)next->data)->t);
            next = next->next;
      }
      return queue;
}

void* q_pop( Queue* queue ){
      if( !queue->tail )
            return NULL;

      void* data = queue->tail->data;
      QueueNode* tmp = queue->tail;

      queue->tail->data = NULL;
      queue->tail = queue->tail->next;
      
      free(tmp);

      return data;
}

Queue* new_queue(){
      Queue* queue = (Queue*)malloc(sizeof(Queue));

      queue->head = NULL;
      queue->tail = NULL;

      return queue;
}



//----------------------------------------------------------------
// Warehouse
//----------------------------------------------------------------



/**
 * @heap usage: 
 * the key is the expire time of the ingredient
 * the data contains the quantity
 */

/**
 * @hashmap usage:
 * the key is the ingredient
 * the data contains the heap defined before
 */

StoredIngredient* new_ingredient(){
      StoredIngredient* ingredient = (StoredIngredient*)malloc( sizeof(StoredIngredient) );
      ingredient->quantity = 0;
      ingredient->lot = new_heap();
      return ingredient;
}
int trash_expired( Hash* wh, int t ){
      int next_exp = 2147483647;

      for( int i = 0; i < wh->length; i++ ){
            Bucket* curr = wh->map[i];
            Bucket* prev = wh->map[i];

            while( curr && curr->data ){

                  StoredIngredient* stored = (StoredIngredient*)curr->data;
                  Heap* heap = stored->lot;
                  Data* tmp;

                  while( ( tmp = heap_get_top( heap ) ) && tmp->key >= t ){

                        stored->quantity -= *(int*)tmp->data;
                        free( max_heap_pop_max( heap ) );
                  }

                  if( !tmp ){
                        if( curr == wh->map[i] ){
                              wh->map[i] = curr->next;
                        }else{
                              prev->next = curr->next;
                        }
                        free( curr );
                        curr = prev;
                        wh->size--;
                  }else if( tmp->key < next_exp ){
                        next_exp = tmp->key;
                  }
                  prev = curr;
                  curr = curr->next;
            }

      }
      return next_exp;
}


int use_ingredient( Hash* wh, int key, int quantity ){
      
      StoredIngredient* stored = (StoredIngredient*)hash_get( wh, key );

      if( !stored )
            return 0;
      if( stored->quantity < quantity )
            return 0;
      if( stored->quantity == quantity ){
            free( hash_remove( wh, key ) );
            return 1;
      }
      
      stored->quantity -= quantity;


      while( quantity > 0 ){
            Data* top = heap_get_top( stored->lot );

            if( *(int*)top->data > quantity ){
                  *(int*)top->data -= quantity;
                  return 1;
            }else{
                  quantity -= *(int*)top->data;
                  free( min_heap_pop_min(stored->lot) );
            }
      }
      return 1;
}

int check_ingredient( Hash* wh, int key, int qty ){
      StoredIngredient* stored = (StoredIngredient*)hash_get( wh, key );
      //stored && printf("needed %d usable %d\n", qty, stored->quantity );
      return stored && qty <= stored->quantity;
}

void add_ingredient( Hash* wh, int key, int quantity, int exp_t ){

      int* qty = (int*)malloc(sizeof(int));
      StoredIngredient* stored = (StoredIngredient*)hash_get( wh, key );

      *qty = quantity;

      if( !stored ){
            stored = new_ingredient();
            hash_set( wh, key, stored );
      }

      stored->quantity += quantity;
      //printf("quantity: %d added: %d\n", stored->quantity, quantity );
      min_heap_insert( stored->lot, exp_t, qty );
}



//----------------------------------------------------------------
// MAIN
//----------------------------------------------------------------



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

            free( camion_queue_pop_max( rq ) );
            
      }
}

int can_be_prepared( Hash* recipes, Hash* wh, int qty, int recipe_key ){
      Recipe* recipe = (Recipe*)hash_get( recipes, recipe_key );
      IngredientList* list = recipe->ingredients;
      while( list ){
            if( !check_ingredient( wh, list->ing_key, list->qty * qty ) )
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
      printf("Preparing %d total weight = %d\n", order->t, total_weight );
      camion_queue_insert( rq, total_weight, order );
      return total_weight;
}

void check_waiting( Hash* recipes, Hash* wh, Queue* nrq, Heap* rq ){
      QueueNode* curr = nrq->tail;
      QueueNode* prev = nrq->tail;
      while( curr ){
            OrderedRecipe* recipe = (OrderedRecipe*)curr->data;
            if( can_be_prepared( recipes, wh, recipe->qty, recipe->key ) ){
                  prepare( recipes, wh, rq, recipe );

                  if( curr == nrq->tail ){
                        if( curr == nrq->head ){
                              nrq->head = NULL;
                              nrq->tail = NULL;
                              break;
                        }else{
                              nrq->tail = curr->next;
                              prev = curr->next;
                        }
                  }else if( curr == nrq->head ){
                        nrq->head = prev;
                        nrq->head->next = NULL;
                  }else{
                        prev->next = curr->next;
                        curr = prev;
                  }
                  /*prev->next = curr->next;

                  if( curr == nrq->head ){
                        nrq->head = prev;
                  }
                  if( curr == nrq->tail ){
                        nrq->tail = curr->next;
                  }*/
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
      int _;
      _ = scanf( "%s", cmd );
      _ = decode_instruction_code( cmd );
      return _;
}




//return the next command
void add_recipe( Hash* recipes, char* cmd_buffer, char* name ){
      int qty;
      int key;
      IngredientList* head = NULL;
      char term;

      key = scanf("%s", name);
      key = str_hash(name);

      if( hash_has( recipes, key ) ){
            printf("ignorato\n");
            while( scanf("%s %d%c", cmd_buffer, &qty, &term ) == 3 && term < 'A' );
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

      key = scanf("%s", name);
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
      
      key = scanf( "%s %d", name, &qty );
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
           //printf("enqueue %d\n", t);
      }else{
            prepare( recipes, wh, rq, order );
      }

      printf("accettato\n");
}
//return the next command
int supply( Hash* wh, char* cmd_buffer, int next_exp ){
      int qty;
      int exp_time;

      while( scanf( "%s %d %d", cmd_buffer, &qty, &exp_time ) == 3 ){
            int key = str_hash( cmd_buffer );
            if( exp_time < next_exp )
                  next_exp = exp_time;
            add_ingredient( wh, key, qty, exp_time );
      }
      printf("rifornito\n");
      return next_exp;
}


void main_loop(){
      Queue* not_ready_queue = new_queue();
      Hash* wh = new_hash();
      Hash* recipes = new_hash();
      Heap* ready_queue = new_heap();
      Camion* camion = (Camion*)malloc(sizeof(Camion));
      char cmd_buffer[256];
      char str_buffer[256];
      int t;
      int next_exp = 2147483647;
      enum Cmd cmd;

      t = scanf( "%d %d", &camion->period, &camion->max_weight );
      t = 0;

      cmd = get_instruction_code();

      while( 1 ){
            //printf("%d) ", t);
            if( !(t%camion->period) && t > 0 )
                  fill_camion( ready_queue, recipes, camion->max_weight );
            if( t == next_exp )
                  trash_expired( wh, t );
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
                        next_exp = supply( wh, cmd_buffer, next_exp );
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