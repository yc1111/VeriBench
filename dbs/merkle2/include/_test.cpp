// To test `merklesquare_go.h`
// run `g++ -o test _test.cpp ../lib/merklesquare_go.a -lpthread -lcrypto` to compile

extern "C" {
#include "./merklesquare_go.h"
}
#include <iostream>
#include <stdlib.h>
#include <unistd.h>

int main() {
    merklesquareclient_handle_t goobj_ = merklesquareclient_new("10.10.10.237:49563","10.10.10.237:49564","10.10.10.237:49562");
    int rc = 1;
    rc = merklesquareclient_set(goobj_, "key", "value");
    if (rc == 0){
        std::cout<< "Insert ok!" <<std::endl;
        char buf[512];
        sleep(3);
        rc = merklesquareclient_get(goobj_, "key", buf);
        if (rc == 0){
            std::cout<< buf <<std::endl;
        } else {
            std::cout<< "Read failed!" <<std::endl;
        }
    } else {
        std::cout<< "Insert failed!" <<std::endl;
    }
}
