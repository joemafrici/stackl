#include "test/io.h"

int inc(int *value)
{
    int ii;
    int my_val;
    my_val = *value;

    for (ii=0; ii<10; ii++)
    {
        asm("nop");
        ++my_val;
        asm("nop");
        asm("nop");
    }
    
    *value = my_val;
    return my_val;
}

int main()
{
    int ii;
    int value;

    value = 1;

    for (ii=0; ii<10; ii++)
    {
        inc(&value);

        printi(value);
        prints("\n");
    }
}

