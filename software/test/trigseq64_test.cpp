#include "gtest/gtest.h"
#include "TrigSeq64.h"

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
              0, // start
              63, // end
              1); // page

  t.advance_page();
  EXPECT_EQ(2, t.page_cursor());
}

TEST(TrigSeq64Test, AdvancePage_AtLastPage_WrapsToFirstPage)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps,
              0,
              0,
              0,
              63,
              TrigSeq64::PAGE_COUNT - 1);

  t.advance_page();
  EXPECT_EQ(0, t.page_cursor());
}

TEST(TrigSeq64Test, RetreatPage_FromMiddle_Decrements)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps,
              0,
              0,
               0,
              63,
              2);

  t.retreat_page();
  EXPECT_EQ(1, t.page_cursor());
}

TEST(TrigSeq64Test, RetreatPage_AtFirstPage_WrapsToLastPage)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps,
              0,
              0,
              0,
              63,
              0);

  t.retreat_page();
  EXPECT_EQ(TrigSeq64::PAGE_COUNT - 1, t.page_cursor());
}

// End cursor navigation tests
TEST(TrigSeq64Test, AdvanceEndCursor_FromMiddle_Increments)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps,
              0,
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
              0,
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
              0,
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
              0,
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
               0,
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
              0,
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
              0,
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
               0,
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
               0,
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
               0,
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

// Page manipulation tests
TEST(TrigSeq64Test, ClearPage_Page0_ClearsOnlyPage0)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;
  steps.set(); // all bits true

  TrigSeq64 t(steps,
              0,
              0,
               0,
              63,
              0);

  t.clear_page();
  
  for (int i = 0; i < 16; ++i) {
    EXPECT_FALSE(t.steps()[i]) << "Step " << i << " in page 0 should be cleared";
  }
  for (int i = 16; i < 64; ++i) {
    EXPECT_TRUE(t.steps()[i]) << "Step " << i << " outside page 0 should remain set";
  }
}

TEST(TrigSeq64Test, ClearPage_Page2_ClearsOnlyPage2)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;
  steps.set();

  TrigSeq64 t(steps,
              0,
              0,
               0,
              63,
              2);

  t.clear_page();
  
  for (int i = 0; i < 32; ++i) {
    EXPECT_TRUE(t.steps()[i]) << "Step " << i << " before page 2 should remain set";
  }
  for (int i = 32; i < 48; ++i) {
    EXPECT_FALSE(t.steps()[i]) << "Step " << i << " in page 2 should be cleared";
  }
  for (int i = 48; i < 64; ++i) {
    EXPECT_TRUE(t.steps()[i]) << "Step " << i << " after page 2 should remain set";
  }
}

TEST(TrigSeq64Test, FillPage_Page1_FillsOnlyPage1)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps; // all false

  TrigSeq64 t(steps,
              0,
              0,
               0,
              63,
              1);

  t.fill_page();
  
  for (int i = 0; i < 16; ++i) {
    EXPECT_FALSE(t.steps()[i]) << "Step " << i << " before page 1 should remain clear";
  }
  for (int i = 16; i < 32; ++i) {
    EXPECT_TRUE(t.steps()[i]) << "Step " << i << " in page 1 should be set";
  }
  for (int i = 32; i < 64; ++i) {
    EXPECT_FALSE(t.steps()[i]) << "Step " << i << " after page 1 should remain clear";
  }
}

TEST(TrigSeq64Test, FillPage_Page3_FillsOnlyPage3)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps,
              0,
              0,
               0,
              63,
              3);

  t.fill_page();
  
  for (int i = 0; i < 48; ++i) {
    EXPECT_FALSE(t.steps()[i]) << "Step " << i << " before page 3 should remain clear";
  }
  for (int i = 48; i < 64; ++i) {
    EXPECT_TRUE(t.steps()[i]) << "Step " << i << " in page 3 should be set";
  }
}

// Page getter tests
TEST(TrigSeq64Test, GetStepCursorPage_VariousPositions_ReturnsCorrectPage)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t0(steps, 0, 0,  0, 63, 0);
  EXPECT_EQ(0, t0.get_step_cursor_page());

  TrigSeq64 t1(steps, 0, 15,  0, 63, 0);
  EXPECT_EQ(0, t1.get_step_cursor_page());

  TrigSeq64 t2(steps, 0, 16,  0, 63, 1);
  EXPECT_EQ(1, t2.get_step_cursor_page());

  TrigSeq64 t3(steps, 0, 31,  0, 63, 1);
  EXPECT_EQ(1, t3.get_step_cursor_page());

  TrigSeq64 t4(steps, 0, 32,  0, 63, 2);
  EXPECT_EQ(2, t4.get_step_cursor_page());

  TrigSeq64 t5(steps, 0, 47,  0, 63, 2);
  EXPECT_EQ(2, t5.get_step_cursor_page());

  TrigSeq64 t6(steps, 0, 48,  0, 63, 3);
  EXPECT_EQ(3, t6.get_step_cursor_page());

  TrigSeq64 t7(steps, 0, 63,  0, 63, 3);
  EXPECT_EQ(3, t7.get_step_cursor_page());
}

