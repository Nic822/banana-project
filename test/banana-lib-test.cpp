#include <filesystem>
#include <string>

#include <gtest/gtest.h>

#include <banana-lib/lib.hpp>

#include "polyfit-test-util.hpp"

/// Assert that two matrices are identical (same values  for all pixels).
#define ASSERT_SAME_MAT(a,b) ASSERT_EQ(cv::Scalar(), cv::sum(a != b))

#define GET_RESULT(path, num_expected)                            \
    banana::Analyzer const analyzer{{                             \
        .pixels_per_meter = 1,                                    \
    }};                                                           \
    auto const image = cv::imread(path);                          \
    auto const result_ = analyzer.AnalyzeAndAnnotateImage(image); \
    ASSERT_TRUE(result_);                                         \
    auto const& result = *result_;                                \
    ASSERT_EQ(num_expected, result.banana.size());                \
    do {} while(false)

TEST(GeneralBananaTestSuite, FailOnNonExistingImage) {
    banana::Analyzer const analyzer{{
        .pixels_per_meter = 1,
    }};
    auto const image = cv::imread("non-existent-image.jpg");
    auto const result = analyzer.AnalyzeImage(image);
    ASSERT_FALSE(result);
}

TEST(BananaContourFinderTestSuite, AnalyzeEmptyPicture) {
    banana::Analyzer const analyzer{{
        .pixels_per_meter = 1,
    }};
    auto const image = cv::imread("resources/test-images/empty.jpg");
    auto const result = analyzer.AnalyzeImage(image);
    ASSERT_TRUE(result);
    ASSERT_EQ(0, (*result).size());
}

TEST(BananaContourFinderTestSuite, AnalyzeAndAnnotateEmptyPicture) {
    GET_RESULT("resources/test-images/empty.jpg", 0);
    ASSERT_SAME_MAT(result.annotated_image, image);
}

TEST(BananaContourFinderTestSuite, FindSingleBanana00) {
    GET_RESULT("resources/test-images/banana-00.jpg", 1);
    // TODO: use ASSERT_SAME_MAT with a reference annotated image
}

TEST(BananaContourFinderTestSuite, FindTwoBananas) {
    GET_RESULT("resources/test-images/banana-22.jpg", 2);
    // TODO: use ASSERT_SAME_MAT with a reference annotated image
}

TEST(CenterLineCoefficientsTestSuite, SingleBanana00) {
    GET_RESULT("resources/test-images/banana-00.jpg", 1);
    ASSERT_COEFFS_NEAR(2482.2342194, -1.8133667, 0.0005347, result.banana.front().center_line.coefficients);
}

TEST(PCATestSuite, SingleBanana00) {
    GET_RESULT("resources/test-images/banana-00.jpg", 1);
    ASSERT_NEAR(-0.0484120, result.banana.front().rotation_angle, 1e-6);
}
