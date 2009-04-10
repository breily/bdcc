int main() {
    int i;

    printf("** start for tests:\n");

    printf("\t* should count to 10: ");
    for (i = 0; i < 11; i = i + 1)
        printf("%d ", i);
    printf("\n");
    
    printf("\t* should count backwards to 0: ");
    for (i = 10; i >= 0; i = i - 1)
        printf("%d ", i);
    printf("\n");
    
    printf("\t* without an action: ");
    for (i = 0; i < 10;) {
        printf(".");
        i = i + 1;
    }
    printf("works\n");

    printf("\t* without a declaration: ");
    i = 0;
    for (; i < 11; i = i + 1) printf(".");
    printf("works\n");

    printf("** end for tests\n");
}
