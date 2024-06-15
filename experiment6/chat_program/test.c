#include<stdio.h>

typedef struct{
    int textid;
    char text[20];
}mytext;

void input(mytext* mt){
    printf("please enter the text:");
    scanf("%[^\n]",mt->text);
    printf("text:%s\n",mt->text);
}

int main(){
    mytext mt;
    input(&mt);
    return 0;
}