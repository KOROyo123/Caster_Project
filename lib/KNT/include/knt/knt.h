#pragma once

#include <string>

std::string util_random_string(int string_len);

std::string util_cal_connect_key(const char *ServerIP, int serverPort, const char *ClientIP, int clientPort);

std::string util_cal_connect_key(int fd);

std::string util_port_to_key(int port);

std::string util_get_user_ip(int fd);
int util_get_user_port(int fd);

std::string util_get_date_time();
std::string util_get_space_time();

std::string util_get_http_date();