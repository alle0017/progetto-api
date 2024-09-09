#include <stdio.h>
#include <stdlib.h>

#define FIX_HEIGHT 2
#define NO_ACTION 1
#define NO_NODE_FOUND 0
#define MAX 0
#define MIN 1

typedef struct Data {
      int key;
      void* data;
} Data;
typedef struct HeapNode {
      struct HeapNode* left;
      struct HeapNode* right;
      Data* data;
} HeapNode;

typedef struct Heap {
      HeapNode* root;
      //ultimo livello completo
      int height;
} Heap;

Heap* new_heap();
void min_heap_insert( Heap* heap, int key, void* data );
Data* heap_get_top( Heap* heap );
Data* min_heap_pop_min( Heap* heap );

int static inline check_conflict( HeapNode* root ){
      if( !root->left || !root->right )
            return 1;
      return (root->left->data->key <= root->data->key && root->right->data->key >= root->data->key) || (root->left->data->key >= root->data->key && root->right->data->key <= root->data->key);
}

static int fix_descendant(HeapNode* root, int min_max) {
      Data* tmp = root->data;

      if (min_max == MAX) {
            if (root->right && root->right->data->key > root->data->key) {
                  root->data = root->right->data;
                  root->right->data = tmp;
                  return 1;
            } else if (root->left && root->left->data->key > root->data->key) {
                  root->data = root->left->data;
                  root->left->data = tmp;
                  return 1;
            }
            return 0;
      } else {
            if (root->right && root->right->data->key < root->data->key) {
                  root->data = root->right->data;
                  root->right->data = tmp;
                  return 1;
            } else if (root->left && root->left->data->key < root->data->key) {
                  root->data = root->left->data;
                  root->left->data = tmp;
                  return 1;
            }
            return 0;
      }
}
static HeapNode* new_node( int key, void* data ) {
      HeapNode* node = (HeapNode*)malloc( sizeof( HeapNode ) );
      
      if( !node ){
            printf("Heap node allocation failed");
            exit( 1 );
      }
      node->left = NULL;
      node->right = NULL;
      node->data = (Data*)malloc(sizeof(Data));
      node->data->key = key;
      node->data->data = data;
      return node;
}
Heap* new_heap(){
      Heap* heap = (Heap*)malloc( sizeof( Heap ) );
      
      if( !heap ){
            printf("Heap allocation failed");
            exit( 1 );
      }

      heap->root = NULL;
      heap->height = 0;
      return heap;
}
static int insert( HeapNode* root, int key, void* data, int height, int all_to_right, int min_max ){
      int flag;
      if ( height <= 0 ){
            if( !root )
                  return NO_NODE_FOUND;
            if( root->left && root->right )
                  return NO_NODE_FOUND;

            flag = root->left && all_to_right? FIX_HEIGHT : NO_ACTION;
            if( !root->left ){
                  root->left = new_node( key, data );
            }else{
                  root->right = new_node( key, data );
            }
            if( key > root->data->key && min_max == MAX ){
                  fix_descendant( root, MAX );
            }else if( key < root->data->key && min_max == MIN ){
                  fix_descendant( root, MIN );
            }
            return flag;
      }

      flag = insert( root->left, key, data, height - 1, 0, min_max );
      if( flag != NO_NODE_FOUND ){
            if( check_conflict( root ) ){
                  fix_descendant( root, min_max );
            }
            return flag;
      }

      flag = insert( root->right, key, data, height - 1, all_to_right, min_max );

      if( flag != NO_NODE_FOUND && check_conflict( root ) ){
            fix_descendant( root, min_max );
      }

      return flag;

}
void max_heap_insert( Heap* heap, int key, void* data ){

      if( heap->root ){
            if( insert( heap->root, key, data, heap->height, 1, MAX ) == FIX_HEIGHT ){
                  heap->height++;
            }
            return;
      }

      heap->root = new_node( key, data );
}
void min_heap_insert( Heap* heap, int key, void* data ){

      if( heap->root ){
            if( insert( heap->root, key, data, heap->height, 1, MIN ) == FIX_HEIGHT ){
                  heap->height++;
            }
            return;
      }

      heap->root = new_node( key, data );
}


Data* heap_get_top( Heap* heap ){
      if( !heap->root )
            return 0;
      return heap->root->data;
}

void min_heap_fix( HeapNode* root ){
      HeapNode* curr = root;

      while( curr && curr->right && curr->left && (curr->data->key > curr->left->data->key || curr->data->key > curr->right->data->key) ){
            if( curr->right->data->key < curr->left->data->key ){
                  Data* tmp = curr->right->data;
                  curr->right->data = curr->data;
                  curr->data = tmp;
                  curr = curr->right;
            }else{
                  Data* tmp = curr->left->data;
                  curr->left->data = curr->data;
                  curr->data = tmp;
                  curr = curr->left;
            }
      }
}
void max_heap_fix( HeapNode* root ){
      HeapNode* curr = root;
      Data* tmp;

      while( curr && curr->right && curr->left && ( curr->data->key < curr->left->data->key || curr->data->key < curr->right->data->key ) ){
            if( curr->right->data->key > curr->left->data->key ){
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
      if( curr->left && !curr->right && curr->data->key < curr->left->data->key ){
            tmp = curr->left->data;
            curr->left->data = curr->data;
            curr->data = tmp;
            curr = curr->left;
      }
}

int static switch_top_last( HeapNode* node, HeapNode* root, int height, int all_to_right  ){
      int signal = NO_ACTION;
      if( !height ){
            HeapNode* tmp;
            if( node->right ){
                  tmp = node->right;
                  node->right = NULL;
            }else if( node->left ){
                  tmp = node->left;
                  node->left = NULL;
            }else{
                  return NO_NODE_FOUND;
            }
            root->data = tmp->data;
            tmp->data = NULL;
            free(tmp);
            return NO_ACTION;
      }

      signal = switch_top_last( node->right, root, height - 1, all_to_right );
      
      if( signal != NO_NODE_FOUND )
            return signal;
      
      return switch_top_last( node->left, root, height - 1, 0 );
}



static Data* delete_top( Heap* heap, int min_max ){
      if( !heap->root )
            return NULL;
      Data* top = heap->root->data;
      if( switch_top_last( heap->root, heap->root, heap->height, 1 ) == NO_NODE_FOUND ){
            HeapNode* next = heap->root;
            HeapNode* prev = heap->root;
            while( next->right ){
                  prev = next;
                  next = next->right;
            }
            heap->root->data = next->data;
            next->data = NULL;
            prev->right = NULL;
            free( next );
            heap->height--;
      }
      return top;
}

Data* max_heap_pop_max( Heap* heap ){
      Data* max = delete_top( heap, MAX );
      if( heap->root )
            max_heap_fix( heap->root );
      return max;
}
Data* min_heap_pop_min( Heap* heap ){
      Data* min = delete_top( heap, MIN );
      if( heap->root )
            min_heap_fix( heap->root );
      return min;
}