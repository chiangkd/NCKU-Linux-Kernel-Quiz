#include <stdio.h>
int concatenatedBinary(int n){
    const int M = 1e9 + 7;
    int len = 0;    /* bit length to be shift */
    long ans = 0;
    for(int i = 1; i <= n; i++) {
        /* removing the rightmost set bit
         * e.g. 100100 -> 100000
         *      000001 -> 000000
         *      000000 -> 000000
         * after removal, if it is 0, then it means it is power of 2
         * as all power of 2 only contains 1 set bit
         * if it is power of 2, we increase the bit length
         */
        if(!(i & (i - 1)))
            len++;
        ans = (i | (ans << len)) % M;
    }
    return ans;
}

int main()
{
    printf("ans = %d\n", concatenatedBinary(4));
    return 0;
}