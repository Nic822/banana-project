#include <stdexcept>

#include <banana-lib/lib.hpp>

namespace banana {

    auto AnalyzeImage(cv::Mat const& image) -> std::expected<std::list<AnalysisResult>, AnalysisError> {
        return std::unexpected(AnalysisError::kNotYetImplementedError);
    }

    /**
     * Annotate an image with the result from a previous analysis (the analysis must come from the same image).
     * This is meant for visualisation to users and is not guaranteed to produce stable results.
     *
     * @param image a previously analysed image
     * @param analysis_result the result of the previous analysis (done using AnalyzeImage).
     * @return a copy of the original image with annotations.
     * @see AnalyzeImage
     */
    [[nodiscard]]
    auto AnnotateImage(cv::Mat const& image, std::list<AnalysisResult> const& analysis_result) -> cv::Mat {
        auto annotated_image = cv::Mat{image};

        // TODO: use analysis_result to annotate the image

        return annotated_image;
    }

    auto AnalyzeAndAnnotateImage(cv::Mat const& image) -> std::expected<AnnotatedAnalysisResult, AnalysisError> {
        return AnalyzeImage(image)
            .and_then([&image](std::list<AnalysisResult> const& analysis_result) -> std::expected<AnnotatedAnalysisResult, AnalysisError> {
                return AnnotatedAnalysisResult{AnnotateImage(image, analysis_result), analysis_result};
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
