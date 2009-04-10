int x, y, z;

int main() {
    int a, b, c, d;
    printf("** declated 3 global variables\n");
    printf("** declared 4 local variables\n");
    a = 0; 
    b = 1;
    c = 2;
    d = 3;
    x = 4;
    y = 5;
    z = 6;
    printf("** assigned 4 integer values\n");

    a = x * y;
    printf("** assignment statement tests:\n");
    printf("\ta = %d [20]\n", a);
    printf("\tb = %d [1]\n", b);
    printf("** end assignment tests\n");


    printf("** integer arithmetic tests:\n");
    printf("\t2 + 2     = %d\t[4]\n", 2 + 2);
    printf("\t2 + -2    = %d\t[0]\n", 2 + -2);
    printf("\t2 - 2     = %d\t[0]\n", 2 - 2);
    printf("\t2 * 2     = %d\t[4]\n", 2 * 2);
    printf("\t2 / 2     = %d\t[1]\n", 2 / 2);
    printf("\t2 % 2     = %d\t[0]\n", 2 % 2);
    printf("\t2 + 3 * 4 = %d\t[14]\n", 2 + 3 * 4);
    printf("** end integer arithmetic tests\n");

    printf("** return statement is required for compilation\n");
    return 0;
}
