/* 
 * phase7.c - The infamous secret stage! 
 * The user has to find leaf value given path in a binary tree.
 */

// typedef struct treeNodeStruct
// {
//     int value;
//     struct treeNodeStruct *left, *right;
// } treeNode;
#include <stdio.h>
#include <stdlib.h>

#define SIZE 8
#define NUM_TARGETS 4
int string_length(char *aString)
{
    int length;
    char *ptr;

    ptr = aString;
    length = 0;

    while (*ptr != 0) {
	ptr++;
	length = length + 1;
    }
    return length;
}

void explode_bomb(void)
{
    printf("\nBy my efflux of deep crimson, topple this white world! EXPLOSION!!!\n");
    puts("     _.-^^---....,,--       ");
    puts(" _--                  --_   ");
    puts("<                        >) ");
    puts("|                         | ");
    puts(" \._                   _./  ");
    puts("    ```--. . , ; .--'''     ");  
    puts("          | |   |           ");  
    puts("       .-=||  | |=-.        ");
    puts("       `-=#$%&%$#=-'        ");
    puts("          | ;  :|           ");
    puts(" _____.,-#%&$@%#&#~,._____  ");
    printf("\nThe bomb has blown up.\n");
    printf("the answer of this case is 002000022122111311002023003310220202");
    exit(8);
}
// r 0 l 1 d 2 u 3
char maze[SIZE][SIZE] = {
    {0, 0, 0, 1, 1, 1, 1, 1},
    {1, 1, 0, 0, 0, 2, 0, 1},
    {1, 0, 0, 1, 1, 1, 0, 1},
    {1, 1, 0, 1, 5, 0, 0, 1},
    {3, 0, 0, 1, 1, 0, 1, 1},
    {1, 1, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 4, 1, 1, 0, 0},
    {1, 1, 1, 1, 1, 1, 1, 0}
};

/* 
 * Searches for a node in a binary tree and returns path value.
 * 0 bit denotes left branch, 1 bit denotes right branch
 * Example: the path to leaf value "35" is left, then right,
 * then right, and thus the path value is 110(base 2) = 6.
 */

// int fun7(treeNode* node, int val)
// {
//     if (node == NULL) 
// 	return -1;
  
//     if (val < node->value) 
// 	return fun7(node->left, val) << 1;
//     else if (val == node->value) 
// 	return 0;
//     else 
// 	return (fun7(node->right, val) << 1) + 1;
// }

int fun7(int target[], int place, int next, char * path){
    int dx[] = {0, 0, 1, -1};
    int dy[] = {1, -1, 0, 0};
    int x = place / SIZE;
    int y = place % SIZE; 
    if (x == y && x == SIZE - 1){
        return ( next == NUM_TARGETS && *path == '\0');
    }
    x = x + dx[(* path - '0') & 0x3];
    y = y + dy[(* path - '0') & 0x3];
    if(x < 0 || x >= SIZE || y < 0 || y >= SIZE || maze[x][y] == 1){
        return 0;
    }
    if (maze[x][y] ==0 || maze[x][y] == target[next]){
        return fun7(target, x * SIZE + y, next + 1, path + 1);
    }
}
     
void secret_phase()
{
    char input[100];
    int target[4] = {2, 3, 5, 4};
    int path;
    scanf("%s", input);
    /* Make sure target is in the right range */
    if(string_length(input) > 70)
	explode_bomb();

    /* Determine the path to the given target */
    path = fun7(target, 0, 0, input);

    /* Compare the retrieved path to a random path */
    if (!path)
	explode_bomb();
  
    printf("Wow! You've defused the secret stage!\n");

}
int main(){
    secret_phase();
    return 0;
}


