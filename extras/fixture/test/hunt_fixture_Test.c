/* Copyright (c) 2010 James Grenning and Contributed to Hunt Project
 * ==========================================
 *  Hunt Project - A Test Framework for C
 *  Copyright (c) 2007 Mike Karlesky, Mark VanderVoord, Greg Williams
 *  [Released under MIT License. Please refer to license.txt for details]
 * ========================================== */

#include "hunt_fixture.h"
#include "hunt_output_Spy.h"
#include <stdlib.h>
#include <string.h>

TEST_GROUP(HuntFixture);

TEST_SETUP(HuntFixture)
{
}

TEST_TEAR_DOWN(HuntFixture)
{
}

static int* pointer1 = 0;
static int* pointer2 = (int*)2;
static int* pointer3 = (int*)3;
static int int1;
static int int2;
static int int3;
static int int4;

TEST(HuntFixture, PointerSetting)
{
    TEST_ASSERT_POINTERS_EQUAL(pointer1, 0);
    UT_PTR_SET(pointer1, &int1);
    UT_PTR_SET(pointer2, &int2);
    UT_PTR_SET(pointer3, &int3);
    TEST_ASSERT_POINTERS_EQUAL(pointer1, &int1);
    TEST_ASSERT_POINTERS_EQUAL(pointer2, &int2);
    TEST_ASSERT_POINTERS_EQUAL(pointer3, &int3);
    UT_PTR_SET(pointer1, &int4);
    HuntPointer_UndoAllSets();
    TEST_ASSERT_POINTERS_EQUAL(pointer1, 0);
    TEST_ASSERT_POINTERS_EQUAL(pointer2, (int*)2);
    TEST_ASSERT_POINTERS_EQUAL(pointer3, (int*)3);
}

TEST(HuntFixture, ForceMallocFail)
{
    void* m;
    void* mfails;
    HuntMalloc_MakeMallocFailAfterCount(1);
    m = malloc(10);
    CHECK(m);
    mfails = malloc(10);
    TEST_ASSERT_POINTERS_EQUAL(0, mfails);
    free(m);
}

TEST(HuntFixture, ReallocSmallerIsUnchanged)
{
    void* m1 = malloc(10);
    void* m2 = realloc(m1, 5);
    TEST_ASSERT_POINTERS_EQUAL(m1, m2);
    free(m2);
}

TEST(HuntFixture, ReallocSameIsUnchanged)
{
    void* m1 = malloc(10);
    void* m2 = realloc(m1, 10);
    TEST_ASSERT_POINTERS_EQUAL(m1, m2);
    free(m2);
}

TEST(HuntFixture, ReallocLargerNeeded)
{
    void* m1 = malloc(10);
    void* m2;
    CHECK(m1);
    strcpy((char*)m1, "123456789");
    m2 = realloc(m1, 15);
    /* CHECK(m1 != m2); //Depends on implementation */
    STRCMP_EQUAL("123456789", m2);
    free(m2);
}

TEST(HuntFixture, ReallocNullPointerIsLikeMalloc)
{
    void* m = realloc(0, 15);
    CHECK(m != 0);
    free(m);
}

TEST(HuntFixture, ReallocSizeZeroFreesMemAndReturnsNullPointer)
{
    void* m1 = malloc(10);
    void* m2 = realloc(m1, 0);
    TEST_ASSERT_POINTERS_EQUAL(0, m2);
}

TEST(HuntFixture, CallocFillsWithZero)
{
    void* m = calloc(3, sizeof(char));
    char* s = (char*)m;
    CHECK(m);
    TEST_ASSERT_BYTES_EQUAL(0, s[0]);
    TEST_ASSERT_BYTES_EQUAL(0, s[1]);
    TEST_ASSERT_BYTES_EQUAL(0, s[2]);
    free(m);
}

static char *p1;
static char *p2;

TEST(HuntFixture, PointerSet)
{
    char c1;
    char c2;
    char newC1;
    char newC2;
    p1 = &c1;
    p2 = &c2;

    HuntPointer_Init();
    UT_PTR_SET(p1, &newC1);
    UT_PTR_SET(p2, &newC2);
    TEST_ASSERT_POINTERS_EQUAL(&newC1, p1);
    TEST_ASSERT_POINTERS_EQUAL(&newC2, p2);
    HuntPointer_UndoAllSets();
    TEST_ASSERT_POINTERS_EQUAL(&c1, p1);
    TEST_ASSERT_POINTERS_EQUAL(&c2, p2);
}

