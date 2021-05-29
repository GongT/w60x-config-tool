#pragma once

#include "helpers.h"

#include <rtthread.h>
#include <rtdevice.h>

void start_tcp_server();
void tcp_thread(void *arg);
