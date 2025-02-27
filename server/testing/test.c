#include <stdio.h>

char** getFruits() {
    static char* fruits[] = {"Apple", "Banana", "Cherry", NULL};  // NULL terminator
    return fruits;
}

int main() {
    char** fruits = getFruits();
    
    for (int i = 0; fruits[i] != NULL; i++) {
        printf("%s\n", fruits[i]);
    }
    return 0;
}
