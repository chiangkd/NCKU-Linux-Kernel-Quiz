#include <stdio.h>
#include <stdint.h>

int ceil_log2(uint32_t x)
{
    uint32_t r=0, shift=0;
    x--;
    r = (x > 0xFFFF) << 4;                 
    x >>= r;
    shift = (x > 0xFF) << 3;
    x >>= shift;
    r |= shift;
    shift = (x > 0xF) << 2;
    x >>= shift;
    r |= shift;
    shift = (x > 0x3) << 1;
    x >>= shift;
    return (r | shift | x>>1) + 1;       
}

int main()
{

    printf("num = %d\n", ceil_log2(33));
    return 0;
}