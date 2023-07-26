#include <iostream>
#include <cstring>

using namespace std;

void change(char s[], int i, char c) { // change a character in a c-string
    s[i] = c;
}

void solve(char s[], int i, char c) { // check boundaries
    int size = sizeof(s) / sizeof(s[0]);
    if (i >= size) {
        cout << "Out of bound!" << endl;
        return;
    }
    s[i] = c;
}

int main(int argc, const char* argv[])
{
    char s[] = "abcdef";
    change(s, 7, 'n'); // stack smashing detected
    cout << strlen(s) << endl;
    solve(s, 4, 'n');
    cout << strlen(s) << endl;
    return 0;
}