TEST(TrigSeq64Test, GetPlayheadPage_VariousPositions_ReturnsCorrectPage)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t0(steps, 0, 0,  0, 63, 0);
  EXPECT_EQ(0, t0.get_playhead_page());

  TrigSeq64 t1(steps, 15, 0,  0, 63, 0);
  EXPECT_EQ(0, t1.get_playhead_page());

  TrigSeq64 t2(steps, 16, 0,  0, 63, 0);
  EXPECT_EQ(1, t2.get_playhead_page());

  TrigSeq64 t3(steps, 31, 0,  0, 63, 1);
  EXPECT_EQ(1, t3.get_playhead_page());

  TrigSeq64 t4(steps, 32, 0,  0, 63, 2);
  EXPECT_EQ(2, t4.get_playhead_page());

  TrigSeq64 t5(steps, 47, 0,  0, 63, 2);
  EXPECT_EQ(2, t5.get_playhead_page());

  TrigSeq64 t6(steps, 48, 0,  0, 63, 3);
  EXPECT_EQ(3, t6.get_playhead_page());

  TrigSeq64 t7(steps, 63, 0,  0, 63, 3);
  EXPECT_EQ(3, t7.get_playhead_page());
}

// Probability initialization tests
TEST(TrigSeq64Test, ProbabilityInitialization_AllStepsStart100Percent)
{
  TrigSeq64 t;

  for (size_t i = 0; i < TrigSeq64::MAX_STEPS; ++i) {
    EXPECT_FLOAT_EQ(1.0f, t.get_step_probability(i)) << "Step " << i << " should start at 100% probability";
  }
}

TEST(TrigSeq64Test, ProbabilityInitialization_WithConstructor_AllStepsStart100Percent)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;
  steps.set(5);
  steps.set(10);

  TrigSeq64 t(steps, 0, 0,  0, 63, 0);

  for (size_t i = 0; i < TrigSeq64::MAX_STEPS; ++i) {
    EXPECT_FLOAT_EQ(1.0f, t.get_step_probability(i)) << "Step " << i << " should start at 100% probability";
  }
}

TEST(TrigSeq64Test, ProbabilityInitialization_WithProbabilitiesConstructor_PreservesValues)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;
  std::array<float, TrigSeq64::MAX_STEPS> probs;
  probs.fill(0.5f);
  probs[10] = 0.7f;
  probs[20] = 0.3f;

  TrigSeq64 t(steps, 0, 0,  0, 63, 0, probs);

  EXPECT_FLOAT_EQ(0.7f, t.get_step_probability(10));
  EXPECT_FLOAT_EQ(0.3f, t.get_step_probability(20));
  EXPECT_FLOAT_EQ(0.5f, t.get_step_probability(0));
  EXPECT_FLOAT_EQ(0.5f, t.get_step_probability(63));
}

// Probability getter tests
TEST(TrigSeq64Test, GetStepCursorProbability_ReturnsCorrectValue)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;
  std::array<float, TrigSeq64::MAX_STEPS> probs;
  probs.fill(1.0f);
  probs[5] = 0.6f;

  TrigSeq64 t(steps, 0, 5,  0, 63, 0, probs);

  EXPECT_FLOAT_EQ(0.6f, t.get_step_cursor_probability());
}

TEST(TrigSeq64Test, GetStepProbability_VariousSteps_ReturnsCorrectValues)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;
  std::array<float, TrigSeq64::MAX_STEPS> probs;
  probs.fill(1.0f);
  probs[0] = 0.1f;
  probs[15] = 0.5f;
  probs[32] = 0.9f;
  probs[63] = 0.2f;

  TrigSeq64 t(steps, 0, 0,  0, 63, 0, probs);

  EXPECT_FLOAT_EQ(0.1f, t.get_step_probability(0));
  EXPECT_FLOAT_EQ(0.5f, t.get_step_probability(15));
  EXPECT_FLOAT_EQ(0.9f, t.get_step_probability(32));
  EXPECT_FLOAT_EQ(0.2f, t.get_step_probability(63));
}

