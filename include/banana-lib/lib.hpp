#ifndef BANANA_PROJECT_LIB_HPP
#define BANANA_PROJECT_LIB_HPP

#include <expected>
#include <iostream>
#include <list>
#include <utility>
#include <vector>

#include <opencv2/opencv.hpp>

namespace banana {

    /// Single contour around a detected object.
    typedef std::vector<cv::Point> Contour;
    /// List of multiple contours.
    typedef std::vector<Contour> Contours;
    /// The coefficients a0, a1 and a2 of the two-dimensional polynomial $y = a0 + a1 * x + a2 * x^2$.
    typedef std::tuple<double, double, double> Polynomial2DCoefficients;

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
            /// Unable to calculate center line of a banana
            kPolynomialCalcFailure,
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
        struct CenterLine {
            /**
             * The coefficients a0, a1 and a2 of the two-dimensional polynomial describing the center line of the banana.
             * Important: note that this is given along the primary axis of the banana and not in relation to the x-axis of the image.
             *
             * The center line is defined approximately by the method $y = a0 + a1 * x + a2 * x^2$.
             */
            Polynomial2DCoefficients coefficients;

            /**
             * The points along the center line inside of the banana contour (rotated, banana coordinate system).
             */
            std::vector<cv::Point2d> points_in_banana_coordsys;
        };

        /// Contour of the banana in the image.
        Contour contour;

        CenterLine center_line;

        /// The rotation angle of the banana, as seen from the x-axis. Given in radians.
        double rotation_angle;

        /// The estimated center of the banana shape. Note that this might actually lie outside of the banana itself due to the curvature!
        cv::Point estimated_center;

        /// The estimated mean curvature of the banana.
        double mean_curvature;

        /// The length of the banana (in px).
        double length;

        /**
         * Ripeness as a percentage (100% = 1.0):
         * * < 100%: not yet ripe
         * * = 100%: ripe
         * * > 100%: over-ripe
         */
        float ripeness;
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

    /// Print detailed information about the result to an output stream.
    std::ostream& operator << (std::ostream& o, AnnotatedAnalysisResult const& analysis_result);

    class Analyzer {
    public:
        struct Settings {
            /// Whether verbose annotations should be used when annotating the image. If enabled more information will be written on the image.
            bool const verbose_annotations{false};

            /// Maximum score of `cv::matchShapes` which we still accept as a banana.
            float const match_max_score{0.6f};

            /// Minimum area value of a banana (in px^2).
            float const min_area{1e5f};

            /// Maximum area value of a banana (in px^2).
            float const max_area{1e7f};

            /// How long (in pixels) is a meter? This is the extrinsic calibration needed to calculate sizes.
            double const pixels_per_meter;

            /// Color used to annotate the contours on the analyzed image.
            cv::Scalar const contour_annotation_color{0, 255, 0};
            /// Color used to annotate debug information on the analyzed image.
            cv::Scalar const helper_annotation_color{0, 0, 255};

            /// Green color range used to filter the ripeness on the analyzed image.
            cv::Scalar const green_lower_threshold_color{35, 50, 50};
            cv::Scalar const green_upper_threshold_color{85, 255, 255};

            /// Yellow color range used to filter the ripeness on the analyzed image.
            cv::Scalar const yellow_lower_threshold_color{20, 100, 100};
            cv::Scalar const yellow_upper_threshold_color{30, 255, 255};

            /// Brown color range used to filter the ripeness on the analyzed image.
            cv::Scalar const brown_lower_threshold_color{10, 100, 20};
            cv::Scalar const brown_upper_threshold_color{20, 200, 100};

            /// Color threshold used to filter the incoming colors on the analyzed image.
            cv::Scalar const filter_lower_threshold_color{0, 41, 0};
            cv::Scalar const filter_upper_threshold_color{177, 255, 255};
        };

        explicit Analyzer(Settings settings);

        /**
         * Analyse an image for the presence of bananas and their properties.
         *
         * @param image an image possibly containing bananas
         * @return the analysis results for each banana which has been found. If no banana has been found this list is empty.
         */
        [[nodiscard]]
        auto AnalyzeImage(cv::Mat const& image) const -> std::expected<std::list<AnalysisResult>, AnalysisError>;

        /**
         * Analyse an image for the presence of bananas and their properties.
         *
         * @param image an image possibly containing bananas
         * @return the result of the analysis and the annotated image, see the description of AnnotatedAnalysisResult for more details.
         */
        [[nodiscard]]
        auto AnalyzeAndAnnotateImage(cv::Mat const& image) const -> std::expected<AnnotatedAnalysisResult, AnalysisError>;

    private:
        /// Internal structure to store the results of `GetPCA` for further processing in a convenient way.
        struct PCAResult {
            cv::Point center;
            std::vector<cv::Point2d> eigen_vecs;
            std::vector<double> eigen_vals;
            double angle;
        };

