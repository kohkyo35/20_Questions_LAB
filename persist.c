#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "lab5.h"

extern Node *g_root;

#define MAGIC 0x41544C35  /* "ATL5" */
#define VERSION 1

typedef struct {
    Node *node;
    int id;
} NodeMapping;//temporarily stores node to ID mappings during save/load

//serailize the tree to a binary file using BFS to assign IDs and write nodes
//ensures consistent structure for later loading, child relationships via IDs
int save_tree(const char *filename) {
    /* TODO 27: Implement save_tree
 * Save the tree to a binary file using BFS traversal
 * 
 * Binary format:
 * - Header: magic (4 bytes), version (4 bytes), nodeCount (4 bytes)
 * - For each node in BFS order:
 *   - isQuestion (1 byte)
 *   - textLen (4 bytes)
 *   - text (textLen bytes, no null terminator)
 *   - yesId (4 bytes, -1 if NULL)
 *   - noId (4 bytes, -1 if NULL)
 * 
 * Steps:
 * 1. Return 0 if g_root is NULL
 * 2. Open file for writing binary ("wb")
 * 3. Initialize queue and NodeMapping array
 * 4. Use BFS to assign IDs to all nodes:
 *    - Enqueue root with id=0
 *    - Store mapping[0] = {g_root, 0}
 *    - While queue not empty:
 *      - Dequeue node and id
 *      - If node has yes child: add to mappings, enqueue with new id
 *      - If node has no child: add to mappings, enqueue with new id
 * 5. Write header (magic, version, nodeCount)
 * 6. For each node in mapping order:
 *    - Write isQuestion, textLen, text bytes
 *    - Find yes child's id in mappings (or -1)
 *    - Find no child's id in mappings (or -1)
 *    - Write yesId, noId
 * 7. Clean up and return 1 on success
 */
    // TODO: Implement this function
    // This is complex - break it into smaller steps
    // You'll need to use the Queue functions you implemented
    if(g_root == NULL) {
        return 0; // Nothing to save
    }
    
    // Open file for writing binary
    FILE *file = fopen(filename, "wb");
    if(file == NULL) {
        return 0; // Failed to open file
    }
    
    // Step 3: Initialize queue and mapping
    Queue q;
    q_init(&q);// Initialize queue
    NodeMapping *mappings = NULL;// Dynamic array for node-ID mappings
    int mapping_capacity = 16;
    int mapping_size = 0;
    mappings = (NodeMapping*)malloc(mapping_capacity * sizeof(NodeMapping));// Allocate initial mapping array
    if(mappings == NULL) {
        fclose(file);
        return 0; // Memory allocation failure
    }
    // Step 4: BFS to assign IDs
    q_enqueue(&q, g_root, 0);// Enqueue root
    mappings[mapping_size].node = g_root;// Map root to ID 0
    mappings[mapping_size].id = 0;// ID 0
    mapping_size++;
    while(!q_empty(&q)) {// While queue not empty
        Node *current;
        int current_id;
        q_dequeue(&q, &current, &current_id);// Dequeue node
        // Yes child
        if(current->yes != NULL) {
            if(mapping_size >= mapping_capacity) {// Resize mapping array
                mapping_capacity *= 2;// Double capacity because we need more space
                mappings = (NodeMapping*)realloc(mappings, mapping_capacity * sizeof(NodeMapping));// Reallocate
                if(mappings == NULL) {
                    q_free(&q);
                    fclose(file);
                    return 0; // Memory allocation failure
                }
            }
            mappings[mapping_size].node = current->yes;// Map yes child
            mappings[mapping_size].id = mapping_size;// Assign new ID
            q_enqueue(&q, current->yes, mapping_size);// Enqueue yes child
            mapping_size++;// Increment size
        }
        // No child
        if(current->no != NULL) {
            if(mapping_size >= mapping_capacity) {// Resize mapping array
                mapping_capacity *= 2;// Double capacity
                mappings = (NodeMapping*)realloc(mappings, mapping_capacity * sizeof(NodeMapping));
                if(mappings == NULL) {
                    q_free(&q);
                    fclose(file);
                    return 0; // Memory allocation failure
                }
            }
            mappings[mapping_size].node = current->no;// Map no child// Assign new ID
            mappings[mapping_size].id = mapping_size;// Assign new ID
            q_enqueue(&q, current->no, mapping_size);// Enqueue no child
            mapping_size++;
        }
    }
    q_free(&q);
    // Step 5: Write header
    uint32_t magic = MAGIC;
    uint32_t version = VERSION;
    uint32_t nodeCount = mapping_size;
    fwrite(&magic, sizeof(uint32_t), 1, file);// Write magic number to file 
    fwrite(&version, sizeof(uint32_t), 1, file);
    fwrite(&nodeCount, sizeof(uint32_t), 1, file);
    // Step 6: Write nodes
    for(int i = 0; i < mapping_size; i++) {// For each mapped node
        Node *node = mappings[i].node;
        uint8_t isQuestion = (uint8_t)(node->isQuestion);
        uint32_t textLen = (uint32_t)strlen(node->text);
        fwrite(&isQuestion, sizeof(uint8_t), 1, file);
        fwrite(&textLen, sizeof(uint32_t), 1, file);
        fwrite(node->text, sizeof(char), textLen, file);
        // Find yesId
        int32_t yesId = -1;
        int32_t noId = -1;
        for(int j = 0; j < mapping_size; j++) {
            if(mappings[j].node == node->yes) {// Find yesId
                yesId = mappings[j].id;
            }
            if(mappings[j].node == node->no) {// Find noId
                noId = mappings[j].id;
            }
        }
        fwrite(&yesId, sizeof(int32_t), 1, file);// Write yesId
        fwrite(&noId, sizeof(int32_t), 1, file);
    }
    // Clean up
    free(mappings);
    fclose(file);
    return 1; // Success
}

