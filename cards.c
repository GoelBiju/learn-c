/*
    * Cards program example.
*/

#include <stdio.h>
#include <stdlib.h>

int main()
{
    char card_name[3];
    puts("Enter the card name: ");
    scanf("%2s", card_name);
    printf("Entered name %s", card_name);

    return 0;
}