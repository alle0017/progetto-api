#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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

#define SUPPLY_SUCCESS "rifornito\n"

#define RECIPE_ADDED_SUCCESS "aggiunta\n"
#define RECIPE_ADDED_FAILED "ignorato\n"

#define RECIPE_REMOVED_SUCCESS "rimossa\n"
#define RECIPE_REMOVED_NOT_FOUND "non presente\n"
#define RECIPE_REMOVED_WAITING_ORDERS "ordini in sospeso\n"

#define ORDER_SUCCESS "accettato\n"
#define ORDER_FAILED "rifiutato\n"

#define CAMION_EMPTY "camioncino vuoto\n"

#define TRACK_ING 0
#define TRACKED 1900736913
#define DEBUG_SUPPLY 0
#define DEBUG_PROCESSING_ORDERS 0
#define DEBUG_MISS 0
#define DEBUG_SUPPLY_NAME 0
#define DEBUG 0
#define DEBUG_USE_ING 0
#define DEBUG_ING_QTY 0
#define DEBUG_TRASH 0
#define DEBUG_EXPIRE_TIME 0
#define DEBUG_READY_RECIPE 0
#define DEBUG_WEIGHT 0
#define DEBUG_HEAP_SIZE_PREPARE_RECIPE 0
#define DEBUG_RECIPE 0
#define DEBUG_ING_OCC 0

#define DEPLOY 1
#define GAB 0


typedef struct List {
      void* data;
      struct List* next;
} List;

typedef struct Queue {
      List* head;
      List* tail;
} Queue;

typedef struct Data {
      void* data; 
      unsigned int key;
} Data;

typedef struct Heap {
      Data** array;
      unsigned int heap_size;
      unsigned int length;
} Heap;

typedef struct Bucket {
      Data* data;
      struct Bucket* next;
} Bucket;

typedef struct HashMap {
      Bucket** map;
      unsigned int length;
      unsigned int size;
} HashMap;

typedef struct Ingredient {
      Heap* lot;
      unsigned int tot;
} Ingredient;

typedef struct Recipe {
      unsigned long* compressed;
      char length;
      unsigned int waiting_orders;
      unsigned int weight;
      List* ingredients;
} Recipe;

typedef struct RBucket {
      Recipe* recipe;
      struct RBucket* next;
} RBucket;

typedef struct RHash {
      RBucket** map;
      unsigned int length;
      unsigned int size;
} RHash;

typedef struct RNeededIngredient {
      unsigned int key;
      unsigned int qty;
} RNeededIngredient;

