int main() {
    int a, b, i;
    a = 4;
    b = 5;
    if (a > b) printf("if statement: a=%d,b=%d\n", a, b);
    else printf("else statement: a=%d,b=%d\n", a, b);

    i = 0;
    while (i < 10) {
        /*
        if (i == 0) {
            printf("i is 0\n");
        } else if ((i % 2) == 1) {
            printf("i=%d is odd\n", i);
        } else {
            printf("i=%d is even\n", i);
        }
        */
        if ((i % 2) == 1) printf("i is odd\n");
        if (i > 0 && (i % 2) == 0) printf("i is even\n");
        printf("i = %d\n", i);
        i = i + 1;
    }

    return 0;
}
