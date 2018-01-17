/*
 * APS106 Project - Chain Reaction
 *
 * April, 2012
 */


#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#ifdef __linux__
#include <unistd.h>
#endif

//Macros
#define SIZE 9              //Max dimension of the game board

#define CB 1
#define CR 2
#define RB 3
#define RR 4

#define UNCHECKED 0         //chose 0 because it is easiest to assign when initialize an array
#define CHECKED 1           //excluding current position
#define CURRENT 2           //current position




//Function declarations
void randomize_board(int board[][SIZE], int *mp);                                         //Generate a random initial board.
void import_board_file(int board[][SIZE], int *mp);                                       //Load board by file
void interactive_game(int board[][SIZE], int status[][SIZE], int m);                      //Interactive game
void log_interactive(int board[][SIZE], int path[][2], int m);                            //Write log for interactive game
void autoplay_initiation(int board[][SIZE], int status[][SIZE], int m);                          //Computer solve, write log file, call recursive function find_next_step to go through all solutions
void find_next_step(int current_position[], int board[][SIZE], int status[][SIZE], int m, int step_count, unsigned long long *valid_paths_count_p, unsigned long long *invalid_paths_count_p, int path[][2], FILE *fp);
void show_demo(int path[][2], int board[][SIZE], int m, int count);                                      //Show a step by step demo of a valid path
void print_board(int board[][SIZE], int status[][SIZE], int m);                           //Print the game board. Show the shape and color of each element, and which element has been visited.
int validate_move(int chosen_position[], int current_position[], int board[][SIZE], int status[][SIZE], int m);  //Return 0 if the move user has requested is invalid, 1 if valid.


void my_sleep(int seconds)
{
#ifdef __linux__
  sleep(seconds);
#elif _WIN32
  Sleep(seconds * 1000);
#else
  static_assert(false, "Bad OS Macro");
#endif
}

void my_clear_screen()
{
#ifdef __linux__
  system("clear");
#elif _WIN32
  system("cls");
#else
  static_assert(false, "Bad OS Macro");
#endif
}


int main(void) {

    //Variable declaration
    int m;                              //m is to store the real dimension of the board (not of the arrays)
    int status[SIZE][SIZE] = {CURRENT};       //Assign the top left corner to CURRENT and all the other elements of the status array to UNCHECKED
    int board[SIZE][SIZE];              //Register the shape and color of each element

    int random_or_file;
    int interactive_or_computer;


    //Welcome screen
    printf("\n************************ WELCOME TO CHAIN REACTION! ************************\n\n\n");


    //User chooses whether initialize the game randomly or by file
    printf("DO YOU WANT TO?:\n");
    printf("  1. START WITH A RANDOM INITIAL BOARD\n");
    printf("  2. IMPORT AN INITIAL BOARD FROM A FILE\n");
    do {
        printf("PLEASE ENTER YOUR CHOICE (1 OR 2): ");
        fflush(stdin);
        scanf("%d", &random_or_file);
    } while (!(random_or_file == 1 || random_or_file == 2));
    printf("\n");


    //If randomly initialze the board
    if (random_or_file == 1)
       randomize_board(board, &m);


    //If initialize the board array by file
    else
        import_board_file(board, &m);


    //Ask interactive or computer play
    printf("\n\nDO YOU WANT TO?:\n");
    printf("  1. START A NEW GAME!\n");
    printf("  2. WATCH A DEMO\n");
    do {
       printf("PLEASE ENTER YOUR CHOICE (1 OR 2): ");
       fflush(stdin);
       scanf("%d", &interactive_or_computer);
    } while (!(interactive_or_computer == 1 || interactive_or_computer == 2));


    //If interactive play
    if (interactive_or_computer == 1)
       interactive_game(board, status, m);


    //If computer play
    else {
       autoplay_initiation(board, status, m);

    }

    fflush(stdin);
    getchar();
    return 0;
}









void randomize_board(int board[][SIZE], int *mp) {
     int i, j;

     //User enters board dimension
     printf("\n");
     do {
        printf("PLEASE ENTER THE BOARD DIMENSION (3 TO 9 INCLUSIVE): ");
        fflush(stdin);
        scanf("%d", mp);
     } while (*mp > SIZE || *mp < 3);

     //Generate m*m random numbers (1~4) in the array board[][]
     srand(time(NULL));
     for (i = 0; i <= *mp; i++) {
         for (j = 0; j <= *mp; j++) {
             board[i][j] = rand() % 4 + 1;
         }
     }
}









