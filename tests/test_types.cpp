#include "core/types.hpp"
#include "test_framework.hpp"

int main()
{
    using namespace qualix;
    using namespace qualix::test;

    Expect(sizeof(i8)  == 1, "sizeof(i8)");
    Expect(sizeof(u8)  == 1, "sizeof(u8)");

    Expect(sizeof(i16) == 2, "sizeof(i16)");
    Expect(sizeof(u16) == 2, "sizeof(u16)");

    Expect(sizeof(i32) == 4, "sizeof(i32)");
    Expect(sizeof(u32) == 4, "sizeof(u32)");

    Expect(sizeof(i64) == 8, "sizeof(i64)");
    Expect(sizeof(u64) == 8, "sizeof(u64)");

    u32 value = 42;
    Expect(value == 42, "u32 assignment");

    return Summary();
}
