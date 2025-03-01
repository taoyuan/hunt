/* Copyright (c) 2010 James Grenning and Contributed to Hunt Project
 * ==========================================
 *  Hunt Project - A Test Framework for C
 *  Copyright (c) 2007 Mike Karlesky, Mark VanderVoord, Greg Williams
 *  [Released under MIT License. Please refer to license.txt for details]
 * ========================================== */

#include "hunt_fixture.h"

static void runAllTests(void)
{
    RUN_TEST_GROUP(HuntFixture);
    RUN_TEST_GROUP(HuntCommandOptions);
    RUN_TEST_GROUP(LeakDetection);
    RUN_TEST_GROUP(InternalMalloc);
}

int main(int argc, const char* argv[])
{
    return HuntMain(argc, argv, runAllTests);
}

