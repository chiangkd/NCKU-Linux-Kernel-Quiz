// Source code from https://alienryderflex.com/quicksort/


//  quickSort
//
//  This public-domain C implementation by Darel Rex Finley.
//
//  * Returns YES if sort was successful, or NO if the nested
//    pivots went too deep, in which case your array will have
//    been re-ordered, but probably not sorted correctly.
//
//  * This function assumes it is called with valid parameters.
//
//  * Example calls:
//    quickSort(&myArray[0],5); // sorts elements 0, 1, 2, 3, and 4
//    quickSort(&myArray[3],5); // sorts elements 3, 4, 5, 6, and 7

#include <stdbool.h>
#include <stdio.h>
bool quickSort(int *arr, int elements) {

  #define  MAX_LEVELS  1000

  int  piv, beg[MAX_LEVELS], end[MAX_LEVELS], i=0, L, R ;

  beg[0]=0; end[0]=elements;
  while (i>=0) {
    L=beg[i]; R=end[i]-1;
    if (L<R) {
      piv=arr[L]; if (i==MAX_LEVELS-1) return false;
      while (L<R) {
        while (arr[R]>=piv && L<R) R--; if (L<R) arr[L++]=arr[R];
        while (arr[L]<=piv && L<R) L++; if (L<R) arr[R--]=arr[L]; }
      arr[L]=piv; beg[i+1]=L+1; end[i+1]=end[i]; end[i++]=L; }
    else {
      i--; }}
  return true; 
}

void display(int* arr, int len)
{
    for(int i = 0; i < len; i++){
        printf("%d\t", arr[i]);
    }
    printf("\n");
}

int main(){
    
    int arr[8] = {4,5,3,6,2,8,1,7};

    printf("origin:\n");
    display(arr, 8);
    bool result = quickSort(arr, 8);

    if(result){
        printf("sorted:\n");
        display(arr, 8);
    }else
        printf("Something wrong\n");

    return 0;
}