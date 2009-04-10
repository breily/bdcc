int main() {
    
    printf("** if statement tests:\n");

    if (0 == 0) printf("\t- this should print\n");
    /*
    if (0 != 0) printf("\t- this should not print\n");
    */
    if (1 > 2) printf("\t- this should not print\n");
    if (1 < 2) printf("\t- this should print\n");

    printf("** end if statement tests\n\n");

    printf("** if/else statement tests:\n");

    /*
    if (1 == 1)
        printf("\t- this should print\n");
    else
        printf("\t- this should not print\n");
    */
    /*
    //if (1 <= 0) {
    if (1 < 0) {
        printf("\t- this should not print\n");
    } else {
        printf("\t- this should print\n");
    }
    */
    
    printf("** end if/else statement tests\n\n");

    return 0;
}
