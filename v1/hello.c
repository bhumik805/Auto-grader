#include <stdio.h>
int main(){
    int a[10] = {1,3,5,2,4,6,7,8,9,10};
    int n=sizeof(a)/sizeof(int),temp;
    for (int i = 0; i < n; i++)
    {
        for (int j = i; j < n; j++)
        {
            if(a[i]>a[j]){
                temp = a[i];
                a[i] = a[j];
                a[j] = temp;
            }
        }
    }
    for (int i = 0; i < n; i++)
    {
        printf("%d ",a[i]);
    }
    printf("\n");
    return 0;
    
}
// #include <stdio.h>

// // Define a function type
// typedef void (*FunctionPointer)();

// // Example function to be executed at runtime
// void myFunction() {
//     printf("Hello from myFunction!\n");
// }

// int main() {
//     // Declare a function pointer
//     FunctionPointer ptr;

//     // Assign the address of the function to the pointer
//     ptr = myFunction;

//     // Attempt to call the function using the pointer
//     // However, we'll set the pointer to NULL to generate a runtime error
//     ptr = NULL;
//     ptr();  // This will generate a segmentation fault or similar error

//     return 0;
// }
