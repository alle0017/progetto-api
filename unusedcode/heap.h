#include <stdio.h>
#include <stdlib.h>

#define FIX_HEIGHT 2
#define NO_ACTION 1
#define NO_NODE_FOUND 0
#define MAX 0
#define MIN 1

typedef struct HeapNode {
      struct HeapNode* left;
      struct HeapNode* right;
      int key;
} HeapNode;

typedef struct Heap {
      HeapNode* root;
      //ultimo livello completo
      int height;
} Heap;


static HeapNode* new_node( int );
Heap* new_heap();
static int insert( HeapNode*, int, int, int, int );
int max_heap_get_max( Heap* );
int max_heap_delete_max( Heap* );
int get_top( Heap* );
static int fix_descendant( HeapNode*, int );
void max_heap_insert( Heap*, int );

int check_conflict( HeapNode* root ){
      if( !root->left || !root->right )
            return 1;
      return (root->left->key < root->key && root->right->key > root->key) || (root->left->key > root->key && root->right->key < root->key);
}




static HeapNode* new_node( int key ) {
      HeapNode* node = (HeapNode*)malloc( sizeof( HeapNode ) );
      
      if( !node ){
            printf("Heap node allocation failed");
            exit( 1 );
      }
      node->left = NULL;
      node->right = NULL;
      node->key = key;
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
static int insert( HeapNode* root, int key, int height, int all_to_right, int min_max ){
      int flag;
      if ( height<=0){
            // Se per qualche motivo root->left == ?? || root->right == ?? crash
            if( !root )
                  return NO_NODE_FOUND;
            if( root->left && root->right )
                  return NO_NODE_FOUND;

            flag = root->left && all_to_right? FIX_HEIGHT : NO_ACTION;
            if( !root->left ){
                  root->left = new_node(key);
            }else{
                  root->right = new_node(key);
            }
            if( key > root->key && min_max == MAX ){
                  fix_descendant( root, MAX );
            }else if( key < root->key && min_max == MIN ){
                  fix_descendant( root, MIN );
            }
            return flag;
      }

      flag = insert( root->left, key, height - 1, 0, min_max );
      if( flag != NO_NODE_FOUND ){
            if( check_conflict( root ) ){
                  fix_descendant( root, min_max );
            }
            return flag;
      }

      flag = insert( root->right, key, height - 1, all_to_right, min_max );

      if( flag != NO_NODE_FOUND && check_conflict( root ) ){
            fix_descendant( root, min_max );
      }

      return flag;

}
void max_heap_insert( Heap* heap, int key ){

      if( heap->root ){
            if( insert( heap->root, key, heap->height, 1, MAX ) == FIX_HEIGHT ){
                  heap->height++;
            }
            return;
      }

      heap->root = new_node( key );
}
void min_heap_insert( Heap* heap, int key ){

      if( heap->root ){
            if( insert( heap->root, key, heap->height, 1, MIN ) == FIX_HEIGHT ){
                  heap->height++;
            }
            return;
      }

      heap->root = new_node( key );
}

static int fix_descendant(HeapNode* root, int min_max) {
    int tmp;
    tmp = root->key;
    if (min_max == MAX) {
        if (root->right && root->right->key > root->key) {
            root->key = root->right->key;
            root->right->key = tmp;
            return 1;
        } else if (root->left && root->left->key > root->key) {
            root->key = root->left->key;
            root->left->key = tmp;
            return 1;
        }
        return 0;
    } else {
        if (root->right && root->right->key < root->key) {
            root->key = root->right->key;
            root->right->key = tmp;
            return 1;
        } else if (root->left && root->left->key < root->key) {
            root->key = root->left->key;
            root->left->key = tmp;
            return 1;
        }
        return 0;
    }
}
int get_top( Heap* heap ){
      if( !heap->root )
            return 0;
      return heap->root->key;
}

int switch_top_last( HeapNode* node, HeapNode* root, int height, int all_to_right  ){
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
            root->key = tmp->key;
            free(tmp);
            return NO_ACTION;
      }

      signal = switch_top_last( node->right, root, height - 1, all_to_right );
      
      if( signal != NO_NODE_FOUND )
            return signal;
      
      return switch_top_last( node->left, root, height - 1, 0 );
}
void static inline switch_nodes( HeapNode* node1, HeapNode* node2 ){
      int tmp = node1->key;
      node1->key = node2->key;
      node2->key = tmp;
}


void static max_heap_fix( HeapNode* root ){
      if( !root || (!root->left && !root->right) )
            return;
      
      if( !root->right && root->left->key > root->key ){
            switch_nodes( root, root->left );
      }else if( root->right && root->right->key > root->key || root->left->key > root->key ){
            if( root->left->key > root->right->key ){
                  switch_nodes( root, root->left );
                  max_heap_fix( root->right );
            }else{
                  switch_nodes( root, root->right );
                  max_heap_fix( root->left );
            }
      }
}

void static min_heap_fix( HeapNode* root ){
      if( !root )
            return;
      
      if( !root->right && root->left->key < root->key ){
            switch_nodes( root, root->left );
      }else if( root->right && root->right->key < root->key || root->left->key < root->key ){
            if( root->left->key > root->right->key ){
                  switch_nodes( root, root->left );
                  min_heap_fix( root->right );
            }else{
                  switch_nodes( root, root->right );
                  min_heap_fix( root->left );
            }
      }
}

int static delete_top( Heap* heap, int min_max ){
      if( !heap->root )
            return 0;
      int top = heap->root->key;
      if( switch_top_last( heap->root, heap->root, heap->height, 1 ) == NO_NODE_FOUND ){
            HeapNode* next = heap->root;
            HeapNode* prev = heap->root;
            while( next->right ){
                  prev = next;
                  next = next->right;
            }
            heap->root->key = next->key;
            prev->right = NULL;
            free( next );
            heap->height--;
      }
      return top;
}

int max_heap_pop_max( Heap* heap ){
      int max = delete_top( heap, MAX );
      if( heap->root )
            max_heap_fix( heap->root );
      return max;
}
int min_heap_pop_min( Heap* heap ){
      int min = delete_top( heap, MIN );
      if( heap->root )
            min_heap_fix( heap->root );
      return min;
}