TEST(HuntFixture, FreeNULLSafety)
{
    free(NULL);
}

TEST(HuntFixture, ConcludeTestIncrementsFailCount)
{
    HUNT_UINT savedFails = Hunt.TestFailures;
    HUNT_UINT savedIgnores = Hunt.TestIgnores;
    HuntOutputCharSpy_Enable(1);
    Hunt.CurrentTestFailed = 1;
    HuntConcludeFixtureTest(); /* Resets TestFailed for this test to pass */
    Hunt.CurrentTestIgnored = 1;
    HuntConcludeFixtureTest(); /* Resets TestIgnored */
    HuntOutputCharSpy_Enable(0);
    TEST_ASSERT_EQUAL(savedFails + 1, Hunt.TestFailures);
    TEST_ASSERT_EQUAL(savedIgnores + 1, Hunt.TestIgnores);
    Hunt.TestFailures = savedFails;
    Hunt.TestIgnores = savedIgnores;
}

/*------------------------------------------------------------ */

TEST_GROUP(HuntCommandOptions);

static int savedVerbose;
static unsigned int savedRepeat;
static const char* savedName;
static const char* savedGroup;

TEST_SETUP(HuntCommandOptions)
{
    savedVerbose = HuntFixture.Verbose;
    savedRepeat = HuntFixture.RepeatCount;
    savedName = HuntFixture.NameFilter;
    savedGroup = HuntFixture.GroupFilter;
}

TEST_TEAR_DOWN(HuntCommandOptions)
{
    HuntFixture.Verbose = savedVerbose;
    HuntFixture.RepeatCount= savedRepeat;
    HuntFixture.NameFilter = savedName;
    HuntFixture.GroupFilter = savedGroup;
}


static const char* noOptions[] = {
        "testrunner.exe"
};

TEST(HuntCommandOptions, DefaultOptions)
{
    HuntGetCommandLineOptions(1, noOptions);
    TEST_ASSERT_EQUAL(0, HuntFixture.Verbose);
    TEST_ASSERT_POINTERS_EQUAL(0, HuntFixture.GroupFilter);
    TEST_ASSERT_POINTERS_EQUAL(0, HuntFixture.NameFilter);
    TEST_ASSERT_EQUAL(1, HuntFixture.RepeatCount);
}

static const char* verbose[] = {
        "testrunner.exe",
        "-v"
};

TEST(HuntCommandOptions, OptionVerbose)
{
    TEST_ASSERT_EQUAL(0, HuntGetCommandLineOptions(2, verbose));
    TEST_ASSERT_EQUAL(1, HuntFixture.Verbose);
}

static const char* group[] = {
        "testrunner.exe",
        "-g", "groupname"
};

TEST(HuntCommandOptions, OptionSelectTestByGroup)
{
    TEST_ASSERT_EQUAL(0, HuntGetCommandLineOptions(3, group));
    STRCMP_EQUAL("groupname", HuntFixture.GroupFilter);
}

static const char* name[] = {
        "testrunner.exe",
        "-n", "testname"
};

TEST(HuntCommandOptions, OptionSelectTestByName)
{
    TEST_ASSERT_EQUAL(0, HuntGetCommandLineOptions(3, name));
    STRCMP_EQUAL("testname", HuntFixture.NameFilter);
}

static const char* repeat[] = {
        "testrunner.exe",
        "-r", "99"
};

TEST(HuntCommandOptions, OptionSelectRepeatTestsDefaultCount)
{
    TEST_ASSERT_EQUAL(0, HuntGetCommandLineOptions(2, repeat));
    TEST_ASSERT_EQUAL(2, HuntFixture.RepeatCount);
}

TEST(HuntCommandOptions, OptionSelectRepeatTestsSpecificCount)
{
    TEST_ASSERT_EQUAL(0, HuntGetCommandLineOptions(3, repeat));
    TEST_ASSERT_EQUAL(99, HuntFixture.RepeatCount);
}

