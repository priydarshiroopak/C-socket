#include <stdio.h>


int strcmpi(char *a, char *b) {
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0') {
        if (a[i] >= 'A' && a[i] <= 'Z')
            a[i] += 32;
        if (b[i] >= 'A' && b[i] <= 'Z')
            b[i] += 32;
        if (a[i] != b[i])
            return 1;
        i++;
    }
    if (a[i] != '\0' || b[i] != '\0')
        return 1;
    return 0;
}

int main() {
    printf("%d", strcmpi("Hello", "hello"));
}