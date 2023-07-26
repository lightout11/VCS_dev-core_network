#include "filelogger.h"
#include <fcntl.h>
#include <iostream>
#include <sys/stat.h>

using namespace std;

FileLogger::FileLogger(string file_path) {
    file_ = open(file_path.c_str(), O_RDONLY);
    if (file_ == -1) {
        cout << "Error opening " << file_path << endl;
    }
    struct stat file_status;
    if (fstat(file_, &file_status) == -1) {
        cout << "Error getting file status." << endl;
    }
    last_modified_ = file_status.st_ctim.tv_sec;
}

time_t FileLogger::GetLastModified() {
    return last_modified_;
}

bool FileLogger::IsModified() {
    struct stat file_status;
    if (fstat(file_, &file_status) == -1) {
        cout << "Error getting file status." << endl;
    }
    time_t current_time = file_status.st_ctim.tv_sec;
    if (last_modified_ != current_time) {
        last_modified_ = current_time;
        return true;
    }
    return false;
}