static const char* multiple[] = {
        "testrunner.exe",
        "-v",
        "-g", "groupname",
        "-n", "testname",
        "-r", "98"
};

TEST(HuntCommandOptions, MultipleOptions)
{
    TEST_ASSERT_EQUAL(0, HuntGetCommandLineOptions(8, multiple));
    TEST_ASSERT_EQUAL(1, HuntFixture.Verbose);
    STRCMP_EQUAL("groupname", HuntFixture.GroupFilter);
    STRCMP_EQUAL("testname", HuntFixture.NameFilter);
    TEST_ASSERT_EQUAL(98, HuntFixture.RepeatCount);
}

static const char* dashRNotLast[] = {
        "testrunner.exe",
        "-v",
        "-g", "gggg",
        "-r",
        "-n", "tttt",
};

TEST(HuntCommandOptions, MultipleOptionsDashRNotLastAndNoValueSpecified)
{
    TEST_ASSERT_EQUAL(0, HuntGetCommandLineOptions(7, dashRNotLast));
    TEST_ASSERT_EQUAL(1, HuntFixture.Verbose);
    STRCMP_EQUAL("gggg", HuntFixture.GroupFilter);
    STRCMP_EQUAL("tttt", HuntFixture.NameFilter);
    TEST_ASSERT_EQUAL(2, HuntFixture.RepeatCount);
}

static const char* unknownCommand[] = {
        "testrunner.exe",
        "-v",
        "-g", "groupname",
        "-n", "testname",
        "-r", "98",
        "-z"
};
TEST(HuntCommandOptions, UnknownCommandIsIgnored)
{
    TEST_ASSERT_EQUAL(0, HuntGetCommandLineOptions(9, unknownCommand));
    TEST_ASSERT_EQUAL(1, HuntFixture.Verbose);
    STRCMP_EQUAL("groupname", HuntFixture.GroupFilter);
    STRCMP_EQUAL("testname", HuntFixture.NameFilter);
    TEST_ASSERT_EQUAL(98, HuntFixture.RepeatCount);
}

TEST(HuntCommandOptions, GroupOrNameFilterWithoutStringFails)
{
    TEST_ASSERT_EQUAL(1, HuntGetCommandLineOptions(3, unknownCommand));
    TEST_ASSERT_EQUAL(1, HuntGetCommandLineOptions(5, unknownCommand));
    TEST_ASSERT_EQUAL(1, HuntMain(3, unknownCommand, NULL));
}

TEST(HuntCommandOptions, GroupFilterReallyFilters)
{
    HUNT_UINT saved = Hunt.NumberOfTests;
    TEST_ASSERT_EQUAL(0, HuntGetCommandLineOptions(4, unknownCommand));
    HuntIgnoreTest(NULL, "non-matching", NULL);
    TEST_ASSERT_EQUAL(saved, Hunt.NumberOfTests);
}

IGNORE_TEST(HuntCommandOptions, TestShouldBeIgnored)
{
    TEST_FAIL_MESSAGE("This test should not run!");
}

/*------------------------------------------------------------ */

TEST_GROUP(LeakDetection);

TEST_SETUP(LeakDetection)
{
#ifdef HUNT_EXCLUDE_STDLIB_MALLOC
    HuntOutputCharSpy_Create(200);
#else
    HuntOutputCharSpy_Create(1000);
#endif
}

TEST_TEAR_DOWN(LeakDetection)
{
    HuntOutputCharSpy_Destroy();
}

#define EXPECT_ABORT_BEGIN \
  { \
    jmp_buf TestAbortFrame;   \
    memcpy(TestAbortFrame, Hunt.AbortFrame, sizeof(jmp_buf)); \
    if (TEST_PROTECT()) \
    {

#define EXPECT_ABORT_END \
    } \
    memcpy(Hunt.AbortFrame, TestAbortFrame, sizeof(jmp_buf)); \
  }

/* This tricky set of defines lets us see if we are using the Spy, returns 1 if true */
#ifdef __STDC_VERSION__

