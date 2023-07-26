#include <iostream>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

int main(int argc, char* argv[]) {
    int pid = fork();
    if (pid == 0) {
        cout << "pid: " << pid << endl;
        cout << "execl: " << endl << execl("./build/test", nullptr) << endl;
    }
    if (pid > 0) {
        cout << "pid: " << pid << endl;
        cout << "wait: " << endl << wait(nullptr) << endl;
    }
}