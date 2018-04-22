#include <gtest/gtest.h>
#include <amqpcpp.h>

namespace {

TEST(Address, Basics)
{
  AMQP::Address addr("amqp://user:passwd@server/vhost");
  ASSERT_EQ(addr.port(), 5672);
  ASSERT_FALSE(addr.secure());
  ASSERT_STREQ(addr.hostname().c_str(), "server");
  ASSERT_STREQ(addr.vhost().c_str(), "vhost");
  ASSERT_STREQ(addr.login().user().c_str(), "user");
  ASSERT_STREQ(addr.login().password().c_str(), "passwd");

  addr = AMQP::Address("amqps://user:passwd@server/vhost");
  ASSERT_TRUE(addr.secure());
  ASSERT_EQ(addr.port(), 5671);
}

} // ns anonymous
