#include <pthread.h>
#include <iostream>
#include <unistd.h>
#include <cstring>

using namespace std;

void* start_routine(void* args) {
    char* c = (char*)args;
    while (true) {
        cout << c << endl;
        if (c == "A") {
            usleep(200000);
        } else if (c == "B") {
            usleep(500000);
        } else if (c == "C") {
            usleep(1000000);
        }
    }
}

int main() {
    pthread_t threads[3];
    void* res;
    int t1 = pthread_create(&threads[0], NULL, start_routine, (void*)"A");
    if (t1 != 0) {
        cout << "Can't create thread. " << endl;
    }
    int t2 = pthread_create(&threads[1], NULL, start_routine, (void*)"B");
    if (t2 != 0) {
        cout << "Can't create thread. " << endl;
    }
    int t3 = pthread_create(&threads[2], NULL, start_routine, (void*)"C");
    if (t3 != 0) {
        cout << "Can't create thread. " << endl;
    }
    pthread_join(threads[0], &res);
    pthread_join(threads[1], &res);
    pthread_join(threads[2], &res);
}