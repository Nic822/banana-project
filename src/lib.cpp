#include <stdexcept>

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

    Analyzer::Analyzer(bool const verbose_annotations) : verbose_annotations_(verbose_annotations) {
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

    auto Analyzer::ColorFilter(cv::Mat const& image) const -> cv::Mat {
        cv::Mat hsvImage;
        cv::cvtColor(image, hsvImage, cv::COLOR_BGR2HSV);

        // Define the range for colors in the HSV color space
        auto const lowerThreshold = cv::Scalar(0, 41, 0);
        auto const upperThreshold = cv::Scalar(177, 255, 255);

        cv::Mat mask;
        cv::inRange(hsvImage, lowerThreshold, upperThreshold, mask);

        return mask;
    }

    auto Analyzer::IsBananaContour(Contour const& contour) const -> bool {
        return cv::matchShapes(contour, this->reference_contour_, cv::CONTOURS_MATCH_I1, 0.0) > match_max_score_;
    }

    auto Analyzer::FindBananaContours(cv::Mat const& image) const -> Contours {
        auto filtered_image = ColorFilter(image);
        SHOW_DEBUG_IMAGE(filtered_image, "color filtered image");

        // Removing noise
        auto const kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
        cv::morphologyEx(filtered_image, filtered_image, cv::MORPH_OPEN, kernel);
        SHOW_DEBUG_IMAGE(filtered_image, "morph");

        // Smooth the image
        cv::medianBlur(filtered_image, filtered_image, 37); // TODO: test again with 41
        SHOW_DEBUG_IMAGE(filtered_image, "blur");

        Contours contours;
        cv::findContours(filtered_image, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        std::erase_if(contours, [this](auto const& contour) -> auto {
            return this->IsBananaContour(contour);
        });

        return contours;
    }

    auto Analyzer::GetBananaCenterLineCoefficients(Contour const& banana_contour) const -> std::expected<std::tuple<double, double, double>, AnalysisError> {
        auto const to_std_pair_fn = [](auto const& p) -> std::pair<double, double> { return {p.x, p.y}; };
        auto const coeffs = polyfit::Fit2DPolynomial(banana_contour | std::views::transform(to_std_pair_fn));
#ifdef SHOW_DEBUG_INFO
        if (coeffs) {
            std::cout << std::format("y = {} + {} * x + {} * x^2", std::get<0>(*coeffs), std::get<1>(*coeffs),
                                     std::get<2>(*coeffs)) << std::endl;
        } else {
            std::cerr << "couldn't find a solution!" << std::endl;
        }
#endif
        return coeffs.transform_error([](auto const& _) -> auto {return AnalysisError::kPolynomialCalcFailure;});
    }

    auto Analyzer::AnalyzeBanana(cv::Mat const& image, Contour const& banana_contour) const -> std::expected<AnalysisResult, AnalysisError> {
        auto const coeffs = this->GetBananaCenterLineCoefficients(banana_contour);
        if (!coeffs) {
            return std::unexpected{coeffs.error()};
        }

        return AnalysisResult{
                .contour = banana_contour,
                .center_line_coefficients = *coeffs,
        };
    }

    void Analyzer::PlotCenterLine(cv::Mat& draw_target, AnalysisResult const& result) const {
        auto const& [coeff_0, coeff_1, coeff_2] = result.center_line_coefficients;

        auto const minmax_x = std::ranges::minmax(result.contour | std::views::transform(&cv::Point::x));

        /// Calculate a Point2d for the [x,y] coords based on the provided polynomial and x-values.
        auto const calc_xy = [&coeff_0, &coeff_1, &coeff_2](auto const&& x) -> cv::Point2d {
            auto const y = coeff_0 + coeff_1 * x + coeff_2 * x * x;
            return {static_cast<double>(x), y};
        };
        auto const to_point2i = [](auto const&& p) -> cv::Point {return {static_cast<int>(p.x), static_cast<int>(p.y)};};

        auto const line_extension_length = 50; ///< amount of pixels by which the line should be extended on either side
        auto const start = std::max(minmax_x.min - line_extension_length, 0);
        auto const end = std::min(minmax_x.max + line_extension_length, draw_target.cols);
        auto const center_line_points = std::views::iota(start, end) | std::views::transform(calc_xy);
        auto const center_line_points2i = center_line_points | std::views::transform(to_point2i) | std::ranges::to<std::vector>();

        cv::polylines(draw_target, center_line_points2i, false, this->helper_annotation_color_, 10);
    }

    auto Analyzer::AnnotateImage(cv::Mat const& image, std::list<AnalysisResult> const& analysis_result) const -> cv::Mat {
        auto annotated_image = cv::Mat{image};

        for (auto const& result : analysis_result) {
            cv::drawContours(annotated_image, std::vector{{result.contour}}, -1, this->contour_annotation_color_, 10);

            if (this->verbose_annotations_) {
                this->PlotCenterLine(annotated_image, result);
            }
        }

        return annotated_image;
    }
}
