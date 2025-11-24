#include "gtest/gtest.h"
#include "trigseq64/TrigSeq64.h"

TEST(TrigSeq64Test, InitialState_Defaults)
{
  TrigSeq64 t;

  EXPECT_EQ(0,  t.page_cursor());
  EXPECT_EQ(0,  t.step_cursor());
  EXPECT_EQ(0,  t.playhead_cursor());
  EXPECT_EQ(15, t.end_cursor());

  EXPECT_EQ(64, t.steps().size());
  EXPECT_TRUE(t.steps().none());
}
