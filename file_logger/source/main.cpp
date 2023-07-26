#include "filelogger.h"
#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
    cout << "Enter file path: ";
    string file_path;
    cin >> file_path;
    FileLogger file_logger(file_path);

    while (true) {
        if (file_logger.IsModified()) {
            time_t last_modified = file_logger.GetLastModified();
            cout << "File is modified at " << ctime(&last_modified);
        }
    }
}