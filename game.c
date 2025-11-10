#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     // for sync(), unlink()
#include <ncurses.h>    // for mvprintw, wgetch, etc.
#include "lab5.h"


extern Node *g_root;
extern EditStack g_undo;
extern EditStack g_redo;
extern Hash g_index;

/* TODO 31: Implement play_game
 * Main game loop using iterative traversal with a stack
 * 
 * Key requirements:
 * - Use FrameStack (NO recursion!)
 * - Push frames for each decision point
 * - Track parent and answer for learning
 * 
 * Steps:
 * 1. Initialize and display game UI
 * 2. Initialize FrameStack
 * 3. Push root frame with answeredYes = -1
 * 4. Set parent = NULL, parentAnswer = -1
 * 5. While stack not empty:
 *    a. Pop current frame
 *    b. If current node is a question:
 *       - Display question and get user's answer (y/n)
 *       - Set parent = current node
 *       - Set parentAnswer = answer
 *       - Push appropriate child (yes or no) onto stack
 *    c. If current node is a leaf (animal):
 *       - Ask "Is it a [animal]?"
 *       - If correct: celebrate and break
 *       - If wrong: LEARNING PHASE
 *         i. Get correct animal name from user
 *         ii. Get distinguishing question
 *         iii. Get answer for new animal (y/n for the question)
 *         iv. Create new question node and new animal node
 *         v. Link them: if newAnswer is yes, newQuestion->yes = newAnimal
 *         vi. Update parent pointer (or g_root if parent is NULL)
 *         vii. Create Edit record and push to g_undo
 *         viii. Clear g_redo stack
 *         ix. Update g_index with canonicalized question
 * 6. Free stack
 */
void play_game() {
    clear();
    attron(COLOR_PAIR(5) | A_BOLD);
    mvprintw(0, 0, "%-80s", " Playing 20 Questions");
    attroff(COLOR_PAIR(5) | A_BOLD);
    
    mvprintw(2, 2, "Think of an animal, and I'll try to guess it!");
    mvprintw(3, 2, "Press any key to start...");
    refresh();
    getch();
    
    // TODO: Implement the game loop
    // Initialize FrameStack
    // Push root
    // Loop until stack empty or guess is correct
    // Handle question nodes and leaf nodes differently
    
    FrameStack stack;//stack for iterative traversal
    fs_init(&stack);// initialize stack
    // push root frame and start iterative traversal
    fs_push(&stack, g_root, -1);
    Node *parent = NULL;
    int parentAnswer = -1; //-1=root, 1=yes, 0=no
    int guessed = 0;
    char status_msg[256]=""; //buffer for status messages because it breaks in the termianl for some reason
    (void)status_msg; //suppress unused variable warning for now
    while(!fs_empty(&stack) && !guessed){
        Frame f = fs_pop(&stack);
        Node *cur = f.node;
        if(cur == NULL) continue;

        //Question node: ask the question and push the chosen child
        //CASE 1 when current node is a question
        if(cur -> isQuestion){
            // Clear the previous question line
            mvprintw(4, 3, "%-70s", ""); // Clear the line with spaces
            refresh();
            
            char prompt[256];//buffer for prompt
            snprintf(prompt, sizeof(prompt), "%s (y/n): ", cur->text);//prepare prompt
            int ans = get_yes_no(4, 3, prompt);//get user answer

            parent = cur;//update parent pointer
            parentAnswer = ans ? 1 : 0;//update parent answer

            if(ans){
                fs_push(&stack, cur->yes, 1);//push yes child
            } else {
                fs_push(&stack, cur->no, 0);//push no child
            }
        }
        //CASE 2: the current node is a leaf (animal)
        else{
            //leaf node: make a guess
            // Clear the previous line first
            mvprintw(4, 3, "%-70s", ""); // Clear the line with spaces
            refresh();
            
            char prompt[256];
            snprintf(prompt, sizeof(prompt), "Is it a %s? (y/n): ", cur->text);
            int correct = get_yes_no(4, 3, prompt);

            if(correct){
                show_message("Yay! I guessed it right!", 0);
                guessed = 1;
                break;
            }

            // Get new animal and distinguishing question this is the learning phase
            mvprintw(6, 3, "%-70s", "");  // Clear any previous input
            mvprintw(7, 3, "%-70s", "");
            mvprintw(8, 3, "%-70s", "");
            refresh();

            // Get input and make copies since get_input uses a static buffer
            char *newAnimal = strdup(get_input(6, 3, "I give up! What animal were you thinking of? "));
            char *newQuestion = strdup(get_input(7, 3, "Please provide a question that distinguishes your animal: "));
            //I wanted to use the below lines but it was causing issues in the terminal so I used strdup above
            //char *newAnimal = get_input(6, 3, "I give up! What animal were you thinking of? ");
            //char *newQuestion = get_input(7, 3, "Please provide a question that distinguishes your animal: ");
            int newAnswer = get_yes_no(8, 3, "For your animal, what is the answer to that question? (y/n): ");
            
            // Create nodes with the actual question and animal
            Node *newAnimalNode = create_animal_node(newAnimal);
            Node *newQuestionNode = create_question_node(newQuestion);

            // Free our copies since create_*_node makes its own copies
            free(newAnimal);
            free(newQuestion);

            // Debug output
            mvprintw(10, 3, "Adding new animal: %s", newAnimalNode->text);
            mvprintw(11, 3, "With question: %s", newQuestionNode->text);
            mvprintw(12, 3, "Answer for new animal is: %s", newAnswer ? "yes" : "no");
            refresh();
            napms(1000);
            
            // Link nodes based on the answer for the new animal (e.g., does the cat meow?)
            if(newAnswer) {
                // If new animal answers YES (e.g., cat DOES meow)
                newQuestionNode->yes = newAnimalNode;  // New animal (cat) goes to YES
                newQuestionNode->no = cur;             // Old animal (dog) goes to NO
                mvprintw(13, 3, "Setting up: For '%s': YES->%s, NO->%s", 
                        newQuestionNode->text, newAnimalNode->text, cur->text);
            } else {
                // If new animal answers NO
                newQuestionNode->yes = cur;            // Old animal goes to YES
                newQuestionNode->no = newAnimalNode;   // New animal goes to NO
                mvprintw(13, 3, "Setting up: For '%s': YES->%s, NO->%s", 
                        newQuestionNode->text, cur->text, newAnimalNode->text);
            }
            
            // Make sure we're linking this into the tree correctly
            if(parent == NULL) {
                g_root = newQuestionNode;  // This becomes the new root
            } else if(parentAnswer == 1) {
                parent->yes = newQuestionNode;  // Replace the YES branch
            } else {
                parent->no = newQuestionNode;   // Replace the NO branch
            }
            refresh();
            napms(1000);

            // Clear these debug messages after a moment
            refresh();
            napms(2000);
            for(int r=10; r<=13; r++) {
                mvprintw(r, 0, "%-*s", COLS, ""); // Clear the line with spaces
            }
            refresh();
            refresh();
            napms(2000);  // Show debug info for 2 seconds
            //Update parent pointer or g_root
            Edit edit;
            edit.parent = parent;
            edit.wasYesChild = (parent != NULL && parentAnswer == 1) ? 1 : 0;
            edit.oldLeaf = cur;
            edit.newQuestion = newQuestionNode;

            // Record this change for undo/redo
            edit.parent = parent;
            edit.wasYesChild = parentAnswer == 1;
            edit.oldLeaf = cur;
            edit.newQuestion = newQuestionNode;

            es_push(&g_undo, edit);
            es_clear(&g_redo);

            //Update index
            char *key = canonicalize(newQuestion);//canonicalize question for indexing
            if(key){
                h_put(&g_index, key, count_nodes(g_root) - 1); //assuming new animal is the last node added
                free(key);
            }

            // Save the updated tree to file and make sure it's written to disk
            FILE *fp = fopen("animals.dat.tmp", "wb");//open temp file
            if(fp != NULL) {
                fclose(fp);
                if(save_tree("animals.dat.tmp") && rename("animals.dat.tmp", "animals.dat") == 0) {
                    show_message("Thanks! I've learned a new animal and saved it!", 0);
                    sync();
                } else {
                    show_message("Warning: Failed to save tree changes!", 1);
                    unlink("animals.dat.tmp");  // Clean up temporary file
                }
            } else {
                show_message("Warning: Failed to save tree changes!", 1);
            }
            guessed = 1;
        }
    }
    fs_free(&stack);
}

