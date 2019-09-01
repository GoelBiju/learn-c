/*
    Using memory pointers:

    1. Get the address of a variable:
        - Use the & operator to get address:
            
            e.g. int x = 4;
                 printf("x lives at %p\n", &x);

        - Store address in a pointer variable:
            
            e.g. int* address_of_x = &x;

    2. Read contents of an address:
        - When you have memory address, read the data stored there.
          Use the * operator:
            
            e.g. int value_stored = *address_of_x; 

        - * and & are opposites.

        - & operator; takes piece of data and tells you where it's stored.

        - * operator; takes address and tells you what's stored there.

        - Pointers called references, * operator dereferences a pointer.

    3. Change contents of address:
        - Have pointer variables and change address where variable is pointing,
          use the * operator, BUT use it on LEFT SIDE of an assignment:
            
            e.g. *address_of_x = 99;

*/

#include <stdio.h>


/*
    The latitude will go down and the longitude increase 
    when moving south-east.

    We pass in the pointers of latitude and longitude.
*/
void go_south_east(int* lat, int* lon)
{
    // Use the pointer variable and change the contents 
    // of both the latitude and longitude variables.
    
    // Decrease the latitude.
    *lat = *lat - 1;

    // Increase the longitude.
    *lon = *lon + 1;
}


int main()
{
    int latitude = 32;
    int longitude = -64;

    // Pass in the addresses of latitude and longitude variables.
    go_south_east(&latitude, &longitude);

    printf("Avast! Now at: [%i, %i]\n", latitude, longitude);

    return 0;
}
