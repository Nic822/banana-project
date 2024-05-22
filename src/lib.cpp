#include <numbers>
#include <numeric>
#include <ranges>
#include <stdexcept>
#include <utility>

#include <polyfit/Polynomial2DFit.hpp>
#include <banana-lib/lib.hpp>

//#define SHOW_DEBUG_INFO

namespace banana {

#ifdef SHOW_DEBUG_INFO
    namespace debug {
        void ShowImage(cv::Mat const& image, std::string const& windowName, cv::Size const& windowSize) {
            cv::namedWindow(windowName, cv::WINDOW_KEEPRATIO);
            cv::imshow(windowName, image);
            cv::resizeWindow(windowName, windowSize);
        }
    }

#  define SHOW_DEBUG_IMAGE(image, windowName) debug::ShowImage(image, windowName, cv::Size(768, 512))
#else
#  define SHOW_DEBUG_IMAGE(image, windowName)
#endif

    auto AnalysisError::ToString() const -> std::string {
        switch(value) {
            case kInvalidImage:
                return "invalid image!";
            case kPolynomialCalcFailure:
                return "unable to calculate the center line of the banana!";
            default:
                throw std::runtime_error("unknown AnalysisError type!");
        }
    }

    AnalysisError::operator std::string() const {
        return this->ToString();
    }

    std::ostream& operator << (std::ostream& o, AnnotatedAnalysisResult const& analysis_result) {
        o << "found " << analysis_result.banana.size() << " banana(s) in the picture" << std::endl;
        for (auto const& [n, banana] : std::ranges::enumerate_view(analysis_result.banana)) {
            auto const& [coeff_0, coeff_1, coeff_2] = banana.center_line.coefficients;
            o << "  Banana #" << n << ":" << std::endl;
            o << "    " << std::format("y = {:.6f} {:+.6f} * x {:+.6f} * x^2", coeff_0, coeff_1, coeff_2) << std::endl;
            o << "    Rotation = " << std::format("{:.2f}", banana.rotation_angle * 180 / std::numbers::pi) << " degrees" << std::endl;
            o << "    Mean curvature = " << std::format("{:.2f}", banana.mean_curvature / 100) << " 1/cm"
              << " (corresponds to a circle with radius = " << std::format("{:.2f}", 1/banana.mean_curvature * 100) << " cm)" << std::endl;
            o << "    Length along center line = " << std::format("{:.2f}", banana.length * 100) << " cm" << std::endl;
            o << "    ripeness = " << std::format("{:.0f}", banana.ripeness * 100) << " %" << std::endl;
            o << std::endl;
        }

        return o;
    }

    Analyzer::Analyzer(Settings settings) : settings_(std::move(settings)) {
        cv::FileStorage fs("resources/reference-contours.yml", cv::FileStorage::READ);
        if (!fs.isOpened()) {
            throw std::runtime_error("couldn't read the reference contour!");
        }
        fs["banana"] >> this->reference_contour_;
        fs.release();
    }

    auto Analyzer::AnalyzeImage(cv::Mat const& image) const -> std::expected<std::list<AnalysisResult>, AnalysisError> {
        if (image.data == nullptr) {
            return std::unexpected{AnalysisError::kInvalidImage};
        }

        auto const contours = this->FindBananaContours(image);

        std::list<AnalysisResult> analysis_results;

        for (auto const& contour : contours) {
            auto const result = this->AnalyzeBanana(image, contour);

            if (result) {
                analysis_results.push_back(result.value());
            } else {
                return std::unexpected{result.error()};
            }
        }

        return analysis_results;
    }

    auto Analyzer::AnalyzeAndAnnotateImage(cv::Mat const& image) const -> std::expected<AnnotatedAnalysisResult, AnalysisError> {
        return this->AnalyzeImage(image)
            .and_then([&image, this](auto const& analysis_result) -> std::expected<AnnotatedAnalysisResult, AnalysisError> {
                return AnnotatedAnalysisResult{this->AnnotateImage(image, analysis_result), analysis_result};
            });
    }

