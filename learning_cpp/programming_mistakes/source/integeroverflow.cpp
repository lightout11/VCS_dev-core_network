#include <iostream>

using namespace std;

void add(int& a, int& b) {
    long long c = a + b; // overflow
    cout << c << endl;
}

void solve(int& a, int& b) {
    long long c = (long long) a + b; // casting
    cout << c << endl;
}

int main(int argc, const char* argv[]) {
    int a = 2000000000;
    int b = 1000000000;
    add(a, b);
    solve(a, b);
    return 0;
}