// Increase probability tests
TEST(TrigSeq64Test, IncreaseProbability_From100_StaysAt100)
{
  TrigSeq64 t;  // All steps start at 100%

  t.increase_step_cursor_probability();
  EXPECT_FLOAT_EQ(1.0f, t.get_step_cursor_probability());
}

TEST(TrigSeq64Test, IncreaseProbability_From50_GoesTo60)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;
  std::array<float, TrigSeq64::MAX_STEPS> probs;
  probs.fill(1.0f);
  probs[0] = 0.5f;

  TrigSeq64 t(steps, 0, 0,  0, 63, 0, probs);

  t.increase_step_cursor_probability();
  EXPECT_FLOAT_EQ(0.6f, t.get_step_cursor_probability());
}

TEST(TrigSeq64Test, IncreaseProbability_From90_GoesTo100)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;
  std::array<float, TrigSeq64::MAX_STEPS> probs;
  probs.fill(1.0f);
  probs[10] = 0.9f;

  TrigSeq64 t(steps, 0, 10,  0, 63, 0, probs);

  t.increase_step_cursor_probability();
  EXPECT_FLOAT_EQ(1.0f, t.get_step_cursor_probability());
}

TEST(TrigSeq64Test, IncreaseProbability_Multiple_IncrementsBy10Each)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;
  std::array<float, TrigSeq64::MAX_STEPS> probs;
  probs.fill(1.0f);
  probs[5] = 0.7f;

  TrigSeq64 t(steps, 0, 5,  0, 63, 0, probs);

  t.increase_step_cursor_probability();
  EXPECT_FLOAT_EQ(0.8f, t.get_step_cursor_probability());
  
  t.increase_step_cursor_probability();
  EXPECT_FLOAT_EQ(0.9f, t.get_step_cursor_probability());
  
  t.increase_step_cursor_probability();
  EXPECT_FLOAT_EQ(1.0f, t.get_step_cursor_probability());
  
  t.increase_step_cursor_probability();
  EXPECT_FLOAT_EQ(1.0f, t.get_step_cursor_probability());  // Stays at max
}

TEST(TrigSeq64Test, IncreaseProbability_OnlyAffectsStepCursor)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;
  std::array<float, TrigSeq64::MAX_STEPS> probs;
  probs.fill(0.5f);

  TrigSeq64 t(steps, 0, 10,  0, 63, 0, probs);

  t.increase_step_cursor_probability();
  
  EXPECT_FLOAT_EQ(0.6f, t.get_step_probability(10));  // Step cursor changed
  EXPECT_FLOAT_EQ(0.5f, t.get_step_probability(9));   // Others unchanged
  EXPECT_FLOAT_EQ(0.5f, t.get_step_probability(11));
  EXPECT_FLOAT_EQ(0.5f, t.get_step_probability(0));
  EXPECT_FLOAT_EQ(0.5f, t.get_step_probability(63));
}

// Decrease probability tests
TEST(TrigSeq64Test, DecreaseProbability_From100_GoesTo90)
{
  TrigSeq64 t;  // All steps start at 100%

  t.decrease_step_cursor_probability();
  EXPECT_FLOAT_EQ(0.9f, t.get_step_cursor_probability());
}

TEST(TrigSeq64Test, DecreaseProbability_From50_GoesTo40)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;
  std::array<float, TrigSeq64::MAX_STEPS> probs;
  probs.fill(1.0f);
  probs[0] = 0.5f;

  TrigSeq64 t(steps, 0, 0,  0, 63, 0, probs);

  t.decrease_step_cursor_probability();
  EXPECT_FLOAT_EQ(0.4f, t.get_step_cursor_probability());
}

TEST(TrigSeq64Test, DecreaseProbability_From20_GoesTo10)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;
  std::array<float, TrigSeq64::MAX_STEPS> probs;
  probs.fill(1.0f);
  probs[20] = 0.2f;

  TrigSeq64 t(steps, 0, 20,  0, 63, 0, probs);

  t.decrease_step_cursor_probability();
  EXPECT_FLOAT_EQ(0.1f, t.get_step_cursor_probability());
}

TEST(TrigSeq64Test, DecreaseProbability_From10_StaysAt10)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;
  std::array<float, TrigSeq64::MAX_STEPS> probs;
  probs.fill(1.0f);
  probs[0] = 0.1f;

  TrigSeq64 t(steps, 0, 0,  0, 63, 0, probs);

  t.decrease_step_cursor_probability();
  EXPECT_FLOAT_EQ(0.1f, t.get_step_cursor_probability());  // Stays at minimum
}

