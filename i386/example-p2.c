int main() {
    int i, j, k;

    i = 1;
    j = 4;
    k = 6;
    if (i < j)
        printf("i < j\n");
    if (k > j)
        printf("k > j\n");
    else
        printf("k <= j\n");
    if (k == j)
        printf("k == j\n");
    else
        printf("k != j\n");
    i = 0;
    while (i < 10) {
        printf("i is %d\n", i);
        i = i + 1;
    }
    for (j = 0; j < 10; j = j + 1)
        printf("j is %d\n", j);
    return 0;
}