void import_board_file(int board[][SIZE], int *mp) {
     FILE *fp;
     char filename[512];
     char ch;
     int flag;            //Used to check the format of the file
     int i, j;

     do {

        flag = 0;         //Flag turns to 1 if the file format cannot be recognized

        //Select file and check if exists
        do {
           printf("\nPLEASE ENTER THE FILENAME: ");
           fflush(stdin);
           scanf("%s", filename);
           fp = fopen(filename, "r");
           if (fp == NULL)
              printf("SORRY, THIS FILE CANNOT BE OPENED. PLEASE TRY AGAIN.\n\n");
        } while (fp == NULL);

        //Store the dimension in m
        fscanf(fp, "%d", mp);

        //Feed data into the board array
        for (i = 0; i < *mp; i++) {
            for (j = 0; j < *mp; j++) {

                fscanf(fp, " %c", &ch);

                if (ch == 'C') {
                   fscanf(fp, " %c", &ch);

                   if (ch == 'b')
                      board[i][j] = CB;
                   else if (ch == 'r')
                      board[i][j] = CR;
                   else
                      flag = 1;
                }

                else if (ch == 'R') {
                   fscanf(fp, " %c", &ch);

                   if (ch == 'b')
                      board[i][j] = RB;
                   else if (ch == 'r')
                      board[i][j] = RR;
                   else
                      flag = 1;
                }

                else
                   flag = 1;
            }
         }

         fclose(fp);

         //Warning message if file not readable
         if (flag == 1)
            printf("SORRY, THE FORMAT OF THIS FILE CANNOT BE RECOGNIZED. \n\n");

     } while (flag == 1);            //Re-ask file name if file not readable
}









void interactive_game(int board[][SIZE], int status[][SIZE], int m){
     int path[SIZE * SIZE][2] = {1, 1};  //[row_number][0]  [column_number][1]:column   number counts from 1 (not 0)
     int current_position[2] = {1, 1};            //[row#][column#]  row and column number counts from 1 (not 0)
     int chosen_position[2];                      //[row#][column#]  row and column number counts from 1 (not 0)
     int number_of_moves = 0;
     int invalid = 0;
     int user_entry, i;

     for (;;) {        //Outer infinite loop: repeatedly ask for the next move until winning or manual exit


         do{        //Inner loop: repeat without changing the board, status and path, while the move is invalid

           //Print the board, # of moves and current position
           print_board(board, status, m);
           printf("NUMBER OF MOVES: %d\t", number_of_moves);
           printf("CURRENT POSITION: %d%d\n", current_position[0], current_position[1]);

           //Print the path
           printf("PATH: ");
           for (i = 0; i <= number_of_moves; i++) {
               printf("%d%d ", path[i][0], path[i][1]);
           }
           printf("\n\n");

           //Show message and exit if user has won
           if (number_of_moves == (m * m - 1)) {
              printf("\n********************** CONGRATULATIONS! YOU HAVE WON! **********************\n\n\n");
              log_interactive(board, path, m);
              printf("\nPRESS ENTER TO EXIT...");
              return;                          //Program terminates
           }

           //Show warning if last attempt was invalid
           if (invalid) printf("YOUR LAST MOVE WAS INVALID. PLEASE TRY AGAIN.\n");

           //Read user's entry. Exit if 0 is entered first.
           printf("CHOOSE A POSITION (0 TO EXIT): ");
           fflush(stdin);
           scanf("%d", &user_entry);
           if (user_entry == 0) {
              printf("\n\n********************************* GOODBYE! *********************************\n");
              printf("\n\nPRESS ENTER TO EXIT...");
              return;                          //Program terminates
           }

           chosen_position[0] = user_entry / 10;
           chosen_position[1] = user_entry % 10;


           invalid = !validate_move(chosen_position, current_position, board, status, m);
         } while(invalid);                           //End of inner loop. Repeat if move not valid.



         //Proceed if the move is valid
         status[current_position[0] - 1][current_position[1] - 1] = CHECKED;       //Update the status array
         status[chosen_position[0] - 1][chosen_position[1] - 1] = CURRENT;

         current_position[0] = chosen_position[0];                                 //Update the current position
         current_position[1] = chosen_position[1];

         path[number_of_moves + 1][0] = chosen_position[0];                        //Save the path
         path[number_of_moves + 1][1] = chosen_position[1];

         number_of_moves++;

     }          //End of outer infinite loop

}









