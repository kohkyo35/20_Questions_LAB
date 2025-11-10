#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lab5.h"
#include <ncurses.h>

extern Node *g_root;

/* TODO 29: Implement check_integrity
 * Use BFS to verify tree structure:
 * - Question nodes must have both yes and no children (not NULL)
 * - Leaf nodes (isQuestion == 0) must have NULL children
 * 
 * Return 1 if valid, 0 if invalid
 * 
 * Steps:
 * 1. Return 1 if g_root is NULL (empty tree is valid)
 * 2. Initialize queue and enqueue root with id=0
 * 3. Set valid = 1
 * 4. While queue not empty:
 *    - Dequeue node
 *    - If node->isQuestion:
 *      - Check if yes == NULL or no == NULL    
 *      - If so, set valid = 0 and break
 *      - Otherwise, enqueue both children
 *    - Else (leaf node):
 *      - Check if yes != NULL or no != NULL
 *      - If so, set valid = 0 and break
 * 5. Free queue and return valid
 */
//This function verifies that the binary tree structure is valid according to the game rules
//performs a BFS from the root and checks each node's children based on whether it's a question or leaf
//returns 1 if the tree is valid, 0 otherwise
int check_integrity() {
    // Empty tree is valid
    if (g_root == NULL) {
        return 1;
    }

    Queue q;
    q_init(&q);
    int valid = 1;// I first assumed it's valid until proven otherwise

    // Start BFS with root
    q_enqueue(&q, g_root, 0);

    Node *node;
    int id;
    // BFS traversal
    while (!q_empty(&q) && valid) {
        if (!q_dequeue(&q, &node, &id)) {
            break;
        }
        //case 1: internal question node
        if (node->isQuestion) {
            // Question nodes must have both children
            if (node->yes == NULL || node->no == NULL) {
                valid = 0;
                break;
            }
            // Enqueue children for checking
            q_enqueue(&q, node->yes, id + 1);
            q_enqueue(&q, node->no, id + 2);
            //case 2: leaf node
        } else {
            // Leaf nodes must not have any children
            if (node->yes != NULL || node->no != NULL) {
                valid = 0;
                break;
            }
        }
    }
    
    q_free(&q);
    return valid;
}
///////////////////////////////////////////////////////////////////////////
//FIND SHORTEST DISTINGUISHING PATH
///////////////////////////////////////////////////////////////////////////
/* TODO 30: Implement find_shortest_path
 * Find and display the shortest distinguishing path between two animals
 * 
 * Steps:
 * 1. Validate input animals; return error if invalid
 * 2. Use BFS to locate both animals in the tree, storing parent pointers
 * 3. If either animal not found, display error and return
 * 4. Backtrack from both animals to root to find LCA
 * 5. Collect and display distinguishing questions from LCA to each animal
*/

typedef struct PathNode {
    Node *treeNode; //actual tree node
    struct PathNode *parent;//poitner to previous PathNode
    int viaYes;  // 1 if reached via yes, 0 if via no, -1 for root
} PathNode;