#if __STDC_VERSION__ >= 199901L
#define USING_SPY_AS(a)                    EXPAND_AND_USE_2ND(ASSIGN_VALUE(a), 0)
#define ASSIGN_VALUE(a)                    VAL_##a
#define VAL_HuntOutputCharSpy_OutputChar  0, 1
#define EXPAND_AND_USE_2ND(a, b)           SECOND_PARAM(a, b, throwaway)
#define SECOND_PARAM(a, b, ...)            b
#if USING_SPY_AS(HUNT_OUTPUT_CHAR)
  #define USING_OUTPUT_SPY /* HUNT_OUTPUT_CHAR = HuntOutputCharSpy_OutputChar */
#endif
#endif /* >= 199901 */

#else  /* __STDC_VERSION__ else */
#define HuntOutputCharSpy_OutputChar 42
#if HUNT_OUTPUT_CHAR == HuntOutputCharSpy_OutputChar /* Works if no -Wundef -Werror */
  #define USING_OUTPUT_SPY
#endif
#undef HuntOutputCharSpy_OutputChar
#endif /* __STDC_VERSION__ */

TEST(LeakDetection, DetectsLeak)
{
#ifndef USING_OUTPUT_SPY
    TEST_IGNORE_MESSAGE("Build with '-D HUNT_OUTPUT_CHAR=HuntOutputCharSpy_OutputChar' to enable tests");
#else
    void* m = malloc(10);
    TEST_ASSERT_NOT_NULL(m);
    HuntOutputCharSpy_Enable(1);
    EXPECT_ABORT_BEGIN
    HuntMalloc_EndTest();
    EXPECT_ABORT_END
    HuntOutputCharSpy_Enable(0);
    Hunt.CurrentTestFailed = 0;
    CHECK(strstr(HuntOutputCharSpy_Get(), "This test leaks!"));
    free(m);
#endif
}

TEST(LeakDetection, BufferOverrunFoundDuringFree)
{
#ifndef USING_OUTPUT_SPY
    TEST_IGNORE();
#else
    void* m = malloc(10);
    char* s = (char*)m;
    TEST_ASSERT_NOT_NULL(m);
    s[10] = (char)0xFF;
    HuntOutputCharSpy_Enable(1);
    EXPECT_ABORT_BEGIN
    free(m);
    EXPECT_ABORT_END
    HuntOutputCharSpy_Enable(0);
    Hunt.CurrentTestFailed = 0;
    CHECK(strstr(HuntOutputCharSpy_Get(), "Buffer overrun detected during free()"));
#endif
}

TEST(LeakDetection, BufferOverrunFoundDuringRealloc)
{
#ifndef USING_OUTPUT_SPY
    TEST_IGNORE();
#else
    void* m = malloc(10);
    char* s = (char*)m;
    TEST_ASSERT_NOT_NULL(m);
    s[10] = (char)0xFF;
    HuntOutputCharSpy_Enable(1);
    EXPECT_ABORT_BEGIN
    m = realloc(m, 100);
    EXPECT_ABORT_END
    HuntOutputCharSpy_Enable(0);
    Hunt.CurrentTestFailed = 0;
    CHECK(strstr(HuntOutputCharSpy_Get(), "Buffer overrun detected during realloc()"));
#endif
}

TEST(LeakDetection, BufferGuardWriteFoundDuringFree)
{
#ifndef USING_OUTPUT_SPY
    TEST_IGNORE();
#else
    void* m = malloc(10);
    char* s = (char*)m;
    TEST_ASSERT_NOT_NULL(m);
    s[-1] = (char)0x00; /* Will not detect 0 */
    s[-2] = (char)0x01;
    HuntOutputCharSpy_Enable(1);
    EXPECT_ABORT_BEGIN
    free(m);
    EXPECT_ABORT_END
    HuntOutputCharSpy_Enable(0);
    Hunt.CurrentTestFailed = 0;
    CHECK(strstr(HuntOutputCharSpy_Get(), "Buffer overrun detected during free()"));
#endif
}

TEST(LeakDetection, BufferGuardWriteFoundDuringRealloc)
{
#ifndef USING_OUTPUT_SPY
    TEST_IGNORE();
#else
    void* m = malloc(10);
    char* s = (char*)m;
    TEST_ASSERT_NOT_NULL(m);
    s[-1] = (char)0x0A;
    HuntOutputCharSpy_Enable(1);
    EXPECT_ABORT_BEGIN
    m = realloc(m, 100);
    EXPECT_ABORT_END
    HuntOutputCharSpy_Enable(0);
    Hunt.CurrentTestFailed = 0;
    CHECK(strstr(HuntOutputCharSpy_Get(), "Buffer overrun detected during realloc()"));
#endif
}

