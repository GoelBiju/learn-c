
#include <stdio.h>


/*
    The latitude will go down and the longitude increase 
    when moving south-east.
*/
void go_south_east(int lat, int lon)
{
    // Decrease the latitude.
    lat = lat - 1;

    // Increase the longitude.
    lon = lon + 1;
}


int main()
{
    int latitude = 32;
    int longitude = -64;

    go_south_east(latitude, longitude);

    printf("Avast! Now at: [%i, %i]\n", latitude, longitude);

    return 0;
}