void log_interactive(int board[][SIZE], int path[][2], int m) {
    int i, j;
    FILE *fp;
    fp = fopen("log.txt", "w");         //Create file
    if (fp == NULL) {                   //Warning if cannot create
       printf("SORRY, THE LOG FILE CANNOT BE CREATED.\n\n");
       return;
    }

    //Write team info

    //Print the initial board
    fprintf(fp, "%d\n", m);

    for (i = 0; i < m; i++) {
        for (j = 0; j < m; j++) {
            //Print the color and the shape
            if (board[i][j] == CB) fprintf(fp, "Cb\t");
            else if (board[i][j] == CR) fprintf(fp, "Cr\t");
            else if (board[i][j] == RB) fprintf(fp, "Rb\t");
            else fprintf(fp, "Rr\t");
        }
        fprintf(fp, "\n");
    }

    //Print the path and other info
    fprintf(fp, "START\nINTERACTIVE\n");
    for (i = 0; i <= (m * m - 1); i++) {
        fprintf(fp, "%d%d ", path[i][0], path[i][1]);
    }
    fprintf(fp, "\nEND");
    fclose(fp);

    printf("A LOG FILE HAS BEEN CREATED UNDER THE CURRENT PATH.\n");      //msg on screen



}








void autoplay_initiation(int board[][SIZE], int status[][SIZE], int m) {
     int initial_position[2] = {1, 1};
     int path[SIZE * SIZE][2] = {1, 1};
     unsigned long long valid_paths_count = 0;
     unsigned long long invalid_paths_count = 0;
     int i, j;
     FILE *fp;

     fp = fopen("log.txt", "w");            //Create new log file first
     if (fp == NULL) {
        printf("SORRY, THE LOG FILE CANNOT BE CREATED.\n\n");
        return;
     }

     //write team info



     //Print the initial board
     fprintf(fp, "%d\n", m);

     for (i = 0; i < m; i++) {
         for (j = 0; j < m; j++) {
             //Print the color and the shape
             if (board[i][j] == CB) fprintf(fp, "Cb\t");
             else if (board[i][j] == CR) fprintf(fp, "Cr\t");
             else if (board[i][j] == RB) fprintf(fp, "Rb\t");
             else fprintf(fp, "Rr\t");
         }
         fprintf(fp, "\n");

     }

     //Print all paths other info
     fprintf(fp, "START\nCOMPUTER\n");


     find_next_step(initial_position, board, status, m, 1, &valid_paths_count, &invalid_paths_count, path, fp);    //call recursive function to find all paths

     fprintf(fp, "%llu\n%llu\n", valid_paths_count, invalid_paths_count);
     fprintf(fp, "END");

     fclose(fp);

     if (valid_paths_count <= 0)                    //show msg if no valid solutions
        printf("NO VALID SOLUTIONS FOUND.\n");

     printf("\n\n****************************** DEMO COMPLETE! ******************************\n\n\n");   //exit msg on screen
     printf("A LOG FILE HAS BEEN CREATED UNDER THE CURRENT PATH.\n\n");
     printf("PRESS ENTER TO EXIT...");
}









void find_next_step(int current_position[], int board[][SIZE], int status[][SIZE], int m, int step_count, unsigned long long *valid_paths_count_p, unsigned long long *invalid_paths_count_p, int path[][2], FILE *fp) {

     int i;
     int chosen_position[2];
     int valid;
     int flag = 0;        //Remains to be 0 if no legal moves can be found



     //Find all legal moves to elements in the same row and column
     for (i = 0; i < m; i++) {

         int status_copy[SIZE][SIZE], path_copy[SIZE * SIZE][2], x, y;

         //Validate if moving to column i is legal
         chosen_position[0] = current_position[0];
         chosen_position[1] = i + 1;
         valid = validate_move(chosen_position, current_position, board, status, m);

         if (valid) {
            flag = 1;                           //To indicate that at least one valid move can be found

            //Register this move to path[][].
            path[step_count][0] = chosen_position[0];
            path[step_count][1] = chosen_position[1];



            //Make a copy of path[][]. The copy is to be passed to the next recursion.
            for (x = 0; x < SIZE * SIZE; x++) {
                for (y = 0; y < 2; y++) {
                    path_copy[x][y] = path[x][y];
                }
            }


            //Make a copy of status[][], and update in the copy. The copy is to be passed to the next recursion.
            for (x = 0; x < SIZE; x++) {
                for (y = 0; y < SIZE; y++) {
                    status_copy[x][y] = status[x][y];
                }
            }
            status_copy[chosen_position[0] - 1][chosen_position[1] - 1] = CURRENT;
            status_copy[current_position[0] - 1][current_position[1] - 1] = CHECKED;


            //Recurse
            find_next_step(chosen_position, board, status_copy, m, step_count + 1, valid_paths_count_p, invalid_paths_count_p, path_copy, fp);

         }




         //Validate if moving to column i is legal
         chosen_position[0] = i + 1;
         chosen_position[1] = current_position[1];
         valid = validate_move(chosen_position, current_position, board, status, m);

         if (valid) {
            flag = 1;                           //To indicate that at least one valid move can be found

            //Register this move to path[][].
            path[step_count][0] = chosen_position[0];
            path[step_count][1] = chosen_position[1];



            //Make a copy of path[][]. The copy is to be passed to the next recursion.
            for (x = 0; x < SIZE * SIZE; x++) {
                for (y = 0; y < 2; y++) {
                    path_copy[x][y] = path[x][y];
                }
            }

            //Make a copy of status[][], and update in the copy. The copy is to be passed to the next recursion.
            for (x = 0; x < SIZE; x++) {
                for (y = 0; y < SIZE; y++) {
                    status_copy[x][y] = status[x][y];
                }
            }
            status_copy[chosen_position[0] - 1][chosen_position[1] - 1] = CURRENT;
            status_copy[current_position[0] - 1][current_position[1] - 1] = CHECKED;


            //Recurse
            find_next_step(chosen_position, board, status_copy, m, step_count + 1, valid_paths_count_p, invalid_paths_count_p, path_copy, fp);
         }


     }



        //Check if has already won or come to a dead end.
        if (step_count >= m * m) {
           for (i = 0; i <= step_count - 1; i++)
               fprintf(fp, "%d%d ", path[i][0], path[i][1]);          //Print valid path to file
           fprintf(fp, "\n");
           (*valid_paths_count_p)++;                                  //update the counter
           show_demo(path, board, m, *valid_paths_count_p);           //show animation walking through the steps
        }


        else if (flag == 0){
           (*invalid_paths_count_p)++;                                //update the counter
        }


}










