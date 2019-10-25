/* Copyright (c) 2010 James Grenning and Contributed to Hunt Project
 * ==========================================
 *  Hunt Project - A Test Framework for C
 *  Copyright (c) 2007 Mike Karlesky, Mark VanderVoord, Greg Williams
 *  [Released under MIT License. Please refer to license.txt for details]
 * ========================================== */

#include "hunt_fixture.h"

TEST_GROUP_RUNNER(HuntFixture)
{
    RUN_TEST_CASE(HuntFixture, PointerSetting);
    RUN_TEST_CASE(HuntFixture, ForceMallocFail);
    RUN_TEST_CASE(HuntFixture, ReallocSmallerIsUnchanged);
    RUN_TEST_CASE(HuntFixture, ReallocSameIsUnchanged);
    RUN_TEST_CASE(HuntFixture, ReallocLargerNeeded);
    RUN_TEST_CASE(HuntFixture, ReallocNullPointerIsLikeMalloc);
    RUN_TEST_CASE(HuntFixture, ReallocSizeZeroFreesMemAndReturnsNullPointer);
    RUN_TEST_CASE(HuntFixture, CallocFillsWithZero);
    RUN_TEST_CASE(HuntFixture, PointerSet);
    RUN_TEST_CASE(HuntFixture, FreeNULLSafety);
    RUN_TEST_CASE(HuntFixture, ConcludeTestIncrementsFailCount);
}

TEST_GROUP_RUNNER(HuntCommandOptions)
{
    RUN_TEST_CASE(HuntCommandOptions, DefaultOptions);
    RUN_TEST_CASE(HuntCommandOptions, OptionVerbose);
    RUN_TEST_CASE(HuntCommandOptions, OptionSelectTestByGroup);
    RUN_TEST_CASE(HuntCommandOptions, OptionSelectTestByName);
    RUN_TEST_CASE(HuntCommandOptions, OptionSelectRepeatTestsDefaultCount);
    RUN_TEST_CASE(HuntCommandOptions, OptionSelectRepeatTestsSpecificCount);
    RUN_TEST_CASE(HuntCommandOptions, MultipleOptions);
    RUN_TEST_CASE(HuntCommandOptions, MultipleOptionsDashRNotLastAndNoValueSpecified);
    RUN_TEST_CASE(HuntCommandOptions, UnknownCommandIsIgnored);
    RUN_TEST_CASE(HuntCommandOptions, GroupOrNameFilterWithoutStringFails);
    RUN_TEST_CASE(HuntCommandOptions, GroupFilterReallyFilters);
    RUN_TEST_CASE(HuntCommandOptions, TestShouldBeIgnored);
}

TEST_GROUP_RUNNER(LeakDetection)
{
    RUN_TEST_CASE(LeakDetection, DetectsLeak);
    RUN_TEST_CASE(LeakDetection, BufferOverrunFoundDuringFree);
    RUN_TEST_CASE(LeakDetection, BufferOverrunFoundDuringRealloc);
    RUN_TEST_CASE(LeakDetection, BufferGuardWriteFoundDuringFree);
    RUN_TEST_CASE(LeakDetection, BufferGuardWriteFoundDuringRealloc);
    RUN_TEST_CASE(LeakDetection, PointerSettingMax);
}

TEST_GROUP_RUNNER(InternalMalloc)
{
    RUN_TEST_CASE(InternalMalloc, MallocPastBufferFails);
    RUN_TEST_CASE(InternalMalloc, CallocPastBufferFails);
    RUN_TEST_CASE(InternalMalloc, MallocThenReallocGrowsMemoryInPlace);
    RUN_TEST_CASE(InternalMalloc, ReallocFailDoesNotFreeMem);
}
