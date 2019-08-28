/*
    - Pointer is address of piece of data in memory:
        -> Instead of passing around a copy of the data, 
           you can just pass a pointer.
        
        -> You might want two pieces of code to work on the same
           piece of data rather than a separate copy.

    - So, pointers help avoid copies and share data.

    - Declaring a variable inside a function e.g. main(), 
      is usually stored in the stack section of memory.

    - Declaring a variable outside a function stores the variable 
      in the globals section of memory.

    - Use the & operator to get the address of a variable in memory.
      An address is also called a pointer as it points to the location 
      of the variable in memory.
*/

#include <stdio.h>

int y = 1;


int main(){
    int x = 4;

    printf("x is stored at %p\n", &x);

    return 0;
}
