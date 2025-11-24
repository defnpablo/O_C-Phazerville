#include "gtest/gtest.h"
#include "trigseq64/TrigSeq64.h"

// Initialization and state tests
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

// Playhead navigation tests
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

// Page navigation tests
TEST(TrigSeq64Test, AdvancePage_FromMiddle_Increments)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps,
              0, // playhead
              0, // step
              63, // end
              1); // page

  t.advance_page();
  EXPECT_EQ(2, t.page_cursor());
}

TEST(TrigSeq64Test, AdvancePage_AtLastPage_DoesNothing)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps,
              0,
              0,
              63,
              TrigSeq64::PAGE_COUNT - 1);

  t.advance_page();
  EXPECT_EQ(TrigSeq64::PAGE_COUNT - 1, t.page_cursor());
}

TEST(TrigSeq64Test, RetreatPage_FromMiddle_Decrements)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps,
              0,
              0,
              63,
              2);

  t.retreat_page();
  EXPECT_EQ(1, t.page_cursor());
}

TEST(TrigSeq64Test, RetreatPage_AtFirstPage_DoesNothing)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps,
              0,
              0,
              63,
              0);

  t.retreat_page();
  EXPECT_EQ(0, t.page_cursor());
}

// End cursor navigation tests
TEST(TrigSeq64Test, AdvanceEndCursor_FromMiddle_Increments)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps,
              0,
              0,
              10,
              0);

  t.advance_end_cursor();
  EXPECT_EQ(11, t.end_cursor());
}

TEST(TrigSeq64Test, AdvanceEndCursor_AtMax_DoesNothing)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps,
              0,
              0,
              TrigSeq64::MAX_STEPS - 1,
              0);

  t.advance_end_cursor();
  EXPECT_EQ(TrigSeq64::MAX_STEPS - 1, t.end_cursor());
}

TEST(TrigSeq64Test, etreatEndCursor_FromMiddle_Decrements)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps,
              0,
              0,
              20,
              0);

  t.retreat_end_cursor();
  EXPECT_EQ(19, t.end_cursor());
}

TEST(TrigSeq64Test, RetreatEndCursor_AtZero_DoesNothing)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps,
              0,
              0,
              0,
              0);

  t.retreat_end_cursor();
  EXPECT_EQ(0, t.end_cursor());
}

// Endxcursor to page end tests
TEST(TrigSeq64Test, SetEndCursorToPageEnd_Page0_SetsTo15)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps,
              0,
              0,
              10,
              0);

  t.set_end_cursor_to_page_end();
  EXPECT_EQ(15, t.end_cursor());
}

TEST(TrigSeq64Test, SetEndCursorToPageEnd_Page1_SetsTo31)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps,
              0,
              0,
              5,
              1);

  t.set_end_cursor_to_page_end();
  EXPECT_EQ(31, t.end_cursor());
}

TEST(TrigSeq64Test, SetEndCursorToPageEnd_Page2_SetsTo47)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps,
              0,
              0,
              20,
              2);

  t.set_end_cursor_to_page_end();
  EXPECT_EQ(47, t.end_cursor());
}

TEST(TrigSeq64Test, SetEndCursorToPageEnd_Page3_SetsTo63)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps,
              0,
              0,
              30,
              3);

  t.set_end_cursor_to_page_end();
  EXPECT_EQ(63, t.end_cursor());
}

// Step cursor navigation tests
TEST(TrigSeq64Test, AdvanceStepCursor_WithinPage_Increments)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps,
              0,
              5,
              63,
              0);

  t.advance_step_cursor();
  EXPECT_EQ(6, t.step_cursor());
}

TEST(TrigSeq64Test, AdvanceStepCursor_AtPageEnd_WrapsToPageStart)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps,
              0,
              15, // last step of page 0
              63,
              0);

  t.advance_step_cursor();
  EXPECT_EQ(0, t.step_cursor());
}

TEST(TrigSeq64Test, AdvanceStepCursor_Page2_AtPageEnd_WrapsToPageStart)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps,
              0,
              47, // last step of page 2
              63,
              2);

  t.advance_step_cursor();
  EXPECT_EQ(32, t.step_cursor());
}

TEST(TrigSeq64Test, RetreatStepCursor_WithinPage_Decrements)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps,
              0,
              10,
              63,
              0);

  t.retreat_step_cursor();
  EXPECT_EQ(9, t.step_cursor());
}

TEST(TrigSeq64Test, RetreatStepCursor_AtPageStart_WrapsToPageEnd)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps,
              0,
              0, // first step of page 0
              63,
              0);

  t.retreat_step_cursor();
  EXPECT_EQ(15, t.step_cursor());
}

TEST(TrigSeq64Test, RetreatStepCursor_Page1_AtPageStart_WrapsToPageEnd)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps,
              0,
              16, // first step of page 1
              63,
              1);

  t.retreat_step_cursor();
  EXPECT_EQ(31, t.step_cursor());
}

// Toggle step cursor tests
TEST(TrigSeq64Test, ToggleStepCursor_FromFalse_BecomesTrue)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps,
              0,
              5,
              63,
              0);

  EXPECT_FALSE(t.steps()[5]);
  t.toggle_step_cursor();
  EXPECT_TRUE(t.steps()[5]);
}

TEST(TrigSeq64Test, ToggleStepCursor_FromTrue_BecomesFalse)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;
  steps.set(10);

  TrigSeq64 t(steps,
              0,
              10,
              63,
              0);

  EXPECT_TRUE(t.steps()[10]);
  t.toggle_step_cursor();
  EXPECT_FALSE(t.steps()[10]);
}

TEST(TrigSeq64Test, ToggleStepCursor_MultipleTimes_Alternates)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps,
              0,
              20,
              63,
              1);

  EXPECT_FALSE(t.steps()[20]);
  t.toggle_step_cursor();
  EXPECT_TRUE(t.steps()[20]);
  t.toggle_step_cursor();
  EXPECT_FALSE(t.steps()[20]);
  t.toggle_step_cursor();
  EXPECT_TRUE(t.steps()[20]);
}