void show_demo(int path[][2], int board[][SIZE], int m, int valid_paths_count) {
     int status[SIZE][SIZE] = {CURRENT};
     int step, i;

     for (step = 0; step < m * m; step++) {
         //print board and other info
         print_board(board, status, m);
         printf("CURRENT SOLUTION #: %d\n", valid_paths_count);

         printf("PATH: ");
         for (i = 0; i <= step; i++) {
             printf("%d%d ", path[i][0], path[i][1]);
         }
         printf("\n");

         my_sleep(3);   //pause the program for seconds

         //update the status[][] array
         status[path[step][0] - 1][path[step][1] - 1] = CHECKED;
         status[path[step + 1][0] - 1][path[step + 1][1] - 1] = CURRENT;

     }


}









int validate_move(int chosen_position[], int current_position[], int board[][SIZE], int status[][SIZE], int m) {

    int current_color_shape = board[current_position[0] - 1][current_position[1] - 1];
    int chosen_color_shape = board[chosen_position[0] - 1][chosen_position[1] - 1];

    //Check if out of board range
    if (chosen_position[0] > m || chosen_position[1] > m || chosen_position[0] < 1 || chosen_position[1] < 1)
       return 0;

    //Check if same row or column
    else if (!(chosen_position[0] == current_position[0] || chosen_position[1] == current_position[1]))
       return 0;

    //Check if chosen position is unchecked
    else if (status[chosen_position[0] - 1][chosen_position[1] - 1] != UNCHECKED)
       return 0;


    //Check if same color or shape
    else if (current_color_shape == RR && chosen_color_shape == CB)
       return 0;
    else if (current_color_shape == CB && chosen_color_shape == RR)
       return 0;
    else if (current_color_shape == CR && chosen_color_shape == RB)
       return 0;
    else if (current_color_shape == RB && chosen_color_shape == CR)
       return 0;


    //If proceed to here then the move is valid
    else
       return 1;
}









void print_board(int board[][SIZE], int status[][SIZE], int m) {
     int i, j;
     my_clear_screen();

     //Print the upper heading
     printf("\n************************ WELCOME TO CHAIN REACTION! ************************\n\n\n");
     printf("            ");
     for (i = 1; i <= m; i++) {
         printf("%-6d", i);
     }
     printf("\n\n\n");

     //Print the rest of the board
     for (i = 0; i < m; i++) {
         printf("      %-6d", i + 1);                  //Print the left heading

         for (j = 0; j < m; j++) {
             //Print the color and the shape
             if (board[i][j] == CB) printf("Cb");
             else if (board[i][j] == CR) printf("Cr");
             else if (board[i][j] == RB) printf("Rb");
             else printf("Rr");

             //Show the current and already visited positions
             if (status[i][j] == CURRENT) printf("*");
             else if (status[i][j] == CHECKED) printf("X");
             else printf(" ");

             printf("   ");
         }
         printf("\n\n\n");
     }
}
