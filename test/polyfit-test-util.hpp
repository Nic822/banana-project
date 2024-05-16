#ifndef BANANA_PROJECT_POLYFIT_TEST_UTIL_HPP
#define BANANA_PROJECT_POLYFIT_TEST_UTIL_HPP

#define ASSERT_COEFFS_NEAR(coeff_expected_0, coeff_expected_1, coeff_expected_2, coeffs) \
    do {                                                                                 \
        ASSERT_NEAR(coeff_expected_0, std::get<0>(coeffs), 1e-6);                        \
        ASSERT_NEAR(coeff_expected_1, std::get<1>(coeffs), 1e-6);                        \
        ASSERT_NEAR(coeff_expected_2, std::get<2>(coeffs), 1e-6);                        \
    } while (false)

#endif //BANANA_PROJECT_POLYFIT_TEST_UTIL_HPP
