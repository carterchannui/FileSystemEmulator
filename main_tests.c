#include <stdio.h>
#include "checkit.h"

void test_array_loading(char arr[])
{
    checkit_char(arr[0], 'd');
    checkit_char(arr[1], 'd');
    checkit_char(arr[2], 'f');
    checkit_char(arr[3], 'd');
    checkit_char(arr[4], 'f');
}