    auto Analyzer::ColorFilter(cv::Mat const& image, cv::Scalar low, cv::Scalar up) const -> cv::Mat {
        cv::Mat hsvImage;
        cv::cvtColor(image, hsvImage, cv::COLOR_BGR2HSV);

        cv::Mat mask;
        cv::inRange(hsvImage, low, up, mask);

        return mask;
    }

    auto Analyzer::IsBananaContour(Contour const& contour) const -> bool {
        if (cv::matchShapes(contour, this->reference_contour_, cv::CONTOURS_MATCH_I1, 0.0) > this->settings_.match_max_score) {
            return false;
        }
        auto const area = cv::contourArea(contour);
        return settings_.min_area < area && area < settings_.max_area;
    }

    auto Analyzer::FindBananaContours(cv::Mat const& image) const -> Contours {
        auto filtered_image = ColorFilter(image, settings_.filter_lower_threshold_color, settings_.filter_upper_threshold_color);
        SHOW_DEBUG_IMAGE(filtered_image, "color filtered image");

        // Removing noise
        auto const kernel = cv::getStructuringElement(cv::MORPH_RECT, {5, 5});
        cv::morphologyEx(filtered_image, filtered_image, cv::MORPH_OPEN, kernel);
        SHOW_DEBUG_IMAGE(filtered_image, "morph");

        // Smooth the image
        cv::medianBlur(filtered_image, filtered_image, 37);
        SHOW_DEBUG_IMAGE(filtered_image, "blur");

        Contours contours;
        cv::findContours(filtered_image, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        std::erase_if(contours, [this](auto const& contour) -> auto {
            return !this->IsBananaContour(contour);
        });

        return contours;
    }

    auto Analyzer::GetBananaCenterLineCoefficients(Contour const& rotated_banana_contour) const -> std::expected<Polynomial2DCoefficients, AnalysisError> {
        auto const to_std_pair_fn = [](auto const& p) -> std::pair<double, double> { return {p.x, p.y}; };
        auto const coeffs = polyfit::Fit2DPolynomial(rotated_banana_contour | std::views::transform(to_std_pair_fn));
        return coeffs.transform_error([](auto const& _) -> auto {return AnalysisError::kPolynomialCalcFailure;});
    }

    auto Analyzer::GetBananaCenterLine(Contour const& rotated_banana_contour, Polynomial2DCoefficients const& coefficients) const -> std::vector<cv::Point2d> {
        // note that the coefficients for the center line are given in relation to the bananas main axis.
        // accordingly we have to rotate the resulting line to plot it over the banana in the image.

        auto const& [coeff_0, coeff_1, coeff_2] = coefficients;

        auto const minmax_x = std::ranges::minmax(rotated_banana_contour | std::views::transform(&cv::Point::x));

        /// Calculate a Point2d for the [x,y] coords based on the provided polynomial and x-values.
        auto const calc_xy = [&coeff_0, &coeff_1, &coeff_2](auto const&& x) -> cv::Point2d {
            auto const y = coeff_0 + coeff_1 * x + coeff_2 * x * x;
            return {static_cast<double>(x), y};
        };

        return std::views::iota(minmax_x.min, minmax_x.max)
                   | std::views::transform(calc_xy)
                   | std::ranges::to<std::vector>();
    }

    auto Analyzer::RotateContour(Contour const& contour, cv::Point const& center, double const angle) const -> Contour {
        auto const rotation_matrix = cv::getRotationMatrix2D(center, angle * 180 / std::numbers::pi, 1);
        Contour rotated_contour{contour.size()};
        cv::transform(contour, rotated_contour, rotation_matrix);
        return rotated_contour;
    }

    auto Analyzer::GetPCA(const Contour &banana_contour) const -> Analyzer::PCAResult {
        // implementation adapted from https://docs.opencv.org/4.9.0/d1/dee/tutorial_introduction_to_pca.html

        // Convert points to format expected by PCA
        cv::Mat data_pts(static_cast<int>(banana_contour.size()), 2, CV_64F);
        for (int i = 0; i < data_pts.rows; ++i) {
            data_pts.at<double>(i, 0) = banana_contour[i].x;
            data_pts.at<double>(i, 1) = banana_contour[i].y;
        }
        // Perform PCA analysis
        cv::PCA pca{data_pts, {}, cv::PCA::DATA_AS_ROW};
        // Store the center of the object
        cv::Point center{static_cast<int>(pca.mean.at<double>(0, 0)),
                         static_cast<int>(pca.mean.at<double>(0, 1))};
        //Store the eigenvalues and eigenvectors
        std::vector<cv::Point2d> eigen_vecs(2);
        std::vector<double> eigen_vals(2);
        for (int i = 0; i < 2; ++i) {
            eigen_vecs[i] = cv::Point2d{pca.eigenvectors.at<double>(i, 0),
                                        pca.eigenvectors.at<double>(i, 1)};
            eigen_vals[i] = pca.eigenvalues.at<double>(i);
        }

        // The angle (in radians) is defined by the rotation of the x vector which corresponds to the primary direction as deduced by the PCA.
        auto const angle = std::atan2(eigen_vecs[0].y, eigen_vecs[0].x);

        return {
            .center = center,
            .eigen_vecs = eigen_vecs,
            .eigen_vals = eigen_vals,
            .angle = angle,
        };
    }

    auto Analyzer::CalculateMeanCurvature(AnalysisResult::CenterLine const& center_line) const -> double {
        auto const& [coeff_0, coeff_1, coeff_2] = center_line.coefficients;

        auto const x = center_line.points_in_banana_coordsys
                       | std::views::transform(&cv::Point2d::x);

        auto const calc_first_deriv = [coeff_1, coeff_2](auto const& x) -> auto {
            return 2 * coeff_2 * x + coeff_1;
        };

        /// y'(x) = 2ax + b
        auto const d1 = x | std::views::transform(calc_first_deriv);
        /// y''(x) = 2a = constant
        auto const d2 = std::views::repeat(2 * coeff_2);

        auto const calc_curvature = [](auto const&& d) -> auto {
            auto const& [d1_, d2_] = d;
            auto const c = std::abs(d2_) / std::sqrt(std::pow(1 + d1_*d1_, 3));
            return c;
        };

        // calculate the curvature of the center line at every point (in pixel)
        auto const curvature = std::views::zip(d1, d2) | std::views::transform(calc_curvature) | std::ranges::to<std::vector>();

        auto const mean_in_px = std::accumulate(curvature.cbegin(), curvature.cend(), 0.0) / static_cast<double>(curvature.size());

        return mean_in_px * this->settings_.pixels_per_meter; // 1/px * px/m = 1/m
    }

    auto Analyzer::CalculateBananaLength(AnalysisResult::CenterLine const& center_line) const -> double {
        auto const Distance = [](auto const& p1, auto const& p2) -> auto {
            return cv::norm(p1-p2);
        };
        auto const distances = center_line.points_in_banana_coordsys | std::views::pairwise_transform(Distance);
        auto const length_in_px = std::accumulate(distances.cbegin(), distances.cend(), 0.0);
        return length_in_px / this->settings_.pixels_per_meter;
    }

    auto Analyzer::GetMaskedImage(const cv::Mat& image, const Contour& contour) const -> cv::Mat {
        auto mask = cv::Mat{image.size(), CV_8UC3, cv::Scalar{255,255,255}};
        cv::drawContours(mask, std::vector{{contour}}, -1, {0,0,0}, cv::FILLED);
        SHOW_DEBUG_IMAGE(mask, "mask");
        cv::Mat masked;
        cv::bitwise_or(image, mask, masked);
        SHOW_DEBUG_IMAGE(masked, "filtered image (masked area only)");
        return masked;
    }

    auto Analyzer::IdentifyBananaRipeness(const cv::Mat& banana_image) const -> float {
        /// mask for green, yellow and brown colors
        auto const green_mask = ColorFilter(banana_image, settings_.green_lower_threshold_color, settings_.green_upper_threshold_color);
        auto const yellow_mask = ColorFilter(banana_image, settings_.yellow_lower_threshold_color, settings_.yellow_upper_threshold_color);
        auto const brown_mask = ColorFilter(banana_image, settings_.brown_lower_threshold_color, settings_.brown_upper_threshold_color);

        /// count pixels in the three color spaces
        auto const green_pixel_count = cv::countNonZero(green_mask);
        auto const yellow_pixel_count = cv::countNonZero(yellow_mask);
        auto const brown_pixel_count = cv::countNonZero(brown_mask);

        auto const total_pixel_count = green_pixel_count + yellow_pixel_count + brown_pixel_count;
        float green_share = static_cast<float>(green_pixel_count) / (static_cast<float>(total_pixel_count)+1e-3f);
        float brown_share = static_cast<float>(brown_pixel_count) / (static_cast<float>(total_pixel_count)+1e-3f);

        // assumption: if 100% is yellow we consider it ripe.
        // the more brown there is the riper it is, the more green there is the more unripe it is
        return 1 - green_share + brown_share;
    }

    auto Analyzer::AnalyzeBanana(cv::Mat const& image, Contour const& banana_contour) const -> std::expected<AnalysisResult, AnalysisError> {
        auto const pca = this->GetPCA(banana_contour);

        // rotate the contour so that it's horizontal
        auto const rotated_contour = this->RotateContour(banana_contour, pca.center, pca.angle);

        auto const coeffs = this->GetBananaCenterLineCoefficients(rotated_contour);
        if (!coeffs) {
            return std::unexpected{coeffs.error()};
        }

        AnalysisResult::CenterLine const center_line{
                .coefficients = *coeffs,
                .points_in_banana_coordsys = this->GetBananaCenterLine(rotated_contour, *coeffs),
        };

        auto const banana_only = this->GetMaskedImage(image, banana_contour);

        return AnalysisResult{
                .contour = banana_contour,
                .center_line = center_line,
                .rotation_angle = pca.angle,
                .estimated_center = pca.center,
                .mean_curvature = this->CalculateMeanCurvature(center_line),
                .length = this->CalculateBananaLength(center_line),
                .ripeness = this->IdentifyBananaRipeness(banana_only),
        };
    }

    void Analyzer::PlotCenterLine(cv::Mat& draw_target, AnalysisResult const& result) const {
        auto const to_point2i = [](auto const& p) -> cv::Point {return {static_cast<int>(p.x), static_cast<int>(p.y)};};
        auto const center_line_points2i = result.center_line.points_in_banana_coordsys
                                                                     | std::views::transform(to_point2i)
                                                                     | std::ranges::to<std::vector>();

        // rotate the center line back so that it fits on the image
        auto const rotated_center_line = this->RotateContour(center_line_points2i, result.estimated_center, -result.rotation_angle);

        cv::polylines(draw_target, rotated_center_line, false, this->settings_.helper_annotation_color, 10);
    }

    void Analyzer::PlotPCAResult(cv::Mat& draw_target, AnalysisResult const& result) const {
        auto const arrow_length = 50;
        auto const& rotation = result.rotation_angle;
        auto const& center = result.estimated_center;
        auto const x_endpoint = center + cv::Point{static_cast<int>(arrow_length * std::cos(rotation)), static_cast<int>(arrow_length * std::sin(rotation))};
        auto const y_endpoint = center + cv::Point{static_cast<int>(arrow_length * std::sin(rotation)),-static_cast<int>(arrow_length * std::cos(rotation))};
        cv::arrowedLine(draw_target, center, x_endpoint, {0, 0, 255}, 5);
        cv::arrowedLine(draw_target, center, y_endpoint, {255, 0, 0}, 5);
    }

    auto Analyzer::AnnotateImage(cv::Mat const& image, std::list<AnalysisResult> const& analysis_result) const -> cv::Mat {
        auto annotated_image = cv::Mat{image};

        for (auto const& [n, result] : std::ranges::enumerate_view(analysis_result)) {
            cv::drawContours(annotated_image, std::vector{{result.contour}}, -1, this->settings_.contour_annotation_color, 10);

            if (this->settings_.verbose_annotations) {
                cv::putText(annotated_image, std::to_string(n), result.estimated_center + cv::Point{35, -35}, cv::FONT_HERSHEY_COMPLEX_SMALL, 2, this->settings_.helper_annotation_color);
                this->PlotCenterLine(annotated_image, result);
                this->PlotPCAResult(annotated_image, result);
            }
        }

        return annotated_image;
    }

}
