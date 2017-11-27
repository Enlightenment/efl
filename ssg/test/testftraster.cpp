#include <gtest/gtest.h>
#include "sgpath.h"
#include "ftraster_p.h"

class FTRasterTest : public ::testing::Test {
public:
    void SetUp()
    {
        path.moveTo(SGPointF(0,0));
        path.lineTo(SGPointF(10,0));
        path.lineTo(SGPointF(10,10));
        path.lineTo(SGPointF(0,10));
        path.close();
        pathRect = SGRect(0,0,10,10);
    }
    void TearDown()
    {

    }
public:
  SGPath path;
  SGRect pathRect;
};


TEST_F(FTRasterTest, constructor) {
    FTOutline *outline = FTRaster::toFTOutline(path);
    ASSERT_TRUE(outline != nullptr);
    FTRaster::deleteFTOutline(outline);
}
