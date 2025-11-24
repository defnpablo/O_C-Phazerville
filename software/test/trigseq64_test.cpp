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

  t.reset_playhead();

  EXPECT_EQ(0,  t.playhead_cursor());
  EXPECT_EQ(0,  t.step_cursor());
  EXPECT_EQ(15, t.end_cursor());
  EXPECT_EQ(1,  t.page_cursor());
}

TEST(TrigSeq64Test, AdvanceStep_RegularNext)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps,
              5,
              0,
              63,
              0);

  t.advance_playhead();
  EXPECT_EQ(6, t.playhead_cursor());
}

TEST(TrigSeq64Test, AdvanceStep_EndOfFirstPage_GoesToNextPage)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps,
              15,
              0,
              63,
              0);

  t.advance_playhead();

  EXPECT_EQ(16, t.playhead_cursor());
}

TEST(TrigSeq64Test, AdvanceStep_EndOfLastPage_WrapsToZero)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps,
              63,
              0,
              63,
              0);

  t.advance_playhead();

  EXPECT_EQ(0, t.playhead_cursor());
}