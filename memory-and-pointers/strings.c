/*
    Strings

    - Passing strings to a function

    - sizeof; tells you how many bytes of space something takes up in memory
      (call with type or piece of data)

    - On some systems sizeof returns 4 or 8 bytes when giving the character.. Why?
      (on 32-bit systems sizeof returns 4 bytes and 64-bit returns 8 bytes for the sizeof a string)
    - Getting the sizeof a string gives the size of the pointer variable
*/

#include <stdio.h>


// char msg[] is a pointer variable to the string
void fortune_cookie(char msg[])
{
    printf("Message reads: %s\n", msg);
    printf("msg occupies %i bytes\n", sizeof(msg));
}


int main()
{
    // quote variable stores the address to the location of 
    // the first character in memory.
    // Array variable is just like a pointer.
    char quote[] = "Cookies make you fat";

    fortune_cookie(quote);

    // You can use it like a pointer variable.
    printf("The quote string is stored at: %p\n", quote);
}
