#include <stdexcept>

#include <banana-lib/lib.hpp>

namespace banana {

    auto Analyzer::AnalyzeImage(cv::Mat const& image) const -> std::expected<std::list<AnalysisResult>, AnalysisError> {
        return std::unexpected(AnalysisError::kNotYetImplementedError);
    }

    auto Analyzer::AnnotateImage(cv::Mat const& image, std::list<AnalysisResult> const& analysis_result) const -> cv::Mat {
        auto annotated_image = cv::Mat{image};

        // TODO: use analysis_result to annotate the image

        return annotated_image;
    }

    auto Analyzer::AnalyzeAndAnnotateImage(cv::Mat const& image) const -> std::expected<AnnotatedAnalysisResult, AnalysisError> {
        return this->AnalyzeImage(image)
            .and_then([&image, this](std::list<AnalysisResult> const& analysis_result) -> std::expected<AnnotatedAnalysisResult, AnalysisError> {
                return AnnotatedAnalysisResult{this->AnnotateImage(image, analysis_result), analysis_result};
            });
    }

    auto AnalysisError::ToString() const -> std::string {
        switch(value) {
            case kNotYetImplementedError:
                return "not yet implemented!";
            default:
                throw std::runtime_error("unknown AnalysisError type!");
        }
    }

    AnalysisError::operator std::string() const {
        return this->ToString();
    }
}
