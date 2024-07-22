/**
 * @brief ArduinoAPI example
 */

#include <iostream>
#include "soft_timer.h"

void callback(soft_timer_t* timer) {
    printf("hello\n");
    if(timer->user_data){
        printf("%s", (char *)timer->user_data);
    }
}
void callback2(soft_timer_t* timer) {
    printf("world\n");
}

int main() {
    soft_timer_init(NULL);

    char *str = "user data";
    soft_timer_create(1000, callback, str);
    soft_timer_t *s = soft_timer_create(1000, callback2, NULL);
    soft_timer_set_repeat_count(s, 3);

    while (true)
    {
        soft_timer_handler();
    }

    std::cout << "Hello World!\n";
}