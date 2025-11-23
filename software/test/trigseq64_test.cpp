#include "gtest/gtest.h"
#include "trigseq64/TrigSeq64.h"

TEST(TrigSeq64Test, ValueEqualsOne_ShouldFail)
{
  TrigSeq64 t;
  EXPECT_EQ(1, t.selected_step);
}
