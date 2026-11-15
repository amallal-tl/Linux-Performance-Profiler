#pragma once

#include <unistd.h>
#include <utility>

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

        FileDescriptor(FileDescriptor&& other) noexcept :fd_(other.fd_){

        }

        FileDescriptor& operator=(FileDescriptor&& other) noexcept{
            if(this == &other) return *this;

            if(fd_ != -1){
                ::close(fd_);
            }

            fd_ = std::exchange(other.fd_, -1);

            return *this;

        }

        int get() const noexcept
        {
            return fd_;
        }

        int release() noexcept{
            return std::exchange(fd_, -1);
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