#include <stdio.h>
#include "../include/socket.h"

int return_sock_port_from_number(int number)
{
  int sum_of_digits = 0;
  int number_backup = number;

  int index = 1;
  while (number) {
    sum_of_digits += number%10 * index;
    number/=10;
    index++;
  }
  int port = 8000+sum_of_digits;

  if (port > MAX_PORT)
    return -1;

  return port;
}
