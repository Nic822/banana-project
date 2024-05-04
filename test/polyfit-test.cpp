#include <gtest/gtest.h>

#include <polyfit/Polynomial2DFit.hpp>

#define ASSERT_COEFFS_NEAR(coeff_expected_0, coeff_expected_1, coeff_expected_2, coeffs) \
    do {                                                                                 \
        ASSERT_NEAR(coeff_expected_0, std::get<0>(coeffs), 1e-6);                        \
        ASSERT_NEAR(coeff_expected_1, std::get<1>(coeffs), 1e-6);                        \
        ASSERT_NEAR(coeff_expected_2, std::get<2>(coeffs), 1e-6);                        \
    } while (false)

/** y = 1 + x */
TEST(Polynomial2DFitTestSuite, FitSimpleLine) {
    std::vector<std::pair<double, double>> points = {
            {0,1},
            {1,2},
            {2,3},
            {3,4},
    };
    auto const result = polyfit::Fit2DPolynomial(points);
    ASSERT_TRUE(result);
    ASSERT_COEFFS_NEAR(1, 1, 0, *result);
}

/** y = -1 + x^2 */
TEST(Polynomial2DFitTestSuite, FitSimpleCurve) {
    std::vector<std::pair<double, double>> points = {
            {-1,0},
            {0,-1},
            {1,0},
    };
    auto const result = polyfit::Fit2DPolynomial(points);
    ASSERT_TRUE(result);
    ASSERT_COEFFS_NEAR(-1, 0, 1, *result);
}

/** y = -1 + 3*x + 2*x^2 */
TEST(Polynomial2DFitTestSuite, FitSimpleCurve2) {
    std::vector<std::pair<double, double>> points = {
            {-1,-2},
            {0,-1},
            {1,4},
    };
    auto const result = polyfit::Fit2DPolynomial(points);
    ASSERT_TRUE(result);
    ASSERT_COEFFS_NEAR(-1, 3, 2, *result);
}
