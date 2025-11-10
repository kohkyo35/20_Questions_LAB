#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>    // for strcasecmp (used in utils.c sometimes)
#include <ctype.h>
#include "lab5.h"

/* TODO 1: Implement create_question_node
 * - Allocate memory fo#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lab5.h"
*/

/* TODO 1: Implement create_question_node
 * - Allocate memory for a Node structure
 * - Use strdup() to copy the question string (heap allocation)
 * - Set isQuestion to 1
 * - Initialize yes and no pointers to NULL
 * - Return the new node
 */
Node *create_question_node(const char *question) {
    Node *newNode = (Node*)malloc(sizeof(Node));// Allocate memory for Node
    if (newNode == NULL) {
        return NULL; // Check for malloc failure
    }
    newNode->text = strdup(question); // Duplicate question string
    newNode->isQuestion = 1; // Set as question node
    newNode->yes = NULL; // Initialize yes pointer
    newNode->no = NULL; // Initialize no pointer
    return newNode; // Return the new node
    // TODO: Implement this function
}
/* create_question_node
1. Creates a question node for internal tree nodes which allocates memory for Node structure
2. Duplicates the question string, sets isQuestion to 1, and initializes yes/no pointers to NULL*/

/* TODO 2: Implement create_animal_node
 * - Similar to create_question_node but set isQuestion to 0
 * - This represents a leaf node with an animal name
 */
Node *create_animal_node(const char *animal) {
    // TODO: Implement this function
    Node *newNode = (Node*)malloc(sizeof(Node));// Allocate memory for Node
    if (newNode == NULL) {
        return NULL; // Check for malloc failure
    }
    newNode->text = strdup(animal); // Duplicate animal string
    newNode->isQuestion = 0; // Set as animal node
    newNode->yes = NULL; // Initialize yes pointer
    newNode->no = NULL; // Initialize no pointer
    return newNode; // Return the new node
}
/* create_animal_node
1. Creates an animal node for leaf tree nodes which allocates memory for Node structure
2. Duplicates the animal string, sets isQuestion to 0, and initializes yes/no pointers to NULL*/

/* TODO 3: Implement free_tree (recursive)
 * - This is one of the few recursive functions allowed
 * - Base case: if node is NULL, return
 * - Recursively free left subtree (yes)
 * - Recursively free right subtree (no)
 * - Free the text string
 * - Free the node itself
 * IMPORTANT: Free children before freeing the parent!
 */
void free_tree(Node *node) {
    if(node == NULL) {
        return; // Base case: nothing to free
    }
    free_tree(node->yes); // Free yes subtree
    free_tree(node->no); // Free no subtree
    free(node->text); // Free the text string
    free(node); // Free the node itself

    // TODO: Implement this function
}/*free_tree
1. Frees the entire tree rooted at the given node recursively
2. Frees left and right subtrees before freeing the node itself to avoid memory leaks
3. Free yes/left subtree first, then no/right subtree, then the text string, and finally the node*/

/* TODO 4: Implement count_nodes (recursive)
 * - Base case: if root is NULL, return 0
 * - Return 1 + count of left subtree + count of right subtree
 */
int count_nodes(Node *root) {
    // TODO: Implement this function
    if(root == NULL) {
        return 0; // Base case: no nodes
    }
    return 1 + count_nodes(root->yes) + count_nodes(root->no); // Count current node + left + right
    return 0;
}
/*count_nodes
1. recursive function to count total nodes
2. return 1+ number of yes subtree +number of no subtree
3. used by other functions to size arrays/buffers*/

/* ========== Frame Stack (for iterative tree traversal) ========== */

/* TODO 5: Implement fs_init
 * - Allocate initial array of frames (start with capacity 16)
 * - Set size to 0
 * - Set capacity to 16
 */
void fs_init(FrameStack *s) {
    // TODO: Implement this function
    s->capacity = 16; // Initial capacity
    s->size = 0; // Initialize size
    s->frames = (Frame*)malloc(s->capacity * sizeof(Frame)); // Allocate frames array
    if(!s->frames){
        fprintf(stderr, "Memory allocation failed in fs_init\n");
        s->capacity = s->size = 0;
        return; //failure 
    }
}
/*fs_init
1. initializes the frame stack data structure used for iterative tree traversal
2. replaces recursion with an explicit stack for tracking game state setting up game's navigation and undo/redo system
typedef struct Frame {
    Node *node;
    int answeredYes; -1 unset, 0 no, 1 yes 
} Frame;
typedef struct {
    Frame *frames;
    int size;
    int capacity;
} FrameStack;
*/

/* TODO 6: Implement fs_push
 * - Check if size >= capacity
 *   - If so, double the capacity and reallocate the array
 * - Store the node and answeredYes in frames[size]
 * - Increment size
 */
