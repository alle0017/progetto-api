#include <stdio.h>
#include <stdlib.h>


#define MAX_UINT_ENCRYPT 5
#define CHAR_LIMIT 23
#define COMPRESS_BASE 100
#define MAX_STORED_DIGITS 9
#define CHAR_OFFSET 23
#define INIT_LEN 20
#define LOAD_FACT 0.75
#define IDEAL_FACT 0.2
#define BASE 10
#define MAX 1
#define MIN 2
#define MAX_U32_SIZE 4294967295
#define ROUNDING 8
#define NUMBER 1
#define STR 2
#define CONTINUE 4
#define END_ROW 5
#define END_OF_FILE 6
#define REMOVE 'm'
#define REMOVE_CHECK 'r'
#define ADD 'g'
#define ADD_CHECK 'a'
#define ORDER 'd'
#define ORDER_CHECK 'o'
#define SUPPLY 'f'
#define SUPPLY_CHECK 'r'

#define SUPPLY_SUCCESS "rifornito"

#define RECIPE_ADDED_SUCCESS "aggiunta"
#define RECIPE_ADDED_FAILED "ignorato"

#define RECIPE_REMOVED_SUCCESS "rimossa"
#define RECIPE_REMOVED_NOT_FOUND "non presente"
#define RECIPE_REMOVED_WAITING_ORDERS "ordini in sospeso"

#define ORDER_SUCCESS "accettato"
#define ORDER_FAILED "rifiutato"

#define CAMION_EMPTY "camioncino vuoto"


typedef struct List {
      void* data;
      struct List* next;
} List;

typedef struct Queue {
      List* head;
      List* tail;
} Queue;

typedef struct Data {
      unsigned int data; 
      unsigned int key;
} Data;

typedef struct Heap {
      Data** array;
      unsigned short heap_size;
      unsigned short length;
} Heap;

typedef struct Bucket {
      void* data;
      unsigned int key;
      struct Bucket* next;
} Bucket;

typedef struct HashMap {
      Bucket** map;
      unsigned short length;
      unsigned short size;
} HashMap;

typedef struct Ingredient {
      Heap* lot;
      unsigned int tot;
} Ingredient;

typedef struct RNeededIngredient {
      unsigned int key;
      unsigned int qty;
} RNeededIngredient;

typedef struct Recipe {
      unsigned long* compressed;
      char length;
      unsigned short waiting_orders;
      unsigned int weight;
      RNeededIngredient** ingredients;
      unsigned short n_ing;
} Recipe;

typedef struct RBucket {
      Recipe* recipe;
      struct RBucket* next;
} RBucket;

typedef struct RHash {
      RBucket** map;
      unsigned short length;
      unsigned short size;
} RHash;

typedef struct OrderedRecipe {
      unsigned int t;
      Recipe* recipe;
      unsigned int qty;
} OrderedRecipe;

typedef struct OrderHeap {
      unsigned short heap_size;
      unsigned short length;
      OrderedRecipe** array;
} OrderHeap;


#define GET_BUCKET( hash, key ) hash->map[(key)%hash->length]



HashMap* new_hash( unsigned int size ){
      HashMap* hash = (HashMap*)malloc(sizeof(HashMap));

      hash->length = size;
      hash->size = 0;
      hash->map = (Bucket**)calloc(size, sizeof(Bucket*));

      return hash;
}


