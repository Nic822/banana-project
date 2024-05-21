#ifndef BANANA_PROJECT_POLYFIT_TEST_UTIL_HPP
#define BANANA_PROJECT_POLYFIT_TEST_UTIL_HPP

#define ASSERT_COEFFS_NEAR(coeff_expected_0, coeff_expected_1, coeff_expected_2, coeffs) \
    do {                                                                                 \
        auto const& [coeff_0, coeff_1, coeff_2] = coeffs;                                \
        ASSERT_NEAR(coeff_expected_0, coeff_0, 1e-6);                                    \
        ASSERT_NEAR(coeff_expected_1, coeff_1, 1e-6);                                    \
        ASSERT_NEAR(coeff_expected_2, coeff_2, 1e-6);                                    \
    } while (false)

#endif //BANANA_PROJECT_POLYFIT_TEST_UTIL_HPP
