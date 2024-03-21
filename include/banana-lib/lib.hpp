#ifndef BANANA_PROJECT_LIB_HPP
#define BANANA_PROJECT_LIB_HPP

#include <expected>
#include <list>

#include <opencv2/opencv.hpp>

namespace banana {

    /**
     * All errors which may occur during analysis of the image.
     *
     * Note that this is a class wrapping an enum (instead of an `enum class`) to be able to provide methods on the values.
     */
    class AnalysisError {
    public:
        /**
         * Implementation Detail. Use `AnalysisError` to access the enum constants and interact with them.
         */
        enum Value {
            /// the requested functionality has not yet been implemented. note: will be removed once implemented!
            kNotYetImplementedError,
        };

        AnalysisError() = default;
        constexpr AnalysisError(Value const value) : value(value) { }
        constexpr explicit operator Value() const { return value; }
        explicit operator bool() const = delete;

        [[nodiscard]]
        auto ToString() const -> std::string;

        explicit operator std::string() const;
    private:
        Value value;
    };

    /**
     * The analysis results for a banana which has been found in the image.
     */
    struct AnalysisResult {
        // TODO: add attributes once we start analysing the image
    };

    struct AnnotatedAnalysisResult {
        /**
         * A copy of the original image with annotations in it for visualisation.
         */
        cv::Mat annotated_image;

        /**
         * The results for each banana which has been found. If no banana has been found this list is empty.
         */
        std::list<AnalysisResult> banana;
    };

    /**
     * Analyse an image for the presence of bananas and their properties.
     *
     * @param image an image possibly containing bananas
     * @return the analysis results for each banana which has been found. If no banana has been found this list is empty.
     */
    [[nodiscard]]
    auto AnalyzeImage(cv::Mat const& image) -> std::expected<std::list<AnalysisResult>, AnalysisError>;

    /**
     * Analyse an image for the presence of bananas and their properties.
     *
     * @param image an image possibly containing bananas
     * @return the result of the analysis and the annotated image, see the description of AnnotatedAnalysisResult for more details.
     */
    [[nodiscard]]
    auto AnalyzeAndAnnotateImage(cv::Mat const& image) -> std::expected<AnnotatedAnalysisResult, AnalysisError>;

}

#endif //BANANA_PROJECT_LIB_HPP
