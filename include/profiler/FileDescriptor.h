#pragma once

#include <unistd.h>

namespace profiler{
    class FileDescriptor{
        public:
            explicit FileDescriptor(int fd) noexcept : fd_(fd){

            }

            ~FileDescriptor(){
                if(fd_ != -1){
                    ::close(fd_);
                }
            }

        FileDescriptor(const FileDescriptor&) = delete;
        FileDescriptor& operator=(
            const FileDescriptor&
        ) = delete;

        int get() const noexcept
        {
            return fd_;
        }

        void reset(int newFd = -1) noexcept
        {
            if(fd_ != -1){
                ::close(fd_);
            }

            fd_ = newFd;
        }

        private:
            int fd_ = -1;
    };
}