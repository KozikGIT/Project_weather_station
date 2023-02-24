#pragma once   //aby wykonywalo sie tylko raz

#include "stm32l4xx.h"     //aby uzywac huart i reszty
#include <stdio.h> //aby uzywac printf
#include <string.h>  //aby nie bylo warningu przy strcmp
extern UART_HandleTypeDef huart2;
void  print_your_console_write();




