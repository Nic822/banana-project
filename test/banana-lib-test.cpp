#include <filesystem>
#include <string>

#include <gtest/gtest.h>

#include "banana-lib/lib.hpp"

/// Assert that two matrices are identical (same values  for all pixels).
#define ASSERT_SAME_MAT(a,b) ASSERT_EQ(cv::Scalar(), cv::sum(a != b))

TEST(GeneralBananaTestSuite, FailOnNonExistingImage) {
    banana::Analyzer const analyzer;
    auto const image = cv::imread("non-existent-image.jpg");
    auto const result = analyzer.AnalyzeImage(image);
    ASSERT_FALSE(result.has_value());
}

TEST(BananaContourFinderTestSuite, AnalyzeEmptyPicture) {
    banana::Analyzer const analyzer;
    auto const image = cv::imread("resources/test-images/empty.jpg");
    auto const result = analyzer.AnalyzeImage(image);
    ASSERT_TRUE(result.has_value());
    auto const& results = result.value();
    ASSERT_EQ(0, results.size());
}

TEST(BananaContourFinderTestSuite, AnalyzeAndAnnotateEmptyPicture) {
    banana::Analyzer const analyzer;
    auto const image = cv::imread("resources/test-images/empty.jpg");
    auto const result = analyzer.AnalyzeAndAnnotateImage(image);
    ASSERT_TRUE(result.has_value());
    auto const& analysis_result = result.value();
    ASSERT_EQ(0, analysis_result.banana.size());
    ASSERT_SAME_MAT(analysis_result.annotated_image, image);
}

TEST(BananaContourFinderTestSuite, FindSingleBanana) {
    GTEST_SKIP(); // TODO: implement test once contour detection is implemented
}