void find_shortest_path(const char *animal1, const char *animal2) {
    if (g_root == NULL) {
        mvprintw(4, 3, "Error: Tree is empty. Initialize first.");
        refresh();
        return;
    }//checking for sanity

    if (!animal1 || !animal2) {
        mvprintw(4, 3, "Error: Invalid input.");
        refresh();
        return;
    }//check for inputs being NULL

    //clean up input strings
    char a1[128], a2[128];
    strncpy(a1, animal1, sizeof(a1) - 1);
    strncpy(a2, animal2, sizeof(a2) - 1);
    a1[sizeof(a1) - 1] = '\0';
    a2[sizeof(a2) - 1] = '\0';
    a1[strcspn(a1, "\n")] = '\0';
    a2[strcspn(a2, "\n")] = '\0';

    //if the two inputs are the same animal, no distinguishing path exists
    if (strcasecmp(a1, a2) == 0) {
        mvprintw(4, 3, "Both animals are the same: %s", a1);
        refresh();
        return;
    }
    //count total nodes to size pools
    int maxNodes = count_nodes(g_root);
    if (maxNodes <= 0) {
        mvprintw(4, 3, "Error: Invalid tree structure.");
        refresh();
        return;
    }
    //BFS to find both animals and store parent pointers
    //pool holds all PathNodes created during BFS
    PathNode *pool = malloc(sizeof(PathNode) * maxNodes);
    int *queue = malloc(sizeof(int) * maxNodes);
    if (!pool || !queue) {
        mvprintw(4, 3, "Error: Memory allocation failed.");
        refresh();
        free(pool);
        free(queue);
        return;
    }
    // Initialize BFS
    int front = 0, rear = 0, poolCount = 0;
    pool[poolCount].treeNode = g_root;
    pool[poolCount].parent = NULL;
    pool[poolCount].viaYes = -1;
    queue[rear++] = poolCount++;

    int idx1 = -1, idx2 = -1;//indices of found animals initially -1 (not found)
    //when found they will be updated to their respective indices in pool

    //BFS traversal starts here - find both target animals
    while (front < rear && (idx1 == -1 || idx2 == -1)) {
        int idx = queue[front++];
        Node *cur = pool[idx].treeNode;
        //check if it's a leaf and matches either target animal
        if (!cur->isQuestion) {
            if (strcasecmp(cur->text, a1) == 0 && idx1 == -1)
                idx1 = idx;
            if (strcasecmp(cur->text, a2) == 0 && idx2 == -1)
                idx2 = idx;
        }
        //if yes child exists, add to pool and queue
        if (cur->yes) {
            pool[poolCount].treeNode = cur->yes;
            pool[poolCount].parent = &pool[idx];
            pool[poolCount].viaYes = 1;
            queue[rear++] = poolCount++;
        }
        //if no child exists, add to pool and queue
        if (cur->no) {
            pool[poolCount].treeNode = cur->no;
            pool[poolCount].parent = &pool[idx];
            pool[poolCount].viaYes = 0;
            queue[rear++] = poolCount++;
        }
    }
    // Step 3: Check if both animals were found, else display error
    if (idx1 == -1 || idx2 == -1) {
        mvprintw(4, 3, "Error: ");
        if (idx1 == -1) mvprintw(4, 11, "Animal not found: %s", a1);
        if (idx2 == -1) mvprintw(5, 11, "Animal not found: %s", a2);
        refresh();
        free(pool);
        free(queue);
        return;
    }
    // Step 4: Backtrack to find LCA
    //it stores the paths from each animal to the root
    PathNode *path1[1000];
    PathNode *path2[1000];
    int len1 = 0, len2 = 0;
    //tracing back from animal1 to root
    PathNode *p = &pool[idx1];
    while (p) {
        path1[len1++] = p;
        p = p->parent;
    }
    //tracing back from animal2 to root
    p = &pool[idx2];
    while (p) {
        path2[len2++] = p;
        p = p->parent;
    }
    //reverse both paths to have root to leaf order
    for (int i = 0; i < len1 / 2; i++) {
        PathNode *tmp = path1[i];
        path1[i] = path1[len1 - 1 - i];
        path1[len1 - 1 - i] = tmp;
    }
    //reverse path2
    for (int i = 0; i < len2 / 2; i++) {
        PathNode *tmp = path2[i];
        path2[i] = path2[len2 - 1 - i];
        path2[len2 - 1 - i] = tmp;
    }
    //this is where we find the LCA by comparing paths, find when two paths diverge
    int i1 = len1 - 1, i2 = len2 - 1;
    PathNode *lca = NULL;
    while (i1 >= 0 && i2 >= 0 && path1[i1]->treeNode == path2[i2]->treeNode) {
        lca = path1[i1];
        i1--;
        i2--;
    }
    if(lca == NULL) lca = &pool[0]; //root
    if(i1<0) i1=0;//safety
    if(i2<0) i2=0;

    // Step 5: Display distinguishing questions from LCA to each animal
    int baseY = 10;//this is a base Y position for displaying output because I hate writing numbers every time
    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(baseY, 0, "%-80s", "Find Shortest Distinguishing Path");
    attroff(COLOR_PAIR(3) | A_BOLD);

    mvprintw(baseY+2, 3, "Distinguishing question(s) between '%s' and '%s':", a1, a2);
    //compare both paths to find the first difference
    int diffIdx = -1;
    int minLen = (len1 < len2) ? len1 : len2;//minimum length of the two paths
    for (int i = 0; i < minLen; i++) {
        if (path1[i]->treeNode != path2[i]->treeNode) {
            diffIdx = i - 1;  // LCA index
            break;
        }
    }
    if (diffIdx == -1) diffIdx = minLen - 1;
    //get the distinguishing question node
    Node *distinguishQ = NULL;
    if (diffIdx + 1 < len1 && diffIdx + 1 < len2) {
        Node *candidate1 = path1[diffIdx + 1]->treeNode;//next node in path1
        Node *candidate2 = path2[diffIdx + 1]->treeNode;//next node in path2
        if (path1[diffIdx]->treeNode->isQuestion) {//if LCA is a question node
            distinguishQ = path1[diffIdx]->treeNode;//LCA is the distinguishing question
        } else if (candidate1->isQuestion) {//if candidate1 is a question node
            distinguishQ = candidate1;//set as distinguishing question
        } else if (candidate2->isQuestion) {//if candidate2 is a question node
            distinguishQ = candidate2;
        }//else no distinguishing question found
    }

    if (!distinguishQ) {
        mvprintw(baseY+4, 5, "No distinguishing question found.");
    } else {
        mvprintw(baseY+4, 5, "Q: %s", distinguishQ->text);
        //display paths from LCA to each animal
        PathNode *child1 = path1[diffIdx + 1];
        PathNode *child2 = path2[diffIdx + 1];
        //display how to reach each animal from the distinguishing question
        mvprintw(baseY+6, 7, "Path to %s: %s", a1,
                (child1->viaYes == 1) ? "Yes" : "No");//if viaYes is 1 then "Yes" else "No"
        mvprintw(baseY+7, 7, "Path to %s: %s", a2,
                (child2->viaYes == 1) ? "Yes" : "No");//if viaYes is 1 then "Yes" else "No"
    }

    mvprintw(baseY+9, 2, "Press any key to continue...");
    refresh();

    free(pool);
    free(queue);
}