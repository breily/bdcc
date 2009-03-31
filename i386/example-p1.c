int a;
int b;
int c;
double x;
double y;

int main() {
    x = 3;
    y = 2;
    a = 2;
    b = 6;
    c = b / a;
    printf("2 + 2 = %d\n", 2 + 2);
    printf("2 - 2 = %d\n", 2 - 2);
    printf("3 - 2 = %d\n", 3 - 2);
    printf("2 * 2 = %d\n", 2 * 2);
    printf("2 * 0 = %d\n", 2 * 0);
    printf("2 / 2 = %d\n", 2 / 2);
    printf("3 / 2 = %d\n", 3 / 2);
    printf("2 %% 2 = %d\n", 2 % 2);
    printf("3 %% 2 = %d\n", 3 % 2);
    printf("2 + 3 * 4 = %d\n", 2 + 3 * 4);
    printf("str: %s\n", "str");
    printf("&c: %d\n", &c);
    printf("c: %d\n", c);
    printf("3 + 2 = %f\n", x + y);
    return 0;
}
