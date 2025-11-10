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
int check_integrity() {
    // Empty tree is valid
    if (g_root == NULL) {
        return 1;
    }

    Queue q;
    q_init(&q);
    int valid = 1;

    // Start BFS with root
    q_enqueue(&q, g_root, 0);

    Node *node;
    int id;
    while (!q_empty(&q) && valid) {
        if (!q_dequeue(&q, &node, &id)) {
            break;
        }

        if (node->isQuestion) {
            // Question nodes must have both children
            if (node->yes == NULL || node->no == NULL) {
                valid = 0;
                break;
            }
            // Enqueue children for checking
            q_enqueue(&q, node->yes, id + 1);
            q_enqueue(&q, node->no, id + 2);
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
    Node *treeNode;
    struct PathNode *parent;
    int viaYes;  // 1 if reached via yes, 0 if via no, -1 for root
} PathNode;

void find_shortest_path(const char *animal1, const char *animal2) {
    if (g_root == NULL) {
        mvprintw(4, 3, "Error: Tree is empty. Initialize first.");
        refresh();
        return;
    }

    if (!animal1 || !animal2) {
        mvprintw(4, 3, "Error: Invalid input.");
        refresh();
        return;
    }

    char a1[128], a2[128];
    strncpy(a1, animal1, sizeof(a1) - 1);
    strncpy(a2, animal2, sizeof(a2) - 1);
    a1[sizeof(a1) - 1] = '\0';
    a2[sizeof(a2) - 1] = '\0';
    a1[strcspn(a1, "\n")] = '\0';
    a2[strcspn(a2, "\n")] = '\0';

    if (strcasecmp(a1, a2) == 0) {
        mvprintw(4, 3, "Both animals are the same: %s", a1);
        refresh();
        return;
    }

    int maxNodes = count_nodes(g_root);
    if (maxNodes <= 0) {
        mvprintw(4, 3, "Error: Invalid tree structure.");
        refresh();
        return;
    }

    PathNode *pool = malloc(sizeof(PathNode) * maxNodes);
    int *queue = malloc(sizeof(int) * maxNodes);
    if (!pool || !queue) {
        mvprintw(4, 3, "Error: Memory allocation failed.");
        refresh();
        free(pool);
        free(queue);
        return;
    }

    int front = 0, rear = 0, poolCount = 0;
    pool[poolCount].treeNode = g_root;
    pool[poolCount].parent = NULL;
    pool[poolCount].viaYes = -1;
    queue[rear++] = poolCount++;

    int idx1 = -1, idx2 = -1;

    while (front < rear && (idx1 == -1 || idx2 == -1)) {
        int idx = queue[front++];
        Node *cur = pool[idx].treeNode;

        if (!cur->isQuestion) {
            if (strcasecmp(cur->text, a1) == 0 && idx1 == -1)
                idx1 = idx;
            if (strcasecmp(cur->text, a2) == 0 && idx2 == -1)
                idx2 = idx;
        }

        if (cur->yes) {
            pool[poolCount].treeNode = cur->yes;
            pool[poolCount].parent = &pool[idx];
            pool[poolCount].viaYes = 1;
            queue[rear++] = poolCount++;
        }
        if (cur->no) {
            pool[poolCount].treeNode = cur->no;
            pool[poolCount].parent = &pool[idx];
            pool[poolCount].viaYes = 0;
            queue[rear++] = poolCount++;
        }
    }

    if (idx1 == -1 || idx2 == -1) {
        mvprintw(4, 3, "Error: ");
        if (idx1 == -1) mvprintw(4, 11, "Animal not found: %s", a1);
        if (idx2 == -1) mvprintw(5, 11, "Animal not found: %s", a2);
        refresh();
        free(pool);
        free(queue);
        return;
    }

    PathNode *path1[1000];
    PathNode *path2[1000];
    int len1 = 0, len2 = 0;

    PathNode *p = &pool[idx1];
    while (p) {
        path1[len1++] = p;
        p = p->parent;
    }

    p = &pool[idx2];
    while (p) {
        path2[len2++] = p;
        p = p->parent;
    }

    for (int i = 0; i < len1 / 2; i++) {
        PathNode *tmp = path1[i];
        path1[i] = path1[len1 - 1 - i];
        path1[len1 - 1 - i] = tmp;
    }
    for (int i = 0; i < len2 / 2; i++) {
        PathNode *tmp = path2[i];
        path2[i] = path2[len2 - 1 - i];
        path2[len2 - 1 - i] = tmp;
    }

    int i1 = len1 - 1, i2 = len2 - 1;
    PathNode *lca = NULL;
    while (i1 >= 0 && i2 >= 0 && path1[i1]->treeNode == path2[i2]->treeNode) {
        lca = path1[i1];
        i1--;
        i2--;
    }
    if(lca == NULL) lca = &pool[0]; //root
    if(i1<0) i1=0;
    if(i2<0) i2=0;

    int baseY = 10;
    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(baseY, 0, "%-80s", "Find Shortest Distinguishing Path");
    attroff(COLOR_PAIR(3) | A_BOLD);

    mvprintw(baseY+2, 3, "Distinguishing question(s) between '%s' and '%s':", a1, a2);

    int diffIdx = -1;
    int minLen = (len1 < len2) ? len1 : len2;
    for (int i = 0; i < minLen; i++) {
        if (path1[i]->treeNode != path2[i]->treeNode) {
            diffIdx = i - 1;  // LCA index
            break;
        }
    }
    if (diffIdx == -1) diffIdx = minLen - 1;

    Node *distinguishQ = NULL;
    if (diffIdx + 1 < len1 && diffIdx + 1 < len2) {
        Node *candidate1 = path1[diffIdx + 1]->treeNode;
        Node *candidate2 = path2[diffIdx + 1]->treeNode;
        if (path1[diffIdx]->treeNode->isQuestion) {
            distinguishQ = path1[diffIdx]->treeNode;
        } else if (candidate1->isQuestion) {
            distinguishQ = candidate1;
        } else if (candidate2->isQuestion) {
            distinguishQ = candidate2;
        }
    }

    if (!distinguishQ) {
        mvprintw(baseY+4, 5, "No distinguishing question found.");
    } else {
        mvprintw(baseY+4, 5, "Q: %s", distinguishQ->text);

        PathNode *child1 = path1[diffIdx + 1];
        PathNode *child2 = path2[diffIdx + 1];

        mvprintw(baseY+6, 7, "Path to %s: %s", a1,
                (child1->viaYes == 1) ? "Yes" : "No");
        mvprintw(baseY+7, 7, "Path to %s: %s", a2,
                (child2->viaYes == 1) ? "Yes" : "No");
    }

    mvprintw(baseY+9, 2, "Press any key to continue...");
    refresh();

    free(pool);
    free(queue);
}

/*
    int baseY = 10;
    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(baseY, 0, "%-80s", "Find Shortest Distinguishing Path");
    attroff(COLOR_PAIR(3) | A_BOLD);

    mvprintw(baseY+2, 3, "Distinguishing question(s) between '%s' and '%s':", a1, a2);

    if (!lca || !lca->treeNode->isQuestion) {
        mvprintw(baseY+4, 5, "No distinguishing question found.");
    } else {
        Node *q = lca->treeNode;
        mvprintw(baseY+4, 5, "Q: %s", q->text);
        if(i1 >= len1) i1 = len1 - 1;
        if(i2 >= len2) i2 = len2 - 1;

        PathNode *child1 = path1[i1];
        PathNode *child2 = path2[i2];

        mvprintw(baseY+6, 7, "Path to %s: %s", a1,
                 (child1->viaYes == 1) ? "Yes" : "No");
        mvprintw(baseY+7, 7, "Path to %s: %s", a2,
                 (child2->viaYes == 1) ? "Yes" : "No");
    }

    mvprintw(baseY+9, 2, "Press any key to continue...");
    refresh();

    free(pool);
    free(queue);
}*/