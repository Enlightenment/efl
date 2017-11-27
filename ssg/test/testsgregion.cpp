#include <gtest/gtest.h>
#include"sgregion.h"
#include"sgdebug.h"
#include"sgpoint.h"
class SGRegionTest : public ::testing::Test {
public:
    SGRegionTest():rgn1(-10, -10, 20, 20)
    {
    }
    void SetUp()
    {
        rect1 = SGRect(-10, -10, 20, 20);
        rect2 = SGRect(-15, 5, 10, 10);
        rgn2 += rect2;
        rgn3 = rgn1;
    }
    void TearDown()
    {

    }
public:
    SGRegion emptyRgn;
    SGRegion rgn1;
    SGRegion rgn2;
    SGRegion rgn3;
    SGRect rect1;
    SGRect rect2;
    SGRect rect3;

};

TEST_F(SGRegionTest, constructor) {
    ASSERT_EQ(rgn1.rectCount() , 1);
    ASSERT_TRUE(rgn1.rectAt(0) == rect1);
    ASSERT_TRUE(rgn1==rgn3);
    ASSERT_TRUE(rgn1!=rgn2);
}

TEST_F(SGRegionTest, moveSemantics) {
    // move assignment

    rgn1 = rect1;
    SGRegion tmp;
    tmp = std::move(rgn1);
    ASSERT_TRUE(rgn1.isEmpty());

    // move construction
    rgn1 = rect1;
    SGRegion mvrgn = std::move(rgn1);
    ASSERT_TRUE(rgn1.isEmpty());
    ASSERT_TRUE(mvrgn == rect1);
}
TEST_F(SGRegionTest, isEmpty) {
    ASSERT_TRUE(emptyRgn.isEmpty());
    ASSERT_TRUE(emptyRgn == SGRegion());
    ASSERT_TRUE(emptyRgn.rectCount() == 0);
    ASSERT_TRUE(emptyRgn.boundingRect() == SGRect());
}

TEST_F(SGRegionTest, boundingRect) {
    {
        SGRect rect;
        SGRegion region(rect);
        ASSERT_TRUE(region.boundingRect() == rect);
    }
    {
        SGRect rect(10, -20, 30, 40);
        SGRegion region(rect);
        ASSERT_TRUE(region.boundingRect() == rect);
    }
    {
        SGRect rect(15,25,10,10);
        SGRegion region(rect);
        ASSERT_TRUE(region.boundingRect() == rect);
    }
}

TEST_F(SGRegionTest, swap) {
    SGRegion r1(SGRect(0, 0,10,10));
    SGRegion r2(SGRect(10,10,10,10));
    std::swap(r1 ,r2);
    ASSERT_TRUE(r1.rectAt(0) == SGRect(10,10,10,10));
    ASSERT_TRUE(r2.rectAt(0) == SGRect(0, 0,10,10));
}

TEST_F(SGRegionTest, substracted) {
    SGRegion r1(SGRect(0, 0,20,20));
    SGRegion r2 = r1.subtracted(SGRect(5,5,5,5));
    SGRegion expected;
    expected += SGRect(0,0,20,5);
    expected += SGRect(0,5,5,5);
    expected += SGRect(10,5,10,5);
    expected += SGRect(0,10,20,10);
    ASSERT_TRUE(r2.rectCount() == expected.rectCount());
    ASSERT_TRUE(r2 == expected);
    r2 += SGRect(5,5,5,5);
    ASSERT_TRUE(r2 == r1);
}

TEST_F(SGRegionTest, translate) {
    SGRegion r1(SGRect(0, 0,20,20));
    SGPoint offset(10,10);
    SGRegion r2 =  r1.translated(offset);
    r1.translate(offset);
    ASSERT_TRUE(r2 == r2);
}

TEST_F(SGRegionTest, intersects) {
    SGRegion r1(SGRect(0, 0,20,20));
    SGRegion r2(SGRect(20, 20,10,10));
    ASSERT_FALSE(r1.intersects(r2));
    r2 += SGRect(5, 0,20,20);
    ASSERT_TRUE(r1.intersects(r2));
}

TEST_F(SGRegionTest, contains) {
    SGRegion r1(SGRect(0, 0,20,20));
    ASSERT_TRUE(r1.contains(SGRect(5,5,10,10)));
    ASSERT_FALSE(r1.contains(SGRect(11,5,10,10)));
}
