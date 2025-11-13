#ifndef UART
#define UART

#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <iostream>
#include <string>
#include <vector>
#include "algs.h"

int openSerialPort(const std::string& portname);
bool configureSerialPort(int fd, int speed);
int sendCommand(int fd,const std::string& cmd);
std::vector<std::string> readResponse(int fd, double timeout_sec);
int uart_init(const std::string& portname, int br);
int get_baudrate(int);
#endif