void fs_push(FrameStack *s, Node *node, int answeredYes) {
    // TODO: Implement this function
    if(s->size >= s->capacity) {
        s->capacity *= 2; // Double capacity
        s->frames = realloc(s->frames, s->capacity * sizeof(Frame)); // Reallocate array
        if(!s->frames){
            fprintf(stderr, "Memory allocation failed in fs_push\n");
            return; //failure
        }
    }
    s->frames[s->size].node = node; // Store node
    s->frames[s->size].answeredYes = answeredYes; // Store answeredYes // -1 unset, 0 no, 1 yes
    s->size++; // Increment size
    return; //success
}
/*fs_push
1. Pushes a new frame onto the FrameStack
2. Resizes the internal array if capacity is reached
3. Stores the current node and the answer given to reach that node
*/

/* TODO 7: Implement fs_pop
 * - Decrement size
 * - Return the frame at frames[size]
 * Note: No need to check if empty - caller should use fs_empty() first
 */
Frame fs_pop(FrameStack *s) {
    // No need to check if empty because caller should ensure it's not empty but adding check for safety
    if(fs_empty(s)) {
        fprintf(stderr, "Error: Attempt to pop from empty FrameStack\n");
        exit(1);
    }
    s->size--; // Decrement size
    return s->frames[s->size]; // Return the top frame
}
/*fs_pop
1. removes and returns top frame
2. decrement size by 1
*/

/* TODO 8: Implement fs_empty
 * - Return 1 if size == 0, otherwise return 0
 */
int fs_empty(FrameStack *s) {
    // TODO: Implement this function
    if(s->size == 0) {
        return 1; // Stack is empty
    }
    return 0;
}

/* TODO 9: Implement fs_free
 * - Free the frames array
 * - Set frames pointer to NULL
 * - Reset size and capacity to 0
 */
void fs_free(FrameStack *s) {
    free(s->frames); // Free frames array
    s->frames = NULL; // Set pointer to NULL
    s->size = 0; // Reset size
    s->capacity = 0; // Reset capacity
    // TODO: Implement this function
}

/* ========== Edit Stack (for undo/redo) ========== */

/* TODO 10: Implement es_init
 * Similar to fs_init but for Edit structs
 */
void es_init(EditStack *s) {
    // TODO: Implement this function
    s->capacity = 16; // Initial capacity
    s->size = 0; // Initialize size
    s->edits = (Edit*)malloc(s->capacity * sizeof(Edit)); // Allocate edits array
    if(!s->edits){
        fprintf(stderr, "Memory allocation failed in es_init\n");
        s->capacity = s->size = 0;
        return; //failure
    }
}
/*es_init
1. initializes the edit stack data structure used for undo/redo functionality
2. manages a dynamic array of Edit structs representing changes made to the tree
typedef struct {
    Edit *edits;
    int size;
    int capacity;
} EditStack;
*/

/* TODO 11: Implement es_push
 * Similar to fs_push but for Edit structs
 * - Check capacity and resize if needed
 * - Add edit to array and increment size
 */
void es_push(EditStack *s, Edit e) {
    // TODO: Implement this function
    if(s->size >= s->capacity) {
        s->capacity *= 2; // Double capacity
        s->edits = realloc(s->edits, s->capacity * sizeof(Edit)); // Reallocate array
    }
    s->edits[s->size] = e; // Store edit
    s->size++; // Increment size
}

/* TODO 12: Implement es_pop
 * Similar to fs_pop but for Edit structs
 */
Edit es_pop(EditStack *s) {
    if(es_empty(s)) {
        fprintf(stderr, "Error: Attempt to pop from empty EditStack\n");
        exit(1);
    }
    s->size--; // Decrement size
    return s->edits[s->size]; // Return the top edit
}

/* TODO 13: Implement es_empty
 * Return 1 if size == 0, otherwise 0
 */
int es_empty(EditStack *s) {
    if(s->size == 0) {
        return 1; // Stack is empty
    }
    return 0;
    // TODO: Implement this function
}

/* TODO 14: Implement es_clear
 * - Set size to 0 (don't free memory, just reset)
 * - This is used to clear the redo stack when a new edit is made
 */
void es_clear(EditStack *s) {
    // TODO: Implement this function
    s->size = 0; // Reset size
}

void es_free(EditStack *s) {
    free(s->edits);
    s->edits = NULL;
    s->size = 0;
    s->capacity = 0;
}

void free_edit_stack(EditStack *s) {
    es_free(s);
}

/* ========== Queue (for BFS traversal) ========== */

/* TODO 15: Implement q_init
 * - Set front and rear to NULL
 * - Set size to 0
 */
void q_init(Queue *q) {
    // TODO: Implement this function
    q->front = NULL; // Initialize front
    q->rear = NULL; // Initialize rear
    q->size = 0; // Initialize size
}