void* hash_get( HashMap* hash, unsigned int key ){

      Bucket* bucket = GET_BUCKET( hash, key );


      while( bucket ){
            if( bucket->key == key ){
                  return bucket->data;
            }
            bucket = bucket->next;
      }
      return NULL;
}
void resize_hash( HashMap* hash ){

      Bucket** new_array = (Bucket**)calloc( hash->length* 2, sizeof(Bucket*) );

      for( unsigned int i = 0; i < hash->length; i++ ){
            Bucket* bucket = hash->map[i];

            while( bucket ){

                  Bucket* tmp = bucket;

                  bucket = bucket->next;
                  tmp->next = new_array[tmp->key%(hash->length*2)];
                  new_array[tmp->key%(hash->length*2)] = tmp;
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
      new_bucket_node->key = key;
      new_bucket_node->data = data;
      new_bucket_node->next = bucket;

      hash->map[key%hash->length] = new_bucket_node;
}

void* hash_delete( HashMap* hash, unsigned int key ){
      Bucket* bucket = GET_BUCKET( hash, key );
      Bucket* prev = bucket;

      while( bucket ){
            if( bucket->key == key ){

                  void* data = bucket->data;
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

void queue_push( Queue* queue, void* data ){
      List* node = (List*)malloc( sizeof( List ) );

      node->data = data;
      node->next = NULL;
      
      if( !queue->head ){
            queue->head = node;
            queue->tail = node;
      }else{
            queue->tail->next = node;
            queue->tail = node;
      }
}

#define LEFT(x) (2 * (x) + 1)
#define RIGHT(x) (2 * (x + 1))
#define PARENT(x) ((x - 1) / 2)
#define heap_top( heap ) (heap->array[0]) 

Heap* new_heap( unsigned int height ){
      Heap* h = (Heap*)malloc( sizeof(Heap) );

      h->length = 1 << height;
      h->heap_size = 0;
      h->array = (Data**)malloc( sizeof(Data*)*h->length );

      return h;
}

void min_heapify( Heap* heap, unsigned int n ){

      unsigned int l = LEFT(n);
      unsigned int r = RIGHT(n);

      while( 1 ){
            unsigned int posmax = n;
            if( l < heap->heap_size && heap->array[l]->key < heap->array[n]->key )
                  posmax = l;
            if( r < heap->heap_size && heap->array[r]->key < heap->array[posmax]->key )
                  posmax = r;
            
            if( posmax != n ){
                  Data* tmp = heap->array[posmax];

                  heap->array[posmax] = heap->array[n];
                  heap->array[n] = tmp;

                  n = posmax;
                  l = LEFT(n);
                  r = RIGHT(n);
            }else{
                  return;
            }
      }
}



Data* heap_pop_min( Heap* heap ){
      if( heap->heap_size < 1 )
            return NULL;
      Data* min = heap->array[0];

      heap->array[0] = heap->array[heap->heap_size - 1];
      heap->heap_size--;
      min_heapify( heap, 0 );
      return min;
}

#define expand_heap( heap ) { heap->length *= 2; heap->array = (Data**)realloc(heap->array, sizeof(Data*) * heap->length); }
#define expand_orders( heap ) { heap->length *= 2; heap->array = (OrderedRecipe**)realloc(heap->array, sizeof(OrderedRecipe*) * heap->length); }


void heap_push_min( Heap* heap, unsigned int key, unsigned int data ){
      heap->heap_size++;
      if( heap->heap_size > heap->length )
            expand_heap( heap );
      heap->array[ heap->heap_size - 1 ] = (Data*)malloc(sizeof(Data));
      ( heap->array[ heap->heap_size - 1 ] )->data = data;
      ( heap->array[ heap->heap_size - 1 ] )->key = key;

      unsigned int i = heap->heap_size - 1;

      while( i > 0 && heap->array[PARENT(i)]->key > heap->array[i]->key ){
            
            Data* tmp = heap->array[PARENT(i)];

            heap->array[PARENT(i)] = heap->array[i];
            heap->array[i] = tmp;
            i = PARENT(i);
      } 
}

OrderHeap* new_order_heap( unsigned int height ){
      OrderHeap* h = (OrderHeap*)malloc( sizeof(OrderHeap) );

      h->length = 1 << height;
      h->heap_size = 0;
      h->array = (OrderedRecipe**)malloc( sizeof(OrderedRecipe*)*h->length );

      return h;
}

void min_heapify_orders( OrderHeap* heap, unsigned int n ){

      unsigned int l = LEFT(n);
      unsigned int r = RIGHT(n);

      while( 1 ){
            unsigned int posmax = n;
            if( l < heap->heap_size && heap->array[l]->t < heap->array[n]->t )
                  posmax = l;
            if( r < heap->heap_size && heap->array[r]->t < heap->array[posmax]->t )
                  posmax = r;
            
            if( posmax != n ){
                  OrderedRecipe* tmp = heap->array[posmax];

                  heap->array[posmax] = heap->array[n];
                  heap->array[n] = tmp;

                  n = posmax;
                  l = LEFT(n);
                  r = RIGHT(n);
            }else{
                  return;
            }
      }
}



OrderedRecipe* heap_pop_min_orders( OrderHeap* heap ){
      if( heap->heap_size < 1 )
            return NULL;
      OrderedRecipe* min = heap->array[0];

      heap->array[0] = heap->array[heap->heap_size - 1];
      heap->heap_size--;
      min_heapify_orders( heap, 0 );

      return min;
}

void heap_push_min_orders( OrderHeap* heap, OrderedRecipe* order ){
      heap->heap_size++;
      if( heap->heap_size > heap->length )
            expand_orders( heap );
      heap->array[ heap->heap_size - 1 ] = order;

      unsigned int i = heap->heap_size - 1;

      while( i > 0 && heap->array[PARENT(i)]->t > heap->array[i]->t ){
            
            OrderedRecipe* tmp = heap->array[PARENT(i)];

            heap->array[PARENT(i)] = heap->array[i];
            heap->array[i] = tmp;
            i = PARENT(i);
      } 
}

int get_string( char* str ){
      int tmp = getchar_unlocked();
      int i = 0;

      if( tmp == EOF ){
            return END_OF_FILE;
      }
      
      while( tmp >= '0' ){
            str[i] = tmp;
            i++;
            tmp = getchar_unlocked();
      }
      str[i] = '\0';
      if( tmp == '\n' )
            return END_ROW;
      return CONTINUE;
}

int get_number( unsigned int* var ){
      int tmp = getchar_unlocked();
      *var = 0;

      if( tmp == EOF ){
            return END_OF_FILE;
      }
      
      while( tmp >= '0' && tmp <= '9' ){
            *var = (*var) * BASE + (tmp - '0');
            tmp = getchar_unlocked();
      }
      if( tmp == '\n' )
            return END_ROW;
      return CONTINUE;
}

unsigned long int* compress( char* str, char* length ){
      int len = 0;

      while( str[len] != '\0' ){
            len++;
      }

      unsigned long int* c = (unsigned long int*)malloc( sizeof(unsigned long int)*(len + ROUNDING)/MAX_STORED_DIGITS );

      for( int i = 0; i < (len + ROUNDING)/MAX_STORED_DIGITS; i++ ){
            int j = 0;
            unsigned long int mul = 1;
            c[i] = 0;

            while( j < MAX_STORED_DIGITS && str[i*MAX_STORED_DIGITS+j] != '\0' ){

                  c[i] += (long int)(((int)str[i*MAX_STORED_DIGITS+j]) - CHAR_OFFSET)*mul;
                  mul *= COMPRESS_BASE;
                  j++;
            }
      }

      *length = len;
      return c;
}

void decompress( char* str, unsigned long int* comp, char len ){
      int a_len = ((int)len + ROUNDING)/MAX_STORED_DIGITS;


      for( int i = 0; i < a_len; i++ ){
            unsigned long int tmp = comp[i];
            int j = 0;
            while( tmp > 0 ){
                  str[ i*MAX_STORED_DIGITS + j ] = (char)( tmp%COMPRESS_BASE + CHAR_OFFSET );
                  tmp /= COMPRESS_BASE;
                  j++;
            }
      }
      str[(int)len] = '\0';
      return;
}

char compare_compressed_strings( unsigned long int* a, unsigned long int* b, int len ){
      for( int i = 0; i < len; i++ ){
            if( a[ len - 1 - i ] != b[ len - 1 - i ] ){
                  return 0;
            }
      }
      return 1;
}

unsigned int hash_string( char* ingredient ){
      unsigned int key = 1;
      int len = 0;
      int n_of_chars;

      while( ingredient[len] != '\0' ){
            len++;
      }
      n_of_chars = len;
      
      if( len > MAX_UINT_ENCRYPT ){
            n_of_chars = MAX_UINT_ENCRYPT;
      }

      for ( int i = 0; i < n_of_chars; i++ ){
            key = key*COMPRESS_BASE +  ( ingredient[len - i - 1] - CHAR_LIMIT );
      }

      return key;
}
/**
 * add an ingredient to the store, if the ingredient already exists it sums the new qty with the overall qty
 * otherwise it adds the new ingredient to the store
 */
void store_add_ingredient( Heap* heap, unsigned int key, unsigned int data ){
      int lvl_size = 1;

      for( int prev_lvl = 0; prev_lvl < heap->heap_size; lvl_size <<= 1 ){
            char flag = 1;

            for( int i = 0; i < lvl_size && prev_lvl + i < heap->heap_size; i++ ){
                  if( heap->array[prev_lvl + i]->key < key ){
                        flag = 0;
                  }

                  if( heap->array[prev_lvl + i]->key == key ){
                        heap->array[prev_lvl + i]->data = heap->array[prev_lvl + i]->data + data;
                        return;
                  }
            }

            if( flag )
                  break;

            prev_lvl += lvl_size;
      }

      heap_push_min( heap, key, data );
}
Bucket* new_ingredient( unsigned int key, unsigned int qty, unsigned int expire ){
      Bucket* bucket = (Bucket*)malloc( sizeof(Bucket) );

      bucket->next = NULL;
      bucket->data = malloc( sizeof(Ingredient) );
      bucket->key = key;

      ((Ingredient*)bucket->data)->tot = qty;
      ((Ingredient*)bucket->data)->lot = new_heap( 3 );


      //push ingredient inside heap
      ((Ingredient*)bucket->data)->lot->heap_size = 1;
      ((Ingredient*)bucket->data)->lot->array[0] = (Data*)malloc( sizeof(Data) );
      ((Ingredient*)bucket->data)->lot->array[0]->data = qty;
      ((Ingredient*)bucket->data)->lot->array[0]->key = expire;

      return bucket;
}

void add_ingredient( HashMap* store, char* ingredient, unsigned int qty, unsigned int expire ){
      unsigned int key = hash_string( ingredient );


      Bucket* bucket = GET_BUCKET(store, key );

      

      if( !bucket ){ 
            store->size++;
            if( store->size/store->length >= LOAD_FACT ){
                  resize_hash( store );
            }

            GET_BUCKET( store, key ) = new_ingredient( key, qty, expire );
            return;
      }

      while( bucket ){
            if( bucket->key == key ){  
                  ((Ingredient*)bucket->data)->tot += qty;
                  store_add_ingredient( ((Ingredient*)bucket->data)->lot, expire, qty );
                  return;
            }

            if( !bucket->next ){
                  store->size++;
                  if( store->size/store->length >= LOAD_FACT ){
                        resize_hash( store );
                  }
                  Bucket* new = new_ingredient( key, qty, expire );
                  new->next = GET_BUCKET( store, key );
                  GET_BUCKET( store, key ) = new;

                  return;
            }
            bucket = bucket->next;
      }      
}

void use_ingredient( HashMap* store, unsigned int key, unsigned int qty ){
      

      Ingredient* ing = (Ingredient*)hash_get( store, key );
      ing->tot -= qty;

      if( !ing->tot ){
            hash_delete( store, key );

            for( unsigned int i = 0; i < ing->lot->heap_size; i++ ){
                  free( ing->lot->array[i] );
            }
            free( ing->lot->array );
            free( ing->lot );
            free( ing );

            return;  
      }
      
      while( qty > 0 ){
            if( (unsigned long)heap_top(ing->lot)->data <= qty ){
                  qty -= (unsigned long)heap_top(ing->lot)->data;
                  
                  free( heap_pop_min( ing->lot ) );
            }else{
                  heap_top(ing->lot)->data = heap_top(ing->lot)->data - (unsigned long)qty;
                  break;
            }
      }
}

unsigned int get_ingredient_qty( HashMap* store, unsigned int key ){
      
      
      Ingredient* data = (Ingredient*)hash_get( store, key );

      if( data ){
            return data->tot;
      }

      return 0;
}

unsigned int trash_expired_ingredients( HashMap* store, unsigned int t ){

      unsigned int exp_t = MAX_U32_SIZE; 

      unsigned int i;

      for( i = 0; i < store->length; i++ ){
            
            if( !store->map[i] ){
                  continue;
            }

            Bucket* bucket = store->map[i];
            Bucket* prev = bucket;
      
            while( bucket ){
                  
                  Heap* heap = ((Ingredient*)bucket->data)->lot;

                  if( heap_top( heap )->key <= t ){

                        ((Ingredient*)bucket->data)->tot -= heap_top( heap )->data;
                        free( heap_pop_min( heap ) );
                        
                        if( !heap->heap_size ){
                              //free( hash_delete( store, bucket->data->key ) );
                              free( heap->array );
                              free( heap );
                              store->size--;


                              if( bucket != prev ){
                                    prev->next = bucket->next;
                                    free( bucket->data );
                                    free( bucket );
                                    bucket = prev;
                              }else if( prev->next ){

                                    store->map[i] = prev->next;
                                    prev = prev->next;
                                    free( bucket->data );
                                    free( bucket );
                                    bucket = prev;
                                    continue;
                              }else{
                                    store->map[i] = NULL;
                                    free( bucket->data );
                                    free( bucket );
                                    bucket = NULL;
                                    break;
                              }
                        }else if( heap_top( heap )->key < exp_t ){
                              exp_t = heap_top( heap )->key;
                        }
                        
                  }else if( heap_top( heap )->key < exp_t ){
                        exp_t = heap_top( heap )->key;
                  }
                  prev = bucket;
                  bucket = bucket->next;
                 
            }
      }
      return exp_t;
}

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
                  free( compressed );
                  return bucket->recipe;
            }
            bucket = bucket->next;
      }
      free( compressed );
      return NULL;
}
void resize_recipes_hash( RHash* hash ){
      RBucket** new_array = (RBucket**)calloc( hash->length* 2, sizeof(RBucket*) );

      for( unsigned int i = 0; i < hash->length; i++ ){
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
                  
                  for( int i = 0; i < bucket->recipe->n_ing; i++ ){
                        free( bucket->recipe->ingredients[i] );
                  } 

                  free( compressed );
                  free( bucket->recipe->ingredients );
                  free( bucket->recipe->compressed );
                  free( bucket->recipe );
                  free( bucket );
                  return;
            }
            prev = bucket;
            bucket = bucket->next;
      }
      free( compressed );
}

unsigned short recipes_can_be_prepared( Recipe* recipe, HashMap* store, unsigned int qty ){
      
      for( int i = 0; i < recipe->n_ing; i++ ){
            unsigned int s_qty = get_ingredient_qty( store, recipe->ingredients[i]->key );

            if( s_qty < qty * recipe->ingredients[i]->qty )
                  return 0;
      }
      return 1;
}

unsigned int supply( HashMap* store, unsigned int t, unsigned int next_exp ){
      char buffer[256];
      unsigned int qty;
      unsigned int exp_time;

      while( get_string( buffer ) == CONTINUE && get_number( &qty ) == CONTINUE && get_number( &exp_time ) == CONTINUE ){
            
            if( exp_time <= t )
                  continue;

            
            add_ingredient( store, buffer, qty, exp_time );


            if( next_exp > exp_time ){
                  next_exp = exp_time;
            }

            
      }
      if( exp_time > t ){
            
            add_ingredient( store, buffer, qty, exp_time );

            if( next_exp > exp_time ){
                  next_exp = exp_time;
            }
      }
      puts( SUPPLY_SUCCESS );
      return next_exp;
}
#define INGREDIENTS 10
void add_recipe( RHash* recipes ){

      char buffer[256];
      unsigned int qty;
      int i = 0;

      get_string( buffer );

      if( recipes_get( recipes, buffer ) ){

            while( get_string( buffer ) == CONTINUE && get_number( &qty ) == CONTINUE );
            
            puts( RECIPE_ADDED_FAILED );
            return;
      }
      puts( RECIPE_ADDED_SUCCESS );
      
      Recipe* recipe = (Recipe*)malloc( sizeof( Recipe ) );

      recipe->ingredients = NULL;

      recipe->compressed = compress( buffer, &recipe->length );
      recipe->weight = 0;
      recipe->waiting_orders = 0;

      RNeededIngredient* ing[100];


      while( get_string( buffer ) == CONTINUE && get_number( &qty ) == CONTINUE ){

            RNeededIngredient* ingredient = (RNeededIngredient*)malloc( sizeof( RNeededIngredient ) );

            ingredient->key = hash_string( buffer );
            ingredient->qty = qty;
            recipe->weight += qty;
            ing[i] = ingredient;
            i++;
      }

      recipe->ingredients = (RNeededIngredient**)malloc( sizeof( RNeededIngredient*)* (i+1) );
      
      recipe->ingredients[0] = (RNeededIngredient*)malloc( sizeof( RNeededIngredient ) );

      recipe->ingredients[0]->key = hash_string( buffer );
      recipe->ingredients[0]->qty = qty;
      recipe->weight += qty;
      recipe->n_ing = i + 1;

      for( unsigned short j = 1; j < i + 1; j++ ){
            recipe->ingredients[j] = ing[j-1];
      }
      recipes_set( recipes, recipe );
}
void remove_recipe( RHash* recipes ){
      char buff[256];

      get_string( buff );

      Recipe* recipe = recipes_get( recipes, buff );

      if( !recipe ){
            puts( RECIPE_REMOVED_NOT_FOUND );
            return;
      }

      if( recipe->waiting_orders > 0 ){
            puts( RECIPE_REMOVED_WAITING_ORDERS );
            return;
      }
      puts( RECIPE_REMOVED_SUCCESS );
      recipes_delete( recipes, buff );
}
void order( Queue* waiting, OrderHeap* ready, RHash* recipes, HashMap* store, unsigned int t, unsigned int camion_max_w ){
      unsigned int qty;
      char buff[256];

      get_string( buff );
      get_number( &qty );

      Recipe* recipe = recipes_get( recipes, buff );

      if( !recipe ){
            puts( ORDER_FAILED );
            return;
      }

      OrderedRecipe* order = (OrderedRecipe*)malloc( sizeof( OrderedRecipe ) );

      order->recipe = recipe;
      order->t = t;
      order->qty = qty;

      recipe->waiting_orders++;
      puts( ORDER_SUCCESS );

      if( !recipes_can_be_prepared( recipe, store, qty ) ){
            queue_push( waiting, order );
      }else{

            for( int i = 0; i < recipe->n_ing; i++ ){
                  use_ingredient( store, recipe->ingredients[i]->key, recipe->ingredients[i]->qty * qty );
            }
            
            heap_push_min_orders( ready, order );
      }
}

int _compare_order_( const void* a, const void* b ){
      OrderedRecipe* A = *(OrderedRecipe**)a;
      OrderedRecipe* B = *(OrderedRecipe**)b;

      unsigned int a_weight = A->qty * A->recipe->weight;
      unsigned int b_weight = B->qty * B->recipe->weight;

      if( a_weight > b_weight ){
            return -1;
      }else if( a_weight < b_weight ){
            return 1;
      }else if( A->t > B->t ){
            return 1;
      }

      return -1;
}
void fill_camion_v2( OrderHeap* ready, unsigned int camion_max_w ){
      if( !ready->heap_size || ( heap_top( ready )->qty * heap_top( ready )->recipe->weight > camion_max_w ) ){
            puts( CAMION_EMPTY );
            return;
      }
      //unsigned int len = camion_max_w/16;
      OrderedRecipe* output[3000]; //= (OrderedRecipe**)malloc( sizeof(OrderedRecipe*) * len );
      unsigned int i = 0;
      
      while( camion_max_w && ready->heap_size ){
            if( camion_max_w >= heap_top( ready )->qty * heap_top( ready )->recipe->weight ){
                  camion_max_w -= heap_top( ready )->qty * heap_top( ready )->recipe->weight;
                  /*if( i + 1 == len ){
                        len *= 2;
                        output = realloc( output, len * sizeof( OrderedRecipe* ) );
                  }*/
                  output[i] = heap_pop_min_orders( ready );
                  i++;
            }else{
                  break;
            }
      }

      qsort( output, i, sizeof( OrderedRecipe* ), _compare_order_ );

      for( unsigned int j = 0; j < i; j++ ){
            char name[ output[j]->recipe->length + 1 ];
            decompress( name, output[j]->recipe->compressed, output[j]->recipe->length );

            printf("%u %s %u\n", output[j]->t, name, output[j]->qty );

            
            output[j]->recipe->waiting_orders--;
            free( output[j] );
      }

      //free( output );
}

void check_waiting( Queue* waiting, OrderHeap* ready, RHash* recipes, HashMap* store, unsigned int camion_max_w, unsigned int t ){
      List* node = waiting->head;
      List* prev = node;

      while( node ){
            
            if( recipes_can_be_prepared( ((OrderedRecipe*)node->data)->recipe, store, ((OrderedRecipe*)node->data)->qty ) ){
                  RNeededIngredient** ingredients = ((OrderedRecipe*)node->data)->recipe->ingredients;
                  unsigned int n_ing = ((OrderedRecipe*)node->data)->recipe->n_ing;
                  unsigned int qty = ((OrderedRecipe*)node->data)->qty;            

                  for( unsigned int i = 0; i < n_ing; i++ ){
                        use_ingredient( store, ingredients[i]->key, ingredients[i]->qty * qty );
                  }
                  
                  heap_push_min_orders( ready, node->data );

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
            prev = node;     
            node = node->next;
      }
}

void tear_down( HashMap* store, RHash* recipes, OrderHeap* ready, Queue* waiting ){
      for( int i = 0; i < ready->heap_size; i++ ){
            free( ready->array[i] );
      }
      free( ready->array );
      free( ready );

      List* node = waiting->head;

      while( node ){
            List* tmp = node;
            node = node->next;

            free( tmp->data );
            free( tmp );
      }

      free( waiting );

      unsigned short size = 0;

      for( int i = 0; i < store->length; i++ ){
            while( store->map[i] ){
                  Bucket* tmp = store->map[i];

                  store->map[i] = store->map[i]->next;
                  Ingredient* ing = (Ingredient*)tmp->data;

                  for( int i = 0; i < ing->lot->heap_size; i++ ){
                        free( ing->lot->array[i] );
                  }
                  free( ing->lot->array );
                  free( ing->lot );
                  free( ing );
                  free( tmp );
                  size++;
            }
            if( size == store->size )
                  break;
      }
      free( store->map );
      free( store );

      size = 0;
      for( int i = 0; i < recipes->length; i++ ){
            while( recipes->map[i] ){ 
                  free( recipes->map[i]->recipe->compressed );
                  for( int j = 0; j < recipes->map[i]->recipe->n_ing; j++ ){
                        free( recipes->map[i]->recipe->ingredients[j] );
                  }
                  free( recipes->map[i]->recipe->ingredients );
                  free( recipes->map[i]->recipe );
                  RBucket* tmp = recipes->map[i];
                  recipes->map[i] = recipes->map[i]->next;
                  free( tmp );
                  size++;
            }
            if( size == recipes->size )
                  break;
      }
      free( recipes->map );
      free( recipes );
}
int main( int argc, char **argv ){
      HashMap* store = new_hash( 10 );
      RHash* recipes = new_recipes_store( 10 );

      OrderHeap* ready = new_order_heap( 4 );
      Queue* waiting = (Queue*)calloc( 1, sizeof(Queue) );
      
      char buff[17];
      unsigned int camion_t;
      unsigned int camion_max_w;
      unsigned int t = 0;
      unsigned int next_exp = MAX_U32_SIZE;

      setvbuf(stdout, NULL, _IOFBF, 4096);

      get_number( &camion_t );
      get_number( &camion_max_w );

      

      while( get_string( buff ) != END_OF_FILE ){
            
            
            if( t == next_exp ){
                  next_exp = trash_expired_ingredients( store, t );
            }
            if( !(t%camion_t) && t > 0 ){
                  fill_camion_v2( ready, camion_max_w );
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
                        next_exp = supply( store, t, next_exp );
                        check_waiting( waiting, ready, recipes, store, camion_max_w, t ); 
                  }break;
                  default: {
                        if( !(t%camion_t) && t > 0 ){
                              fill_camion_v2( ready, camion_max_w );
                        }
                        tear_down( store, recipes, ready, waiting );
                        return 0;
                  }
                  
            }
            t++;
      }
      if( !(t%camion_t) && t > 0 ){
            fill_camion_v2( ready, camion_max_w );
      }
      tear_down( store, recipes, ready, waiting );
      return 0;
}