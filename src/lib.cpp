#include <stdexcept>

#include <banana-lib/lib.hpp>

namespace banana {

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

    auto Analyzer::AnnotateImage(cv::Mat const& image, std::list<AnalysisResult> const& analysis_result) const -> cv::Mat {
        auto annotated_image = cv::Mat{image};

        for (auto const& result : analysis_result) {
            cv::drawContours(annotated_image, std::vector{{result.contour}}, -1, this->contour_annotation_color_, 10);
        }

        return annotated_image;
    }

    auto Analyzer::AnalyzeAndAnnotateImage(cv::Mat const& image) const -> std::expected<AnnotatedAnalysisResult, AnalysisError> {
        return this->AnalyzeImage(image)
            .and_then([&image, this](auto const& analysis_result) -> std::expected<AnnotatedAnalysisResult, AnalysisError> {
                return AnnotatedAnalysisResult{this->AnnotateImage(image, analysis_result), analysis_result};
            });
    }

    auto Analyzer::FindBananaContours(cv::Mat const& image) const -> Contours {
        // TODO: implement
        return {};
    }

    auto Analyzer::AnalyzeBanana(const cv::Mat &image, const Contour &banana_contour) const -> std::expected<AnalysisResult, AnalysisError> {
        AnalysisResult result = {
            .contour = banana_contour,
        };

        return {result};
    }

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
}
