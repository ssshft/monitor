#pragma once

#include <exception>
#include <iostream>
#include <cstring>
using namespace std;

class IGException: public exception {
public:
	IGException(const char* msg, int32_t errorcode = 0) : exception() {
        this->errorcode = errorcode;
        strcpy(this->msg, msg);
	}

    const char* what() const noexcept override {
        return msg;
    }

    const int32_t error_code() const noexcept {
        return errorcode;
    }

private:
    char msg[1024];
    int32_t errorcode{0};
};
