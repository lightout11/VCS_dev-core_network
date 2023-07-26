#include <iostream>
#include <cstdlib>

using namespace std;

/*
    Allocate memory with new operator
    Allocate memory with malloc() function
    Allocate memory with calloc() function 
*/
void memoryleak() {
    int* a = new int(1);
    int* b = (int*)malloc(sizeof(int));
    int n = 4;
    int* c = (int*)calloc(n, sizeof(int));
    *b = 10;
    for (int i = 0; i < 4; i++) {
        c[i] = i + 1;
    }

    cout << *a << endl;
    cout << *b << endl;
    for (int i = 0; i < 4; i++) {
        cout << c[i] << " ";
    }
    cout << endl;
} // cause memory leaking

void solve() {
    int* a = new int(1);
    int* b = (int*)malloc(sizeof(int));
    int n = 4;
    int* c = (int*)calloc(n, sizeof(int));
    *b = 10;
    for (int i = 0; i < 4; i++) {
        c[i] = i + 1;
    }

    cout << *a << endl;
    cout << *b << endl;
    for (int i = 0; i < 4; i++) {
        cout << c[i] << " ";
    }
    cout << endl;

    // deallocate memory
    delete a; 
    free(b);
    free(c);
}

int main() {
    memoryleak();
    solve();
    return 0;
}