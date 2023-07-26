#include <iostream>
#include <cstring>

using namespace std;

void copy(char a[], char b[]) { // copy b to a
    strcpy(a, b);
    cout << a << endl;
}

void solve(char a[], char b[]) { // check if size of source is larger than size of destination
    int sizea = sizeof(a) / sizeof(a[0]);
    int sizeb = sizeof(b) / sizeof(b[0]);
    if (sizea < sizeb) {
        cout << "Buffer overflow!" << endl;
        return;
    }
    strcpy(a, b);
}

int main(int argc, char* argv[]) {
    char a[5];
    char b[] = "jiwaoefijoweiafoje";
    copy(a, b);
    return 0;
}