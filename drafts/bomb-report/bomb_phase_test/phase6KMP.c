/* 
 * phase6a.c - The user has to enter the node numbers (from 1 to 6) in 
 * the order that they will occur when the list is sorted in ascending
 * order.
 */
#include <stdio.h>
#include <stdlib.h>

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
    exit(8);
}


void phase_6(char *input)
{
    int len = 7;
    char Match [] = "6Random_String";
    int leni = string_length(input);
    int M = string_length(Match);
    if(leni != len)
        explode_bomb();
    int next[leni];
    next[0] = 0;
    int i = 1;
    len = 0;
    while (i < leni) {
        if (input[i] == input[len]) {
            len++;
            next[i] = len;
            i++;
        } else {
            if (len != 0) {
                len = next[len - 1];
            } else {
                next[i] = 0;
                i++;
            }
        }
    }
    int j = 0;
    i = 2;
    while (i < M) {
        if (input[j] == Match[i]) {
            j++;
            i++;
        }

        if (j == leni) {
            printf("phase_6 defused\n");
            return ;
        } else if (i < M && input[j] != Match[i]) {
            if (j != 0)
                j = next[j - 1];
            else
                i = i + 1;
        }
    }
    explode_bomb();

}
int main(){
    char input[20];
    scanf("%s", input);
    phase_6(input);
}