#include <catch/catch.hpp>
#include <cmath>
#include <limits>
#include <RaPsCallion/serializer.h>

namespace rapscallion {
namespace test {

void test(const double in, const std::ptrdiff_t expected_size = -1) {
  Serializer s;
  serializer<decltype(+in)>::write(s, in);
  Deserializer d(s);

  const auto size = d.end - d.ptr;
  if (expected_size >= 0)
    CHECK(size == expected_size);

  // largest encoding of IEEE754 binary64 float:
  //  (11 exponent bits + exponent sign + NaN/inf flag) = 13 bits / 7 bit/byte = 2 byte
  //  (52 bit fraction + sign) = 53 bits / 7 bit/byte = 8 byte
  // + = 10 byte
  CHECK(size <= 10);

  const auto out = serializer<decltype(+in)>::read(d);
  if (std::isnan(in))
    CHECK(std::isnan(out));
  else
    CHECK(out == in);
}

}
}

TEST_CASE("Floats are serialized properly", "[serialization]") {
  using namespace rapscallion::test;

  // Smallest encodings, only requiring flags
  test(std::numeric_limits<double>::infinity(), 1);
  test(-std::numeric_limits<double>::infinity(), 1);
  test(std::numeric_limits<double>::quiet_NaN(), 1);
  test(0.0, 1);
  test(-0.0, 1);

  test(0.03125, 2);
  test(-0.03125, 2);
  test(0.0625, 2);
  test(-0.0625, 2);
  test(0.125, 2);
  test(-0.125, 2);
  test(0.25, 2);
  test(-0.25, 2);
  test(0.5, 2);
  test(-0.5, 2);
  test(1.0, 2);
  test(-1.0, 2);
  test(2.0, 2);
  test(-2.0, 2);
  test(M_PI, 9);
  test(-M_PI, 9);
  test((float)M_PI, 5);
  test(-(float)M_PI, 5);
  test(std::numeric_limits<double>::epsilon());
  test(std::numeric_limits<double>::min());
  test(std::numeric_limits<double>::max());
  test(std::numeric_limits<double>::denorm_min());
}
