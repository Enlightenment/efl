#include <gtest/gtest.h>
#include"sgdebug.h"

int main(int argc, char **argv) {
    initialize(GuaranteedLogger(), "/tmp/", "ssglog", 1);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
