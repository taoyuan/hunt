/* Copyright (c) 2010 James Grenning and Contributed to Hunt Project
 * ==========================================
 *  Hunt Project - A Test Framework for C
 *  Copyright (c) 2007 Mike Karlesky, Mark VanderVoord, Greg Williams
 *  [Released under MIT License. Please refer to license.txt for details]
 * ========================================== */


#include "hunt_output_Spy.h"
#include "hunt_fixture.h"

#include <stdio.h>
#include <string.h>

static int size;
static int count;
static char* buffer;
static int spy_enable;

void HuntOutputCharSpy_Create(int s)
{
    size = (s > 0) ? s : 0;
    count = 0;
    spy_enable = 0;
    buffer = malloc((size_t)size);
    TEST_ASSERT_NOT_NULL_MESSAGE(buffer, "Internal malloc failed in Spy Create():" __FILE__);
    memset(buffer, 0, (size_t)size);
}

void HuntOutputCharSpy_Destroy(void)
{
    size = 0;
    free(buffer);
}

void HuntOutputCharSpy_OutputChar(int c)
{
    if (spy_enable)
    {
        if (count < (size-1))
            buffer[count++] = (char)c;
    }
    else
    {
        putchar(c);
    }
}

const char * HuntOutputCharSpy_Get(void)
{
    return buffer;
}

void HuntOutputCharSpy_Enable(int enable)
{
    spy_enable = enable;
}
