#pragma once
#include <stdio.h>
#include <stdlib.h>

#define MAX 1
#define MIN 2

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
            if( ( min_max == MAX && curr->data->key <= key ) || ( min_max == MIN && curr->data->key >= key ) ){
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

      while( curr && curr->left ){
            if( curr->right && ( curr->data->key < curr->right->data->key || curr->data->key < curr->left->data->key ) ){
                  if( curr->left->data->key > curr->right->data->key ){
                        tmp = curr->data;
                        curr->data = curr->left->data;
                        curr->left->data = tmp;
                        curr = curr->left;
                  }else{
                        tmp = curr->data;
                        curr->data = curr->right->data;
                        curr->right->data = tmp;
                        curr = curr->right;
                  }
            }else if( curr->data->key < curr->left->data->key ){
                  tmp = curr->data;
                  curr->data = curr->left->data;
                  curr->left->data = tmp;
                  curr = curr->left;
            }else{
                  break;
            }
      }
}
static void min_heap_fix( HNode* root ){
      HNode* curr = root;
      Data* tmp;

      while( curr && curr->right && curr->left && ( curr->data->key > curr->left->data->key || curr->data->key > curr->right->data->key ) ){
            if( curr->right->data->key < curr->left->data->key ){
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
Data* heap_get_top( Heap* heap ){
      return heap->root? heap->root->data: NULL;
}