/* TODO 32: Implement undo_last_edit
 * Undo the most recent tree modification
 * 
 * Steps:
 * 1. Check if g_undo stack is empty, return 0 if so
 * 2. Pop edit from g_undo
 * 3. Restore the tree structure:
 *    - If edit.parent is NULL:
 *      - Set g_root = edit.oldLeaf
 *    - Else if edit.wasYesChild:
 *      - Set edit.parent->yes = edit.oldLeaf
 *    - Else:
 *      - Set edit.parent->no = edit.oldLeaf
 * 4. Push edit to g_redo stack
 * 5. Return 1
 * 
 * Note: We don't free newQuestion/newLeaf because they might be redone
 */
//pops the most recent Edit record from g_undo and reverts the tree to its previous state
//the popped Edit is then pushed onto g_redo for potential redo
int undo_last_edit() {
    if(es_empty(&g_undo)) {
        return 0; // Nothing to undo
    }
    Edit edit = es_pop(&g_undo); // Pop the last edit
    // Restore the tree structure
    if(edit.parent == NULL) {
        g_root = edit.oldLeaf; //restore root
    } else if(edit.wasYesChild) {
        edit.parent->yes = edit.oldLeaf; //restore yes child
    } else {
        edit.parent->no = edit.oldLeaf; //restore no child
    }
    es_push(&g_redo, edit); // Push edit to redo stack
    return 1;
    // TODO: Implement this function
    return 0;
}

/* TODO 33: Implement redo_last_edit
 * Redo a previously undone edit
 * 
 * Steps:
 * 1. Check if g_redo stack is empty, return 0 if so
 * 2. Pop edit from g_redo
 * 3. Reapply the tree modification:
 *    - If edit.parent is NULL:
 *      - Set g_root = edit.newQuestion
 *    - Else if edit.wasYesChild:
 *      - Set edit.parent->yes = edit.newQuestion
 *    - Else:
 *      - Set edit.parent->no = edit.newQuestion
 * 4. Push edit back to g_undo stack
 * 5. Return 1
 */
//pops an Edit from g_redo and reapplies the structural change to the tree
//the re-applied Edit is then pushed back onto g_undo for potential undo
int redo_last_edit() {
    if(es_empty(&g_redo)) {
        return 0; // Nothing to redo
    }
    Edit edit = es_pop(&g_redo); // Pop the last redo edit
    // Reapply the tree modification
    if(edit.parent == NULL) {
        g_root = edit.newQuestion; //restore root
    } else if(edit.wasYesChild) {
        edit.parent->yes = edit.newQuestion; //restore yes child
    } else {
        edit.parent->no = edit.newQuestion; //restore no child
    }
    es_push(&g_undo, edit); // Push edit back to undo stack
    return 1;
    // TODO: Implement this function
    return 0;
}
