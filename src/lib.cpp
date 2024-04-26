#include <stdexcept>

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
            default:
                throw std::runtime_error("unknown AnalysisError type!");
        }
    }

    AnalysisError::operator std::string() const {
        return this->ToString();
    }

    Analyzer::Analyzer() {
        cv::FileStorage fs("resources/reference-contours.yml", cv::FileStorage::READ);
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

    auto Analyzer::AnalyzeBanana(const cv::Mat &image, const Contour &banana_contour) const -> std::expected<AnalysisResult, AnalysisError> {
        AnalysisResult result = {
                .contour = banana_contour,
        };

        return {result};
    }

    auto Analyzer::AnnotateImage(cv::Mat const& image, std::list<AnalysisResult> const& analysis_result) const -> cv::Mat {
        auto annotated_image = cv::Mat{image};

        for (auto const& result : analysis_result) {
            cv::drawContours(annotated_image, std::vector{{result.contour}}, -1, this->contour_annotation_color_, 10);
        }

        return annotated_image;
    }
}