/* TODO 28: Implement load_tree
 * Load a tree from a binary file and reconstruct the structure
 * 
 * Steps:
 * 1. Open file for reading binary ("rb")
 * 2. Read and validate header (magic, version, count)
 * 3. Allocate arrays for nodes and child IDs:
 *    - Node **nodes = calloc(count, sizeof(Node*))
 *    - int32_t *yesIds = calloc(count, sizeof(int32_t))
 *    - int32_t *noIds = calloc(count, sizeof(int32_t))
 * 4. Read each node:
 *    - Read isQuestion, textLen
 *    - Validate textLen (e.g., < 10000)
 *    - Allocate and read text string (add null terminator!)
 *    - Read yesId, noId
 *    - Validate IDs are in range [-1, count)
 *    - Create Node and store in nodes[i]
 * 5. Link nodes using stored IDs:
 *    - For each node i:
 *      - If yesIds[i] >= 0: nodes[i]->yes = nodes[yesIds[i]]
 *      - If noIds[i] >= 0: nodes[i]->no = nodes[noIds[i]]
 * 6. Free old g_root if not NULL
 * 7. Set g_root = nodes[0]
 * 8. Clean up temporary arrays
 * 9. Return 1 on success
 * 
 * Error handling:
 * - If any read fails or validation fails, goto load_error
 * - In load_error: free all allocated memory and return 0
 */
//Read and validate file header
//allocate memory for node arrays and yes/no ID arrays
//read each node's data and create Node structures
//link nodes based on stored yes/no IDs
//set g_root to the reconstructed tree
//clean up and return success/failure
int load_tree(const char *filename) {
    // TODO: Implement this function
    // This is the most complex function in the lab
    // Take it step by step and test incrementally
    FILE *file = fopen(filename, "rb");
    if(file == NULL) {
        return 0; // Failed to open file
    }
    // Step 2: Read and validate header
    uint32_t magic, version, nodeCount;
    if(fread(&magic, sizeof(uint32_t), 1, file) != 1 ||
       fread(&version, sizeof(uint32_t), 1, file) != 1 ||
       fread(&nodeCount, sizeof(uint32_t), 1, file) != 1) {
        fclose(file);
        return 0; // Failed to read header
    }
    if(magic != MAGIC || version != VERSION) {
        fclose(file);
        return 0; // Invalid file format
    }
    // Step 3: Allocate arrays
    Node **nodes = (Node**)calloc(nodeCount, sizeof(Node*));
    int32_t *yesIds = (int32_t*)calloc(nodeCount, sizeof(int32_t));
    int32_t *noIds = (int32_t*)calloc(nodeCount, sizeof(int32_t));
    if(nodes == NULL || yesIds == NULL || noIds == NULL) {
        fclose(file);
        free(nodes);
        free(yesIds);
        free(noIds);
        return 0; // Memory allocation failure
    }
    // Step 4: Read each node
    for(uint32_t i = 0; i < nodeCount; i++) {// For each node
        uint8_t isQuestion;
        uint32_t textLen;
        if(fread(&isQuestion, sizeof(uint8_t), 1, file) != 1 ||
           fread(&textLen, sizeof(uint32_t), 1, file) != 1) {
            goto load_error;
        }
        if(textLen >= 10000) { // Arbitrary limit to prevent abuse
            goto load_error;
        }
        char *text = (char*)malloc(textLen + 1);
        if(text == NULL) {
            goto load_error;
        }
        if(fread(text, sizeof(char), textLen, file) != textLen) {
            free(text);
            goto load_error;
        }
        text[textLen] = '\0'; // Null terminate
        if(fread(&yesIds[i], sizeof(int32_t), 1, file) != 1 ||
           fread(&noIds[i], sizeof(int32_t), 1, file) != 1) {
            free(text);
            goto load_error;
        }
        if((yesIds[i] < -1 || yesIds[i] >= (int32_t)nodeCount) ||
           (noIds[i] < -1 || noIds[i] >= (int32_t)nodeCount)) {
            free(text);
            goto load_error;
        }
        // Create node
        Node *newNode = (Node*)malloc(sizeof(Node));
        if(newNode == NULL) {
            free(text);
            goto load_error;
        }
        newNode->text = text;
        newNode->isQuestion = (int)isQuestion;
        newNode->yes = NULL;
        newNode->no = NULL;
        nodes[i] = newNode;
    }
    // Step 5: Link nodes
    for(uint32_t i = 0; i < nodeCount; i++) {
        if(yesIds[i] >= 0) {
            nodes[i]->yes = nodes[yesIds[i]];
        }
        if(noIds[i] >= 0) {
            nodes[i]->no = nodes[noIds[i]];
        }
    }
    // Step 6: Free old g_root
    if(g_root != NULL) {
        free_tree(g_root);
    }
    // Step 7: Set g_root
    g_root = nodes[0];
    // Step 8: Clean up
    free(nodes);
    free(yesIds);
    free(noIds);
    fclose(file);
    return 1; // Success

load_error:
    // Clean up all allocated memory
    if (nodes) {
        for (uint32_t i = 0; i < nodeCount; i++) {
            if (nodes[i]) {
                free(nodes[i]->text);
                free(nodes[i]);
            }
        }
        free(nodes);
    }
    free(yesIds);
    free(noIds);
    fclose(file);
    return 0;
}
