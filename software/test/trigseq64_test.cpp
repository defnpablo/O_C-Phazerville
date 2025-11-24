#include "gtest/gtest.h"
#include "trigseq64/TrigSeq64.h"

TEST(TrigSeq64Test, InitialState_Defaults)
{
  TrigSeq64 t;

  EXPECT_EQ(0,  t.page_cursor());
  EXPECT_EQ(0,  t.step_cursor());
  EXPECT_EQ(0,  t.playhead_cursor());
  EXPECT_EQ(15, t.end_cursor());

  EXPECT_EQ(64u, t.steps().size());
  EXPECT_TRUE(t.steps().none());
}

TEST(TrigSeq64Test, Reset)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps,
              2,
              0,
              15,
              1);

  t.reset();

  EXPECT_EQ(0,  t.playhead_cursor());
  EXPECT_EQ(0,  t.step_cursor());
  EXPECT_EQ(15, t.end_cursor());
  EXPECT_EQ(1,  t.page_cursor());
}