TEST(LeakDetection, PointerSettingMax)
{
#ifndef USING_OUTPUT_SPY
    TEST_IGNORE();
#else
    int i;
    for (i = 0; i < HUNT_MAX_POINTERS; i++) UT_PTR_SET(pointer1, &int1);
    HuntOutputCharSpy_Enable(1);
    EXPECT_ABORT_BEGIN
    UT_PTR_SET(pointer1, &int1);
    EXPECT_ABORT_END
    HuntOutputCharSpy_Enable(0);
    Hunt.CurrentTestFailed = 0;
    CHECK(strstr(HuntOutputCharSpy_Get(), "Too many pointers set"));
#endif
}

/*------------------------------------------------------------ */

TEST_GROUP(InternalMalloc);
#define TEST_ASSERT_MEMORY_ALL_FREE_LIFO_ORDER(first_mem_ptr, ptr) \
    ptr = malloc(10); free(ptr);                                   \
    TEST_ASSERT_EQUAL_PTR_MESSAGE(first_mem_ptr, ptr, "Memory was stranded, free in LIFO order");


TEST_SETUP(InternalMalloc) { }
TEST_TEAR_DOWN(InternalMalloc) { }

TEST(InternalMalloc, MallocPastBufferFails)
{
#ifdef HUNT_EXCLUDE_STDLIB_MALLOC
    void* m = malloc(HUNT_INTERNAL_HEAP_SIZE_BYTES/2 + 1);
    void* n = malloc(HUNT_INTERNAL_HEAP_SIZE_BYTES/2);
    free(m);
    TEST_ASSERT_NOT_NULL(m);
    TEST_ASSERT_NULL(n);
    TEST_ASSERT_MEMORY_ALL_FREE_LIFO_ORDER(m, n);
#endif
}

TEST(InternalMalloc, CallocPastBufferFails)
{
#ifdef HUNT_EXCLUDE_STDLIB_MALLOC
    void* m = calloc(1, HUNT_INTERNAL_HEAP_SIZE_BYTES/2 + 1);
    void* n = calloc(1, HUNT_INTERNAL_HEAP_SIZE_BYTES/2);
    free(m);
    TEST_ASSERT_NOT_NULL(m);
    TEST_ASSERT_NULL(n);
    TEST_ASSERT_MEMORY_ALL_FREE_LIFO_ORDER(m, n);
#endif
}

TEST(InternalMalloc, MallocThenReallocGrowsMemoryInPlace)
{
#ifdef HUNT_EXCLUDE_STDLIB_MALLOC
    void* m = malloc(HUNT_INTERNAL_HEAP_SIZE_BYTES/2 + 1);
    void* n = realloc(m, HUNT_INTERNAL_HEAP_SIZE_BYTES/2 + 9);
    free(n);
    TEST_ASSERT_NOT_NULL(m);
    TEST_ASSERT_EQUAL(m, n);
    TEST_ASSERT_MEMORY_ALL_FREE_LIFO_ORDER(m, n);
#endif
}

TEST(InternalMalloc, ReallocFailDoesNotFreeMem)
{
#ifdef HUNT_EXCLUDE_STDLIB_MALLOC
    void* m = malloc(HUNT_INTERNAL_HEAP_SIZE_BYTES/2);
    void* n1 = malloc(10);
    void* out_of_mem = realloc(n1, HUNT_INTERNAL_HEAP_SIZE_BYTES/2 + 1);
    void* n2 = malloc(10);

    free(n2);
    if (out_of_mem == NULL) free(n1);
    free(m);

    TEST_ASSERT_NOT_NULL(m);       /* Got a real memory location */
    TEST_ASSERT_NULL(out_of_mem);  /* The realloc should have failed */
    TEST_ASSERT_NOT_EQUAL(n2, n1); /* If n1 != n2 then realloc did not free n1 */
    TEST_ASSERT_MEMORY_ALL_FREE_LIFO_ORDER(m, n2);
#endif
}
