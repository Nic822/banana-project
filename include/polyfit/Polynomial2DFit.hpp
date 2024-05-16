/**\file
 * \brief Header-only library which provides a simple API to fit a 2-dimensional polynomial (i.e. 3 coefficients) to a set of two-dimensional points (x, y coordinates).
 */

#ifndef BANANA_PROJECT_POLYNOMIAL2DFIT_HPP
#define BANANA_PROJECT_POLYNOMIAL2DFIT_HPP

#include <expected>
#include <tuple>
#include <utility>
#include <vector>

// TODO: we currently leak the ceres dependency to everyone which includes this header-only library.
//  when changing this we'll also need to change the return type of `Fit2DPolynomial` which currently returns
//  the ceres termination type in case of errors.
#include <ceres/ceres.h>

/**
 * \brief Header-only library which provides a simple API to fit a 2-dimensional polynomial (i.e. 3 coefficients) to a set of two-dimensional points (x, y coordinates).
 */
namespace polyfit {

    /** Internal helpers, do not use from the outside! */
    namespace internal {
        /**
         * Calculates the residual (= error = distance) of the estimate for a specified point.
         */
        struct Polynomial2DResidual {
        public:
            Polynomial2DResidual(double const x, double const y) : x_(x), y_(y) {}

            template<typename T>
            bool operator()(const T *const a0, const T *const a1, const T *const a2, T *residual) const {
                auto const y_estimate = a0[0] + a1[0] * x_ + a2[0] * x_ * x_;
                residual[0] = y_ - y_estimate;
                return true;
            }

        private:
            double const x_;
            double const y_;
        };
    }

    /**
     * Calculate the coefficients for a two-dimensional polynomial which fits the points as well as possible.
     *
     * @tparam R the underlying container supporting ranges. must contain std::tuple<double, double>
     * @tparam print_report if enabled the underlying solver will print a detailed report to STDOUT. helpful for debugging.
     * @param points the set of points (x & y coordinates) for which the polynomial should be fitted.
     * @return either the set of coefficients or the termination type indicating why the fitting failed.
     */
    template<std::ranges::range R, bool print_report = false>
    [[nodiscard]]
    auto Fit2DPolynomial(
            R&& points) -> std::expected<std::tuple<double, double, double>, ceres::TerminationType> {
        double a0 = 1, a1 = 1, a2 = 1;

        ceres::Problem problem;
        for (auto const& point: points) {
            ceres::CostFunction *cost_function =
                    new ceres::AutoDiffCostFunction<internal::Polynomial2DResidual, 1, 1, 1, 1>(
                            new internal::Polynomial2DResidual(point.first, point.second));
            problem.AddResidualBlock(cost_function, nullptr, &a0, &a1, &a2);
        }

        ceres::Solver::Options options{
                .logging_type = print_report ? ceres::PER_MINIMIZER_ITERATION : ceres::SILENT,
                .minimizer_progress_to_stdout = print_report,
        };
        ceres::Solver::Summary summary;
        ceres::Solve(options, &problem, &summary);
        if (print_report) {
            std::cout << summary.FullReport() << std::endl;
        }
        if (summary.termination_type == ceres::CONVERGENCE) {
            return {{a0, a1, a2}};
        } else {
            return std::unexpected{summary.termination_type};
        }
    };

}

#endif //BANANA_PROJECT_POLYNOMIAL2DFIT_HPP
