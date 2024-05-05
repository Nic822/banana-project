#include <filesystem>
#include <string>

#include <gtest/gtest.h>

#include <banana-lib/lib.hpp>

#include "polyfit-test-util.hpp"

/// Assert that two matrices are identical (same values  for all pixels).
#define ASSERT_SAME_MAT(a,b) ASSERT_EQ(cv::Scalar(), cv::sum(a != b))

TEST(GeneralBananaTestSuite, FailOnNonExistingImage) {
    banana::Analyzer const analyzer;
    auto const image = cv::imread("non-existent-image.jpg");
    auto const result = analyzer.AnalyzeImage(image);
    ASSERT_FALSE(result);
}

TEST(BananaContourFinderTestSuite, AnalyzeEmptyPicture) {
    banana::Analyzer const analyzer;
    auto const image = cv::imread("resources/test-images/empty.jpg");
    auto const result = analyzer.AnalyzeImage(image);
    ASSERT_TRUE(result);
    ASSERT_EQ(0, (*result).size());
}

TEST(BananaContourFinderTestSuite, AnalyzeAndAnnotateEmptyPicture) {
    banana::Analyzer const analyzer;
    auto const image = cv::imread("resources/test-images/empty.jpg");
    auto const result = analyzer.AnalyzeAndAnnotateImage(image);
    ASSERT_TRUE(result);
    ASSERT_EQ(0, (*result).banana.size());
    ASSERT_SAME_MAT((*result).annotated_image, image);
}

TEST(BananaContourFinderTestSuite, FindSingleBanana00) {
    banana::Analyzer const analyzer;
    auto const image = cv::imread("resources/test-images/banana-00.jpg");
    auto const result = analyzer.AnalyzeAndAnnotateImage(image);
    ASSERT_TRUE(result);
    ASSERT_EQ(1, (*result).banana.size());
    // TODO: use ASSERT_SAME_MAT with a reference annotated image
}

TEST(BananaContourFinderTestSuite, FindTwoBananas) {
    banana::Analyzer const analyzer;
    auto const image = cv::imread("resources/test-images/banana-22.jpg");
    auto const result = analyzer.AnalyzeAndAnnotateImage(image);
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(2, (*result).banana.size());
    // TODO: use ASSERT_SAME_MAT with a reference annotated image
}

TEST(CenterLineCoefficientsTestSuite, SingleBanana00) {
    banana::Analyzer const analyzer;
    auto const image = cv::imread("resources/test-images/banana-00.jpg");
    auto const result = analyzer.AnalyzeAndAnnotateImage(image);
    ASSERT_TRUE(result);
    ASSERT_EQ(1, (*result).banana.size());

    ASSERT_COEFFS_NEAR(2536.0389294, -1.8237497, 0.0005237, (*result).banana.front().center_line_coefficients);
}
