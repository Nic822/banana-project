#ifndef BANANA_PROJECT_LIB_HPP
#define BANANA_PROJECT_LIB_HPP

#include <expected>
#include <list>

#include <opencv2/opencv.hpp>

namespace banana {

    /// Single contour around a detected object.
    typedef std::vector<cv::Point> Contour;
    /// List of multiple contours.
    typedef std::vector<Contour> Contours;

    /**
     * All errors which may occur during analysis of the image.
     *
     * Note that this is a class wrapping an enum (instead of an `enum class`) to be able to provide methods on the values.
     */
    class AnalysisError {
    public:
        /**
         * Implementation Detail. Use `AnalysisError` to access the enum constants and interact with them.
         *
         * Ensure that you add any value listed here also to `AnalysisError::ToString`!
         */
        enum Value {
            /// The provided image is invalid (e.g. empty / no data).
            kInvalidImage,
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
        /// Contour of the banana in the image.
        Contour contour;
    };

    /**
     * The analysis results as well as an image annotated with them which can be used for visualisation.
     */
    struct AnnotatedAnalysisResult {
        /// A copy of the original image with annotations in it for visualisation.
        cv::Mat annotated_image;

        /// The results for each banana which has been found. If no banana has been found this list is empty.
        std::list<AnalysisResult> banana;
    };

    class Analyzer {
    public:
        Analyzer();

        /**
         * Analyse an image for the presence of bananas and their properties.
         *
         * @param image an image possibly containing bananas
         * @return the analysis results for each banana which has been found. If no banana has been found this list is empty.
         */
        [[nodiscard]]
        auto AnalyzeImage(cv::Mat const &image) const -> std::expected<std::list<AnalysisResult>, AnalysisError>;

        /**
         * Analyse an image for the presence of bananas and their properties.
         *
         * @param image an image possibly containing bananas
         * @return the result of the analysis and the annotated image, see the description of AnnotatedAnalysisResult for more details.
         */
        [[nodiscard]]
        auto AnalyzeAndAnnotateImage(cv::Mat const &image) const -> std::expected<AnnotatedAnalysisResult, AnalysisError>;

    private:
        /// Color used to annotate the contours on the analyzed image.
        cv::Scalar const contour_annotation_color_{0, 255, 0};

        /// Maximum score of `cv::matchShapes` which we still accept as a banana.
        float match_max_score_ = 0.8f;

        /// Reference contour for the banana, used in filtering.
        Contour reference_contour_;

        /**
         * filters the image for banana-related colors and returns a corresponding binary image.
         * @param image the image to be filtered
         * @return binary image, which colours the matching pixels white, otherwise black
         */
        [[nodiscard]]
        auto ColorFilter(cv::Mat const& image) const -> cv::Mat;

        /**
         * Checks whether the passed contour is - with a good likelihood - a banana.
         * @param contour the contour which may or may not be a banana
         * @return whether it is a banana
         */
        [[nodiscard]]
        auto IsBananaContour(Contour const& contour) const -> bool;

        /**
         * Identify all bananas present in an image and return their contours.
         *
         * @param image the image containing bananas.
         * @return a list of the contours of all identified bananas. may be empty if no bananas have been found.
         */
        [[nodiscard]]
        auto FindBananaContours(cv::Mat const& image) const -> Contours;

        /**
         * Analyse the banana.
         *
         * @param image the image containing bananas.
         * @param banana_contour the contour of the banana to be analysed
         * @return
         */
        [[nodiscard]]
        auto AnalyzeBanana(cv::Mat const& image, Contour const& banana_contour) const -> std::expected<AnalysisResult, AnalysisError>;

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
        auto AnnotateImage(cv::Mat const& image, std::list<AnalysisResult> const& analysis_result) const -> cv::Mat;

    };

}

#endif //BANANA_PROJECT_LIB_HPP
