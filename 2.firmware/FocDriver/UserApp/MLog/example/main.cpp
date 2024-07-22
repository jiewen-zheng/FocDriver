//
// Created by admin on 2024/5/15.
//

#include <iostream>
#include "stdio.h"
#include <time.h>

#include "../MLog.h"


void print_cb(const char *buf){
    printf("%s", buf);
}

uint32_t get_tick(){
    static uint32_t time = 0;
    return time + rand();
}

int main() {
    m_log.setPrintCb(print_cb);
    m_log.setTickCallback(get_tick);

    M_LOG_TRACE("Trace");
    M_LOG_INFO("MLOG");
    M_LOG_ERROR("hello world");
    M_LOG("message");
}