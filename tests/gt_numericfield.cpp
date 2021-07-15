#include <gtest/gtest.h>
#include <amqpcpp.h>

namespace {

TEST(NumericField, implicitCast)
{
  int64_t i64 = AMQP::Float(23);
  ASSERT_EQ(i64, 23);
  float f = AMQP::Long(2);
  ASSERT_EQ(f, 2.0);
}

TEST(NumericField, isInteger)
{
  ASSERT_FALSE(AMQP::Float(1).isInteger());
  ASSERT_TRUE(AMQP::Long(1).isInteger());
}

} // ns anonymous
