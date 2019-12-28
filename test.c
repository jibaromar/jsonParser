#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct test{
    int a;
};
void main(){
    struct test *cc;
    cc = (struct test*) malloc(sizeof(struct test));
    cc->a=5;
    printf("%d\n", cc->a);

}