typedef struct OrderedRecipe {
      unsigned int t;
      Recipe* recipe;
      unsigned int qty;
} OrderedRecipe;


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
//TODO realloc
void resize_hash( HashMap* hash ){

      Bucket** new_array = (Bucket**)calloc( hash->length* 2, sizeof(Bucket*) );

      for( unsigned int i = 0; i < hash->length; i++ ){
            new_array[i] = NULL;
            new_array[i + hash->length] = NULL;
      }
      for( unsigned int i = 0; i < hash->length; i++ ){
            Bucket* bucket = hash->map[i];

            while( bucket ){

                  Bucket* tmp = bucket;

                  bucket = bucket->next;
                  tmp->next = new_array[tmp->data->key%(hash->length*2)];
                  new_array[tmp->data->key%(hash->length*2)] = tmp;
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


void max_heapify( Heap* heap, unsigned int n ){

      unsigned int l = LEFT(n);
      unsigned int r = RIGHT(n);

      while( 1 ){
            unsigned int posmax = n;
            if( l < heap->heap_size && heap->array[l]->key > heap->array[n]->key )
                  posmax = l;
            if( r < heap->heap_size && heap->array[r]->key > heap->array[posmax]->key )
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

void max_heapify_ordered( Heap* heap, unsigned int n ){

      unsigned int l = LEFT(n);
      unsigned int r = RIGHT(n);

      while( 1 ){
            unsigned int posmax = n;
            if( l < heap->heap_size && (heap->array[l]->key > heap->array[n]->key || ( heap->array[l]->key == heap->array[n]->key && ((OrderedRecipe*)heap->array[l]->data)->t < ((OrderedRecipe*)heap->array[n]->data)->t )) )
                  posmax = l;
            if( r < heap->heap_size && (heap->array[r]->key > heap->array[posmax]->key || ( heap->array[r]->key == heap->array[posmax]->key && ((OrderedRecipe*)heap->array[r]->data)->t < ((OrderedRecipe*)heap->array[posmax]->data)->t ))  )
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

Data* heap_pop_max_ordered( Heap* heap ){
      if( heap->heap_size < 1 )
            return NULL;
      Data* max = heap->array[0];

      heap->array[0] = heap->array[heap->heap_size - 1];
      heap->heap_size--;
      max_heapify_ordered( heap, 0 );
      return max;
}


Data* heap_pop_max( Heap* heap ){
      if( heap->heap_size < 1 )
            return NULL;
      Data* max = heap->array[0];

      heap->array[0] = heap->array[heap->heap_size - 1];
      heap->heap_size--;
      max_heapify( heap, 0 );
      return max;
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

void expand_heap( Heap* heap ){
      heap->length *= 2;
      heap->array = (Data**)realloc(heap->array, sizeof(Data*) * heap->length);
}

void heap_push_max( Heap* heap, unsigned int key, void* data ){
      heap->heap_size++;
      if( heap->heap_size > heap->length )
            expand_heap( heap );
      heap->array[ heap->heap_size - 1 ] = (Data*)malloc(sizeof(Data));
      ( heap->array[ heap->heap_size - 1 ] )->data = data;
      ( heap->array[ heap->heap_size - 1 ] )->key = key;

      unsigned int i = heap->heap_size - 1;

      while( i > 0 && heap->array[PARENT(i)]->key < heap->array[i]->key ){

            Data* tmp = heap->array[PARENT(i)];

            heap->array[PARENT(i)] = heap->array[i];
            heap->array[i] = tmp;
            i = PARENT(i);
      } 
}
void heap_push_min( Heap* heap, unsigned int key, void* data ){
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

void order_max_heap( Heap* heap ){
      for( unsigned int i = (unsigned int)heap->heap_size/2; i > 0; i-- ){
            max_heapify( heap, i );
      }
}

void order_min_heap( Heap* heap ){
      for( unsigned int i = (unsigned int)heap->heap_size/2; i > 0; i-- ){
            min_heapify( heap, i );
      }
}

void max_heapify_array( Data* array[], unsigned int n, unsigned int size ){

      unsigned int l = LEFT(n);
      unsigned int r = RIGHT(n);

      while( l < size ){
            if( array[l]->key > array[n]->key && ( r >= size || array[l]->key >= array[r]->key ) ){
                  Data* tmp = array[l];

                  array[l] = array[n];
                  array[n] = tmp;

                  n = l;
                  l = LEFT(n);
                  r = RIGHT(n);

                  continue;
            }else if( r < size && array[r]->key >= array[n]->key ){
                  Data* tmp = array[r];

                  array[r] = array[n];
                  array[n] = tmp;

                  n = r;
                  l = LEFT(n);
                  r = RIGHT(n);

                  continue;
            }
            return;
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

char* decompress( unsigned long int* comp, char len ){
      char* str = (char*)malloc((int)len + 1);
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
      return str;
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
void store_add_ingredient( Heap* heap, unsigned int key, unsigned long int data ){
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

      heap_push_min( heap, key, (void*)data );
}
Bucket* new_ingredient( unsigned int key, unsigned long qty, unsigned int expire ){
      Bucket* bucket = (Bucket*)malloc( sizeof(Bucket) );

      bucket->next = NULL;
      bucket->data = (Data*)malloc( sizeof(Data) );
      bucket->data->data = (Ingredient*)malloc( sizeof(Ingredient) );
      bucket->data->key = key;

      ((Ingredient*)bucket->data->data)->tot = qty;
      ((Ingredient*)bucket->data->data)->lot = new_heap( 3 );


      //push ingredient inside heap
      ((Ingredient*)bucket->data->data)->lot->heap_size = 1;
      ((Ingredient*)bucket->data->data)->lot->array[0] = (Data*)malloc( sizeof(Data) );
      ((Ingredient*)bucket->data->data)->lot->array[0]->data = (void*)qty;
      ((Ingredient*)bucket->data->data)->lot->array[0]->key = expire;

      return bucket;
}
/**
 * store an amount of ingredients inside the store
 */
#if GAB

void add_ingredient( HashMap* store, char* ingredient, unsigned long qty, unsigned int expire, HashMap* domain ){

#else
void add_ingredient( HashMap* store, char* ingredient, unsigned long qty, unsigned int expire ){
#endif
      unsigned int key = hash_string( ingredient );

      #if GAB
            if( !hash_get( domain, key ) ){
                  char* buff = (char*)malloc( sizeof(char)*256 );
                  int i;

                  for( i = 0; ingredient[i] != '\0'; i++ ){
                        buff[i] = ingredient[i];
                  }
                  buff[i] = '\0';
                  hash_set( domain, key, buff );
            }
            
      #endif 


      Bucket* bucket = GET_BUCKET(store, key );

      

      if( !bucket ){
            #if DEBUG_SUPPLY
                  printf("bucket doesn't exists\n");
            #endif    
            store->size++;
            if( store->size/store->length >= LOAD_FACT ){
                  resize_hash( store );
            }

            GET_BUCKET( store, key ) = new_ingredient( key, qty, expire );

            #if TRACK_ING
                  if( key == TRACKED ){
                        printf("added %u\n", ((Ingredient*)GET_BUCKET( store, key )->data)->tot);
                  }
            #endif
            return;
      }

      while( bucket ){
            if( bucket->data->key == key ){
                  #if DEBUG_SUPPLY
                        printf("ingredient found\n");
                  #endif      
                  ((Ingredient*)bucket->data->data)->tot += qty;
                  #if TRACK_ING
                        if( key == TRACKED ){
                              printf("added %u\n", ((Ingredient*)bucket->data->data)->tot);
                        }
                  #endif
                  store_add_ingredient( ((Ingredient*)bucket->data->data)->lot, expire, qty );
                  return;
            }

            if( !bucket->next ){
                  #if DEBUG_SUPPLY
                        printf("last of bucket\n");
                  #endif    
                  store->size++;
                  if( store->size/store->length >= LOAD_FACT ){
                        //printf("%u\n", store->length);
                        resize_hash( store );
                  }
                  Bucket* new = new_ingredient( key, qty, expire );
                  #if TRACK_ING
                        if( key == TRACKED ){
                              printf("added %u\n", ((Ingredient*)new->data->data)->tot);
                        }
                  #endif
                  new->next = GET_BUCKET( store, key );
                  GET_BUCKET( store, key ) = new;

                  return;
            }
            bucket = bucket->next;
      }      
}

#if GAB && DEBUG_USE_ING
void use_ingredient( HashMap* store, unsigned int key, unsigned long qty, HashMap* domain ){
#else 
void use_ingredient( HashMap* store, unsigned int key, unsigned long qty ){
#endif

      

      Ingredient* ing = (Ingredient*)(hash_get( store, key )->data);
      ing->tot -= qty;
      #if TRACK_ING
            if( key == TRACKED ){
                  printf("removed %u\n", ing->tot);
            }
      #endif

      #if DEBUG_USE_ING && GAB
            for( int i = 0; i < ing->lot->heap_size; i++ )
                  printf("%s expire at %u,\n",hash_get( domain, key )->data, ing->lot->array[i]->key );
      #elif DEBUG_USE_ING
            for( int i = 0; i < ing->lot->heap_size; i++ )
                  printf("expire at %u,\n", ing->lot->array[i]->key);
      #endif
      if( !ing->tot ){
            free( ing->lot->array );
            free( ing->lot );
            free( ing );
            free( hash_delete( store, key ) );
            
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
      
      
      Data* data = hash_get( store, key );

      if( data ){
            return ((Ingredient*)(data->data))->tot;
      }

      return 0;
}

/**
 * delete all the elements that are expired and return the next expire time
 */
unsigned int trash_expired_ingredients( HashMap* store, unsigned int t ){

      #if DEBUG_TRASH
            puts("[trash]");
      #endif
      //unsigned int size = 0;
      unsigned int exp_t = MAX_U32_SIZE; 

      unsigned int i;

      for( i = 0; i < store->length; i++ ){
            
            if( !store->map[i] ){
                  continue;
            }
            //1925
            Bucket* bucket = store->map[i];
            Bucket* prev = bucket;
      
            while( bucket ){
                  
                  Heap* heap = ((Ingredient*)bucket->data->data)->lot;

                  if( heap_top( heap )->key <= t ){

                        ((Ingredient*)bucket->data->data)->tot -= (unsigned long int)heap_top( heap )->data;
                        #if DEBUG_EXPIRE_TIME
                              printf("%u %d %d [trashed]\n", bucket->data->key%20, heap_top( heap )->key, (unsigned long int)heap_top( heap )->data );
                        #endif
                        free( heap_pop_min( heap ) );

                        if( !heap->heap_size ){
                              //free( hash_delete( store, bucket->data->key ) );
                              free( heap->array );
                              free( heap );
                              store->size--;


                              if( bucket != prev ){
                                    prev->next = bucket->next;
                                    free( bucket->data->data );
                                    free( bucket->data );
                                    free( bucket );
                                    bucket = prev;
                              }else if( prev->next ){

                                    store->map[i] = prev->next;
                                    prev = prev->next;
                                    free( bucket->data->data );
                                    free( bucket->data );
                                    free( bucket );
                                    bucket = prev;
                                    continue;
                              }else{
                                    store->map[i] = NULL;
                                    free( bucket->data->data );
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
                        #if DEBUG_EXPIRE_TIME
                              printf("%u %d [non trashed], ", bucket->data->key%20, exp_t);
                        #endif
                  }else{
                        #if DEBUG_EXPIRE_TIME
                              printf("%u %d [non trashed], ", bucket->data->key%20, exp_t);
                        #endif
                  }
                  prev = bucket;
                  bucket = bucket->next;
                 
            }
            /*size++;
            if( size == store->size ){
                  printf("end");
                  return exp_t;
            }*/
      }
      return exp_t;
      //order_min_heap();
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
                  return bucket->recipe;
            }
            bucket = bucket->next;
      }
      return NULL;
}
void resize_recipes_hash( RHash* hash ){
      RBucket** new_array = (RBucket**)calloc( hash->length* 2, sizeof(RBucket*) );

      for( unsigned int i = 0; i < hash->length; i++ ){
            new_array[i] = NULL;
            new_array[i + hash->length] = NULL;
      }

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
                  
                  List* node = bucket->recipe->ingredients;

                  while( node ){

                        List* tmp = node;

                        node = node->next;

                        free( tmp->data );
                        free( tmp );
                  }

                  free( bucket->recipe->compressed );
                  free( bucket->recipe );
                  free( bucket );
                  return;
            }
            prev = bucket;
            bucket = bucket->next;
      }
}

unsigned short recipes_can_be_prepared( Recipe* recipe, HashMap* store, unsigned int qty ){
      
      List* node = recipe->ingredients;

      #if DEBUG_MISS
            printf(" recipe qty %d [", qty);
      #endif

      while( node ){
            unsigned int tmp = get_ingredient_qty( store, ((RNeededIngredient*)(node->data))->key );
            
            #if DEBUG_MISS
                  printf("( key = %u qty = %u, stored = %u)", ((RNeededIngredient*)(node->data))->key, ((RNeededIngredient*)(node->data))->qty,tmp );
            #endif

            
            if( tmp < ((RNeededIngredient*)(node->data))->qty * qty ){
                  #if DEBUG_ING_QTY
                        printf("key = %u qty = %d needed = %d\n", ((RNeededIngredient*)(node->data))->key, tmp, ((RNeededIngredient*)(node->data))->qty * qty );
                  #endif
                  #if DEBUG_MISS
                        printf("]");
                  #endif
                  return 0;
            }
            node = node->next;
      }
      #if DEBUG_MISS
            printf("]");
      #endif
      return 1;
}
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
                        exit(1);
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
      #if DEBUG_ING_OCC
            if( hash_get( map, ingredient->key ) ){
                  exit(1);
            }
            hash_set( map, ingredient->key, NULL );
      #endif
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
#if GAB && DEBUG_USE_ING
void order( Queue* waiting, Heap* ready, RHash* recipes, HashMap* store, unsigned int t, unsigned int camion_max_w, HashMap* domain ){
#else
void order( Queue* waiting, Heap* ready, RHash* recipes, HashMap* store, unsigned int t, unsigned int camion_max_w ){
#endif
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
                  #if GAB && DEBUG_USE_ING
                        use_ingredient( store, ((RNeededIngredient*)ingredients->data)->key, ((RNeededIngredient*)ingredients->data)->qty * qty, domain );
                  #else
                        use_ingredient( store, ((RNeededIngredient*)ingredients->data)->key, ((RNeededIngredient*)ingredients->data)->qty * qty );
                  #endif
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
#if GAB && DEBUG_USE_ING
void check_waiting( Queue* waiting, Heap* ready, RHash* recipes, HashMap* store, unsigned int camion_max_w, unsigned int t, HashMap* domain ){
#else
void check_waiting( Queue* waiting, Heap* ready, RHash* recipes, HashMap* store, unsigned int camion_max_w, unsigned int t ){
#endif
      List* node = waiting->head;
      List* prev = node;

      while( node ){
            
            if( recipes_can_be_prepared( ((OrderedRecipe*)node->data)->recipe, store, ((OrderedRecipe*)node->data)->qty ) ){
                  List* ingredients = ((OrderedRecipe*)node->data)->recipe->ingredients;

                  #if GAB 
                        printf("Cooking (time %d): %d of %s ordered at %d\n", t, ((OrderedRecipe*)node->data)->qty, decompress( ((OrderedRecipe*)node->data)->recipe->compressed, ((OrderedRecipe*)node->data)->recipe->length ), ((OrderedRecipe*)node->data)->t );
                  #endif                

                  while( ingredients ){
                        #if GAB && DEBUG_USE_ING
                              use_ingredient( store, ((RNeededIngredient*)ingredients->data)->key, ((RNeededIngredient*)ingredients->data)->qty * ((OrderedRecipe*)node->data)->qty, domain );
                        #else
                              use_ingredient( store, ((RNeededIngredient*)ingredients->data)->key, ((RNeededIngredient*)ingredients->data)->qty * ((OrderedRecipe*)node->data)->qty );
                        #endif
                        ingredients = ingredients->next;
                  }  
                  
                  
                  if( camion_max_w >= ((OrderedRecipe*)node->data)->qty * ((OrderedRecipe*)node->data)->recipe->weight ){
                        heap_push_min( ready, ((OrderedRecipe*)node->data)->t, node->data );
                  }else{
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

      //printf("%u\n", hash_string( "KeRztwminGFlrmS" )%20 );
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
                        #if GAB && DEBUG_USE_ING
                              order( waiting, ready, recipes, store, t, camion_max_w, domain );
                        #else
                              order( waiting, ready, recipes, store, t, camion_max_w );
                        #endif
                       
                  }break;
                  case SUPPLY: {
                        #if GAB && DEBUG_USE_ING
                              next_exp = supply( store, t, next_exp, domain );
                              check_waiting( waiting, ready, recipes, store, camion_max_w, t, domain );
                        #elif GAB
                              next_exp = supply( store, t, next_exp, domain );
                              check_waiting( waiting, ready, recipes, store, camion_max_w, t );
                        #else
                              next_exp = supply( store, t, next_exp );
                              check_waiting( waiting, ready, recipes, store, camion_max_w, t ); 
                        #endif
                        #if DEBUG_EXPIRE_TIME
                              printf("next exp %d [supply]\n", next_exp);
                        #endif
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