#pragma once 

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
      unsigned short heap_size;
      unsigned short length;
} Heap;

typedef struct Bucket {
      Data* data;
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

typedef struct Recipe {
      unsigned long* compressed;
      char length;
      unsigned short waiting_orders;
      unsigned int weight;
      List* ingredients;
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

typedef struct RNeededIngredient {
      unsigned int key;
      unsigned int qty;
} RNeededIngredient;

typedef struct OrderedRecipe {
      unsigned int t;
      Recipe* recipe;
      unsigned int qty;
} OrderedRecipe;