/* TODO 16: Implement q_enqueue
 * - Allocate a new QueueNode
 * - Set its treeNode and id fields
 * - Set its next pointer to NULL
 * - If queue is empty (rear == NULL):
 *   - Set both front and rear to the new node
 * - Otherwise:
 *   - Link rear->next to the new node
 *   - Update rear to point to the new node
 * - Increment size
 */
void q_enqueue(Queue *q, Node *node, int id) {
    // TODO: Implement this function
    QueueNode *newNode = (QueueNode*)malloc(sizeof(QueueNode)); // Allocate new node
    newNode->treeNode = node; // Set treeNode
    newNode->id = id; // Set id
    newNode->next = NULL; // Set next to NULL
    if(q->rear == NULL) {
        q->front = newNode; // Queue was empty, set front
        q->rear = newNode; // Set rear
    } else {
        q->rear->next = newNode; // Link new node at end
        q->rear = newNode; // Update rear
    }
    q->size++; // Increment size
}

/* TODO 17: Implement q_dequeue
 * - If queue is empty (front == NULL), return 0
 * - Save the front node's data to output parameters (*node, *id)
 * - Save front in a temp variable
 * - Move front to front->next
 * - If front is now NULL, set rear to NULL too
 * - Free the temp node
 * - Decrement size
 * - Return 1
 */
int q_dequeue(Queue *q, Node **node, int *id) {
    // TODO: Implement this function
    if(q->front == NULL) {
        return 0; // Queue is empty
    }
    QueueNode *temp = q->front; // Save front node
    *node = temp->treeNode; // Output treeNode
    *id = temp->id; // Output id
    q->front = q->front->next; // Move front to next
    if(q->front == NULL) {
        q->rear = NULL; // Queue is now empty
    }
    free(temp); // Free old front node
    q->size--; // Decrement size
    return 1; // Successful dequeue

    return 0;
}

/* TODO 18: Implement q_empty
 * Return 1 if size == 0, otherwise 0
 */
int q_empty(Queue *q) {
    // TODO: Implement this function
    if(q->size == 0) {
        return 1; // Queue is empty
    }
    return 0;
}

/* TODO 19: Implement q_free
 * - Dequeue all remaining nodes
 * - Use a loop with q_dequeue until queue is empty
 */
void q_free(Queue *q){
    Node *node;
    int id;
    while(!q_empty(q)) {
        q_dequeue(q, &node, &id); // Dequeue and discard
    }
    // TODO: Implement this function
}

/* ========== Hash Table ========== */

/* TODO 20: Implement canonicalize
 * Convert a string to canonical form for hashing:
 * - Convert to lowercase
 * - Keep only alphanumeric characters
 * - Replace spaces with underscores
 * - Remove punctuation
 * Example: "Does it meow?" -> "does_it_meow"
 * 
 * Steps:
 * - Allocate result buffer (strlen(s) + 1)
 * - Iterate through input string
 * - For each character:
 *   - If alphanumeric: add lowercase version to result
 *   - If whitespace: add underscore
 *   - Otherwise: skip it
 * - Null-terminate result
 * - Return the new string
 */
char *canonicalize(const char *s) {
    // TODO: Implement this function
    int len = strlen(s);
    char *result = (char*)malloc(len + 1); // Allocate result buffer
    if (result == NULL) {
        return NULL; // Check for malloc failure
    }
    int j = 0; // Index for result
    for (int i = 0; i < len; i++) {
        if (isalnum((unsigned char)s[i])) {
            result[j++] = tolower((unsigned char)s[i]); // Add lowercase alphanumeric
        } else if (isspace((unsigned char)s[i])) {
            result[j++] = '_'; // Add underscore for whitespace
        }
        // Ignore punctuation
    }
    result[j] = '\0'; // Null-terminate result
    return result; // Return the new string
    return NULL;
}

/* TODO 21: Implement h_hash (djb2 algorithm)
 * unsigned hash = 5381;
 * For each character c in the string:
 *   hash = ((hash << 5) + hash) + c;  // hash * 33 + c
 * Return hash
 */
