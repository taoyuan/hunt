/* AUTOGENERATED FILE. DO NOT EDIT. */

/*=======Test Runner Used To Run Each Test Below=====*/
#define RUN_TEST(TestFunc, TestLineNum) \
{ \
  Hunt.CurrentTestName = #TestFunc; \
  Hunt.CurrentTestLineNumber = TestLineNum; \
  Hunt.NumberOfTests++; \
  if (TEST_PROTECT()) \
  { \
    CEXCEPTION_T e; \
    Try { \
      setUp(); \
      TestFunc(); \
    } Catch(e) { TEST_ASSERT_EQUAL_HEX32_MESSAGE(CEXCEPTION_NONE, e, "Unhandled Exception!"); } \
  } \
  if (TEST_PROTECT() && !TEST_IS_IGNORED) \
  { \
    tearDown(); \
  } \
  HuntConcludeTest(); \
}

/*=======Automagically Detected Files To Include=====*/
#include "hunt.h"
#include <setjmp.h>
#include <stdio.h>
#include "CException.h"
#include "one.h"
#include "two.h"
#include "funky.h"
#include "stanky.h"
#include <setjmp.h>

int GlobalExpectCount;
int GlobalVerifyOrder;
char* GlobalOrderError;

/*=======External Functions This Runner Calls=====*/
extern void setUp(void);
extern void tearDown(void);
extern void test_TheFirstThingToTest(void);
extern void test_TheSecondThingToTest(void);
extern void test_TheThirdThingToTest(void);
extern void test_TheFourthThingToTest(void);


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
  HuntBegin("testdata/testsample.c");
  RUN_TEST(test_TheFirstThingToTest, 21);
  RUN_TEST(test_TheSecondThingToTest, 43);
  RUN_TEST(test_TheThirdThingToTest, 53);
  RUN_TEST(test_TheFourthThingToTest, 58);

  return (HuntEnd());
}