        /// All externally configurable settings used by the analyzer.
        Settings const settings_;

        /// Reference contour for the banana, used in filtering.
        Contour reference_contour_;

        /**
         * filters the image for banana-related colors and returns a corresponding binary image.
         *
         * @param image the image to be filtered
         * @param low the lower bound which should be passed through - value must be in HSV!
         * @param high the upper bound which should be passed through - value must be in HSV!
         * @return binary image, which colours the matching pixels white, otherwise black
         */
        [[nodiscard]]
        auto ColorFilter(cv::Mat const& image, cv::Scalar low, cv::Scalar up) const -> cv::Mat;

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
         * Calculate the coefficients of the two-dimensional polynomial describing the center line.
         * Important: note that this is given along the primary axis of the banana and not in relation to the x-axis of the image.
         *
         * @param rotated_banana_contour the contour of the banana to be analysed - already rotated so that the x-axis is along the primary axis of the banana.
         * @param pca_result the result from the PCA analysis
         * @return the coefficients of the two-dimensional polynomial describing the center line of the banana.
         */
        [[nodiscard]]
        auto GetBananaCenterLineCoefficients(Contour const& rotated_banana_contour) const -> std::expected<Polynomial2DCoefficients, AnalysisError>;

        /**
         * Calculate the center line of the banana in the coordinate system of the banana (x-axis along the primary axis of the banana).
         *
         * @param rotated_banana_contour the contour of the banana to be analysed - already rotated so that the x-axis is along the primary axis of the banana.
         * @param coefficients the coefficients of the two-dimensional polynomial describing the center line of the banana
         * @return a consecutive list of points with 1px spacing on the x-axis along the whole x-axis of the banana, describing the center line.
         */
        [[nodiscard]]
        auto GetBananaCenterLine(Contour const& rotated_banana_contour, Polynomial2DCoefficients const& coefficients) const -> std::vector<cv::Point2d>;

        /**
         * Rotate a contour by the defined angle around the specified point.
         *
         * @param contour the contour to be rotated
         * @param center the center around which the contour is to be rotated
         * @param angle the angle in radians by which it should be rotated
         * @return the rotated contour
         * @see cv::getRotationMatrix2D
         * @see cv::transform
         */
        [[nodiscard]]
        auto RotateContour(Contour const& contour, cv::Point const& center, double const angle) const -> Contour;

        /**
         * Calculate the PCA of the provided contour. This yields information about the center and rotation of the shape.
         *
         * @param banana_contour the contour of the banana to be analysed
         * @return the result of the PCA analysis.
         */
        [[nodiscard]]
        auto GetPCA(Contour const& banana_contour) const -> PCAResult;

        /**
         * Calculate the mean curvature of the center line.
         *
         * @param center_line the center line of the banana.
         * @return the mean curvature of the center line (in 1/m).
         */
        [[nodiscard]]
        auto CalculateMeanCurvature(AnalysisResult::CenterLine const& center_line) const -> double;

        /**
         * Calculate the length of the banana (in meters) along the center line.
         *
         * @param center_line the center line of the banana.
         * @return the length of the banana in meters.
         */
        [[nodiscard]]
        auto CalculateBananaLength(AnalysisResult::CenterLine const& center_line) const -> double;

        /**
         * Extract the masked part of an image for the defined contour.
         * @param image the image from which the masked part should be extracted.
         * @param contour the contour defining the mask.
         * @return the masked image. all parts outside of the mask will be black.
         */
        [[nodiscard]]
        auto GetMaskedImage(cv::Mat const& image, Contour const& contour) const -> cv::Mat;

        /**
         * Identify the ripeness of the banana.
         * @param banana_image the image containing exactly the banana to be analysed (extracted using mask from original).
         * @return Ripeness as a percentage (100% = 1.0):
         * * < 100%: not yet ripe
         * * = 100%: ripe
         * * > 100%: over-ripe
         */
        [[nodiscard]]
        auto IdentifyBananaRipeness(cv::Mat const& banana_image) const -> float;

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
         * Plot the center line of the banana onto the provided draw target.
         *
         * @param draw_target the image being annotated.
         * @param center_line the center line along the whole x-axis of the banana in the coordinate system of the x-axis
         * @param result the analysis result for the banana to be drawn.
         */
        void PlotCenterLine(cv::Mat& draw_target, AnalysisResult const& result) const;

        /**
         * Plot the results of the PCA, i.e. the center and coordinate system of the banana.
         *
         * @param draw_target the image being annotated.
         * @param result the analysis result for the banana to be drawn.
         */
        void PlotPCAResult(cv::Mat& draw_target, AnalysisResult const& result) const;

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