unsigned h_hash(const char *s) {
    // TODO: Implement this function

    unsigned hash = 5381; // Initial hash value
    int c;
    while ((c = (unsigned char)*s++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash; // Return computed hash
    return 0;

}

/* TODO 22: Implement h_init
 * - Allocate buckets array using calloc (initializes to NULL)
 * - Set nbuckets field
 * - Set size to 0
 */
void h_init(Hash *h, int nbuckets) {
    // TODO: Implement this function

    h->buckets = (Entry**)calloc(nbuckets, sizeof(Entry*)); // Allocate buckets
    h->nbuckets = nbuckets; // Set number of buckets
    h->size = 0; // Initialize size

}

/* TODO 23: Implement h_put
 * Add animalId to the list for the given key
 * 
 * Steps:
 * 1. Compute bucket index: idx = h_hash(key) % nbuckets
 * 2. Search the chain at buckets[idx] for an entry with matching key
 * 3. If found:
 *    - Check if animalId already exists in the vals list
 *    - If yes, return 0 (no change)
 *    - If no, add animalId to vals.ids array (resize if needed), return 1
 * 4. If not found:
 *    - Create new Entry with strdup(key)
 *    - Initialize vals with initial capacity (e.g., 4)
 *    - Add animalId as first element
 *    - Insert at head of chain (buckets[idx])
 *    - Increment h->size
 *    - Return 1
 */
int h_put(Hash *h, const char *key, int animalId) {
    // TODO: Implement this function
    
    unsigned idx = h_hash(key) % h->nbuckets; // Compute bucket index
    Entry *current = h->buckets[idx];
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            // Key found, check for existing animalId
            for (int i = 0; i < current->vals.count; i++) {
                if (current->vals.ids[i] == animalId) {
                    return 0; // animalId already exists
                }
            }
            // Add new animalId
            if (current->vals.count >= current->vals.capacity) {
                current->vals.capacity *= 2; // Double capacity
                current->vals.ids = realloc(current->vals.ids, current->vals.capacity * sizeof(int)); // Reallocate array
            }
            current->vals.ids[current->vals.count++] = animalId; // Add animalId
            return 1; // Successfully added
        }
        current = current->next; // Move to next entry
    }
    // Key not found, create new entry
    Entry *newEntry = (Entry*)malloc(sizeof(Entry)); // Allocate new entry
    newEntry->key = strdup(key); // Duplicate key string
    newEntry->vals.ids = (int*)malloc(4 * sizeof(int)); // Initial capacity
    newEntry->vals.capacity = 4; // Set initial capacity
    newEntry->vals.count = 1; // Initialize count
    newEntry->vals.ids[0] = animalId; // Add animalId
    newEntry->next = h->buckets[idx]; // Insert at head of chain
    h->buckets[idx] = newEntry; // Update bucket head
    h->size++; // Increment size
    return 1; // Successfully added
    return 0;
    
}

/* TODO 24: Implement h_contains
 * Check if the hash table contains the given key-animalId pair
 * 
 * Steps:
 * 1. Compute bucket index
 * 2. Search the chain for matching key
 * 3. If found, search vals.ids array for animalId
 * 4. Return 1 if found, 0 otherwise
 */
int h_contains(const Hash *h, const char *key, int animalId) {
    // TODO: Implement this function
    unsigned idx = h_hash(key) % h->nbuckets; // Compute bucket index
    Entry *current = h->buckets[idx];
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            // Key found, search for animalId
            for (int i = 0; i < current->vals.count; i++) {
                if (current->vals.ids[i] == animalId) {
                    return 1; // animalId found
                }
            }
            return 0; // animalId not found
        }
        current = current->next; // Move to next entry
    }
    return 0;
    
}

/* TODO 25: Implement h_get_ids
 * Return pointer to the ids array for the given key
 * Set *outCount to the number of ids
 * Return NULL if key not found
 * 
 * Steps:
 * 1. Compute bucket index
 * 2. Search chain for matching key
 * 3. If found:
 *    - Set *outCount = vals.count
 *    - Return vals.ids
 * 4. If not found:
 *    - Set *outCount = 0
 *    - Return NULL
 */
int *h_get_ids(const Hash *h, const char *key, int *outCount) {
    // TODO: Implement this function
    unsigned idx = h_hash(key) % h->nbuckets; // Compute bucket index
    Entry *current = h->buckets[idx];
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            // Key found
            *outCount = current->vals.count; // Set outCount
            return current->vals.ids; // Return ids array
        }
        current = current->next; // Move to next entry
    }
    *outCount = 0;
    return NULL;
    
}

/* TODO 26: Implement h_free
 * Free all memory associated with the hash table
 * 
 * Steps:
 * - For each bucket:
 *   - Traverse the chain
 *   - For each entry:
 *     - Free the key string
 *     - Free the vals.ids array
 *     - Free the entry itself
 * - Free the buckets array
 * - Set buckets to NULL, size to 0
 */
void h_free(Hash *h) {
    // TODO: Implement this function
    
    for (int i = 0; i < h->nbuckets; i++) {
        Entry *current = h->buckets[i];
        while (current != NULL) {
            Entry *temp = current;
            current = current->next;
            free(temp->key); // Free key string
            free(temp->vals.ids); // Free ids array
            free(temp); // Free entry
        }
    }
    free(h->buckets); // Free buckets array
    h->buckets = NULL; // Set to NULL
    h->size = 0; // Reset size
    
}