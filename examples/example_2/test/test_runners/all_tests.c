#include "hunt_fixture.h"

static void RunAllTests(void)
{
  RUN_TEST_GROUP(ProductionCode);
  RUN_TEST_GROUP(ProductionCode2);
}

int main(int argc, const char * argv[])
{
  return HuntMain(argc, argv, RunAllTests);
}
