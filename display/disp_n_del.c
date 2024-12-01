#include <stdio.h>
#include <unistd.h>
  int main (void)
  {
    int i = 3;
    printf("\nText to keep\n");
    printf("Text to erase****************************\n");
    printf("Text to erase****************************\n");
    printf("\033[A\33[2KT\r");
    printf("\033[A\33[2KT\r");
    // while(i > 0) { // 3 second countdown
    //     printf("\033[A\33[2KT\rT minus %d seconds...\n", i);
    //     i--;
    //     sleep(1);
    // }
    return 0;
 }