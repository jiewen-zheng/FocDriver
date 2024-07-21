//
// Created by moon on 2023/10/29.
//

#ifndef COMMANDER_H
#define COMMANDER_H

#include <cstdio>
#include "FifoArray/FifoArray.h"

#define COMMAND_REGISTER_SIZE   16
#define MAX_COMMAND_LENGTH      32

#define CMD_PRINT(format, ...)  printf(format, ##__VA_ARGS__)

typedef void (*CommandCallback)(char *);    //!< command callback function pointer

class Commander : public FifoArray<char> {
public:
    /**
     * @param midl - the command and value interval character
     * @param eol - the end of line sentinel character
     * @param echo - echo last typed character (for command line feedback)
     */
    explicit Commander(size_t _size, char midl = '=', char eol = '\n', bool echo = true) :
            FifoArray<char>(_size), midl(midl), eol(eol), echo(echo) {}

    ~Commander(){}

    void add(const char *id, CommandCallback onCommand, const char *label = nullptr);

    void run();

    /**
     * Float variable scalar command interface
     * @param value     - float variable pointer
     * @param user_cmd  - the string command
     *
     * - It only has one property - one float value
     * - It can be get by sending an empty string '\n'
     * - It can be set by sending 'value' - (ex. 0.01f for setting *value=0.01)
     */
    void scalar(float *value, char *user_cmd);

    void inputCheck(char *input);

    bool isEnds(char ch);

private:

    void print(char msg);
    void print(const char *msg);
    void println(char msg);
    void println(float msg);
    void println(const char *msg);

private:
    struct {
        char            *id;
        CommandCallback onCommand;
        char            *label;
    }   commandList[COMMAND_REGISTER_SIZE];
    int cmd_count = 0;

    char midl = ' ';
    char eol  = '\n';
    bool echo = false;

    char receivedBuff[MAX_COMMAND_LENGTH] = {0};
    int  rec_cnt                          = 0;

};

#endif // COMMANDER_H
