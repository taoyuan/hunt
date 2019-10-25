/* AUTOGENERATED FILE. DO NOT EDIT. */

/*=======Test Runner Used To Run Each Test Below=====*/
#define RUN_TEST(TestFunc, TestLineNum) \
{ \
  Hunt.CurrentTestName = #TestFunc; \
  Hunt.CurrentTestLineNumber = TestLineNum; \
  Hunt.NumberOfTests++; \
  if (TEST_PROTECT()) \
  { \
      setUp(); \
      TestFunc(); \
  } \
  if (TEST_PROTECT()) \
  { \
    tearDown(); \
  } \
  HuntConcludeTest(); \
}

/*=======Automagically Detected Files To Include=====*/
#include "hunt.h"
#include <setjmp.h>
#include <stdio.h>
#include "ProductionCode2.h"

/*=======External Functions This Runner Calls=====*/
extern void setUp(void);
extern void tearDown(void);
extern void test_IgnoredTest(void);
extern void test_AnotherIgnoredTest(void);
extern void test_ThisFunctionHasNotBeenTested_NeedsToBeImplemented(void);


/*=======Test Reset Option=====*/
void resetTest(void);
void resetTest(void)
{
  tearDown();
  setUp();
}


/*=======MAIN=====*/
int main(void)
{
  HuntBegin("test/TestProductionCode2.c");
  RUN_TEST(test_IgnoredTest, 18);
  RUN_TEST(test_AnotherIgnoredTest, 23);
  RUN_TEST(test_ThisFunctionHasNotBeenTested_NeedsToBeImplemented, 28);

  return (HuntEnd());
}