TEST(TrigSeq64Test, DecreaseProbability_Multiple_DecrementsBy10Each)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;
  std::array<float, TrigSeq64::MAX_STEPS> probs;
  probs.fill(1.0f);
  probs[15] = 0.4f;

  TrigSeq64 t(steps, 0, 15,  0, 63, 0, probs);

  t.decrease_step_cursor_probability();
  EXPECT_FLOAT_EQ(0.3f, t.get_step_cursor_probability());
  
  t.decrease_step_cursor_probability();
  EXPECT_FLOAT_EQ(0.2f, t.get_step_cursor_probability());
  
  t.decrease_step_cursor_probability();
  EXPECT_FLOAT_EQ(0.1f, t.get_step_cursor_probability());
  
  t.decrease_step_cursor_probability();
  EXPECT_FLOAT_EQ(0.1f, t.get_step_cursor_probability());  // Stays at min
}

TEST(TrigSeq64Test, DecreaseProbability_OnlyAffectsStepCursor)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;
  std::array<float, TrigSeq64::MAX_STEPS> probs;
  probs.fill(0.8f);

  TrigSeq64 t(steps, 0, 25,  0, 63, 0, probs);

  t.decrease_step_cursor_probability();
  
  EXPECT_FLOAT_EQ(0.7f, t.get_step_probability(25));  // Step cursor changed
  EXPECT_FLOAT_EQ(0.8f, t.get_step_probability(24));  // Others unchanged
  EXPECT_FLOAT_EQ(0.8f, t.get_step_probability(26));
  EXPECT_FLOAT_EQ(0.8f, t.get_step_probability(0));
  EXPECT_FLOAT_EQ(0.8f, t.get_step_probability(63));
}

// Combined increase/decrease tests
TEST(TrigSeq64Test, IncreaseDecreaseProbability_RoundTrip_ReturnsToOriginal)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;
  std::array<float, TrigSeq64::MAX_STEPS> probs;
  probs.fill(1.0f);
  probs[5] = 0.5f;

  TrigSeq64 t(steps, 0, 5,  0, 63, 0, probs);

  t.increase_step_cursor_probability();
  EXPECT_FLOAT_EQ(0.6f, t.get_step_cursor_probability());
  
  t.increase_step_cursor_probability();
  EXPECT_FLOAT_EQ(0.7f, t.get_step_cursor_probability());
  
  t.decrease_step_cursor_probability();
  EXPECT_FLOAT_EQ(0.6f, t.get_step_cursor_probability());
  
  t.decrease_step_cursor_probability();
  EXPECT_FLOAT_EQ(0.5f, t.get_step_cursor_probability());
}

TEST(TrigSeq64Test, Probability_Constants_HaveCorrectValues)
{
  EXPECT_FLOAT_EQ(0.1f, TrigSeq64::min_probability());
  EXPECT_FLOAT_EQ(1.0f, TrigSeq64::max_probability());
  EXPECT_FLOAT_EQ(0.1f, TrigSeq64::probability_increment());
}

// Start cursor tests
TEST(TrigSeq64Test, InitialState_StartCursorDefaultsToZero)
{
  TrigSeq64 t;
  EXPECT_EQ(0, t.start_cursor());
}

TEST(TrigSeq64Test, StartCursorConstructor_PreservesValue)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps, 0, 0, 5, 15, 0);
  EXPECT_EQ(5, t.start_cursor());
}

TEST(TrigSeq64Test, GetStartCursorPage_VariousPositions_ReturnsCorrectPage)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t0(steps, 0, 0, 0, 63, 0);
  EXPECT_EQ(0, t0.get_start_cursor_page());

  TrigSeq64 t1(steps, 0, 0, 15, 63, 0);
  EXPECT_EQ(0, t1.get_start_cursor_page());

  TrigSeq64 t2(steps, 0, 0, 16, 63, 1);
  EXPECT_EQ(1, t2.get_start_cursor_page());

  TrigSeq64 t3(steps, 0, 0, 31, 63, 1);
  EXPECT_EQ(1, t3.get_start_cursor_page());

  TrigSeq64 t4(steps, 0, 0, 32, 63, 2);
  EXPECT_EQ(2, t4.get_start_cursor_page());

  TrigSeq64 t5(steps, 0, 0, 47, 63, 2);
  EXPECT_EQ(2, t5.get_start_cursor_page());

  TrigSeq64 t6(steps, 0, 0, 48, 63, 3);
  EXPECT_EQ(3, t6.get_start_cursor_page());

  TrigSeq64 t7(steps, 0, 0, 63, 63, 3);
  EXPECT_EQ(3, t7.get_start_cursor_page());
}

