/* 
 * phase2a.c - To defeat this stage the user must enter a sequence of 
 * 6 nonnegative numbers where x[i] = x[i-1] + i
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

// void read_six_numbers(char *input, int *numbers)
// {
//   int numScanned = sscanf(input, "%d %d %d %d %d %d",
// 			  numbers, numbers + 1, numbers + 2,
// 			  numbers + 3, numbers + 4, numbers + 5);
//     printf("numScanned = %d\n", numScanned);
//   if (numScanned < 6)
//     explode_bomb();
// }

void phase_2()
{
    int i;
    int numbers[6];
    int nums[6] = {0, 0, 0, 0, 0, 0};
    char str[] = "AACCCBBBAA";
    volatile static int mul = 0;
    
    scanf("%d %d %d %d %d %d", numbers, numbers + 1, numbers + 2, numbers + 3, numbers + 4, numbers + 5);
    if (numbers[0] < 0)
	explode_bomb();
    for(int i =0; i< string_length(str); i++){
        nums[str[i] - 'A']++;
    }
    for(i = 0; i < 6; i++) {
	if (numbers[i] != nums[i])
	    explode_bomb();
    }
    printf("Phase 2 is defused. How about the next one?\n");
}
int main(){
    phase_2();
    return 0;
}
