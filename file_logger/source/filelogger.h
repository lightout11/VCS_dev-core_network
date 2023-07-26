#ifndef FILE_LOGGER_H
#define FILE_LOGGER_H

#include <sys/stat.h>
#include <ctime>
#include <string>

class FileLogger {
    public:
        FileLogger(std::string file_path);

        time_t GetLastModified();

        bool IsModified();

    protected:

    private:
        int file_;
        time_t last_modified_;
};

#endif