TEST(TrigSeq64Test, AdvanceStartCursor_WithinBounds_Increments)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps, 0, 0, 5, 20, 0);

  t.advance_start_cursor();
  EXPECT_EQ(6, t.start_cursor());
}

TEST(TrigSeq64Test, AdvanceStartCursor_AtEndCursor_DoesNothing)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps, 0, 0, 15, 15, 0);

  t.advance_start_cursor();
  EXPECT_EQ(15, t.start_cursor());
}

TEST(TrigSeq64Test, AdvanceStartCursor_OneBeforeEnd_GoesToEnd)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps, 0, 0, 14, 15, 0);

  t.advance_start_cursor();
  EXPECT_EQ(15, t.start_cursor());
}

TEST(TrigSeq64Test, AdvanceStartCursor_CrossesPageBoundary_UpdatesPage)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps, 0, 0, 15, 63, 0);

  t.advance_start_cursor();
  EXPECT_EQ(16, t.start_cursor());
  EXPECT_EQ(1, t.page_cursor());
}

TEST(TrigSeq64Test, RetreatStartCursor_FromMiddle_Decrements)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps, 0, 0, 10, 63, 0);

  t.retreat_start_cursor();
  EXPECT_EQ(9, t.start_cursor());
}

TEST(TrigSeq64Test, RetreatStartCursor_AtZero_DoesNothing)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps, 0, 0, 0, 63, 0);

  t.retreat_start_cursor();
  EXPECT_EQ(0, t.start_cursor());
}

TEST(TrigSeq64Test, RetreatStartCursor_CrossesPageBoundary_UpdatesPage)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps, 0, 0, 16, 63, 1);

  t.retreat_start_cursor();
  EXPECT_EQ(15, t.start_cursor());
  EXPECT_EQ(0, t.page_cursor());
}

TEST(TrigSeq64Test, ResetStartCursorToPageStart_Page0_SetsToZero)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps, 0, 0, 10, 63, 0);

  t.reset_start_cursor_to_page_start();
  EXPECT_EQ(0, t.start_cursor());
}

TEST(TrigSeq64Test, ResetStartCursorToPageStart_Page1_SetsTo16)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps, 0, 0, 20, 63, 1);

  t.reset_start_cursor_to_page_start();
  EXPECT_EQ(16, t.start_cursor());
}

TEST(TrigSeq64Test, ResetStartCursorToPageStart_Page2_SetsTo32)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps, 0, 0, 40, 63, 2);

  t.reset_start_cursor_to_page_start();
  EXPECT_EQ(32, t.start_cursor());
}

TEST(TrigSeq64Test, ResetStartCursorToPageStart_Page3_SetsTo48)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps, 0, 0, 50, 63, 3);

  t.reset_start_cursor_to_page_start();
  EXPECT_EQ(48, t.start_cursor());
}

TEST(TrigSeq64Test, StartCursor_MultipleAdvances_StaysWithinEndBounds)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps, 0, 0, 5, 10, 0);

  t.advance_start_cursor();
  EXPECT_EQ(6, t.start_cursor());
  
  t.advance_start_cursor();
  EXPECT_EQ(7, t.start_cursor());
  
  t.advance_start_cursor();
  EXPECT_EQ(8, t.start_cursor());
  
  t.advance_start_cursor();
  EXPECT_EQ(9, t.start_cursor());
  
  t.advance_start_cursor();
  EXPECT_EQ(10, t.start_cursor());
  
  t.advance_start_cursor();
  EXPECT_EQ(10, t.start_cursor());  // Stays at end_cursor
}

TEST(TrigSeq64Test, StartCursor_MultipleRetreats_StaysAboveZero)
{
  std::bitset<TrigSeq64::MAX_STEPS> steps;

  TrigSeq64 t(steps, 0, 0, 3, 63, 0);

  t.retreat_start_cursor();
  EXPECT_EQ(2, t.start_cursor());
  
  t.retreat_start_cursor();
  EXPECT_EQ(1, t.start_cursor());
  
  t.retreat_start_cursor();
  EXPECT_EQ(0, t.start_cursor());
  
  t.retreat_start_cursor();
  EXPECT_EQ(0, t.start_cursor());  // Stays at 0
}