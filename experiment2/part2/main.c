#include "function.h"

int main(){
    int num1,num2;
    int res1,res2;
    printf("pls input two numbers to get two results:");
    scanf("%d %d", &num1,&num2);
    res1 = caculate1(num1,num2);
    res2 = caculate2(num1,num2);
    printf("result1:%d\n", res1);
    printf("result2:%d\n", res2);
    return 0;
}