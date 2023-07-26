#include <iostream>
#include <fcntl.h>
#include <string>

using namespace std;

void create_file() {
    cout << "Enter file path: ";
    string path;
    cin >> path;
    cout << "Creating file..." << endl;
    if (creat(path.c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) {
        cout << "Success" << endl;
    } else {
        cout << "Failed" << endl;
    }
}

int main() {
    create_file();
}