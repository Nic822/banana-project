#include <filesystem>
#include <format>
#include <iostream>
#include <stdexcept>
#include <numbers>

#include <opencv2/opencv.hpp>

#include <banana-lib/lib.hpp>

const cv::Size kWindowSize{768, 512};

[[nodiscard]]
auto GetPathFromArgs(int const argc, char const * const argv[]) -> std::filesystem::path {
    if(argc != 2) {
        throw std::runtime_error(std::format("expected 1 argument but got {}!", argc-1));
    }

    auto const image_path = std::filesystem::path(argv[1]);
    if (!std::filesystem::exists(image_path)) {
        throw std::runtime_error(std::format("specified path does not exist: {}", image_path.string()));
    }

    return image_path;
}

void PrintAnalysisResult(banana::AnnotatedAnalysisResult const& analysis_result) {
    std::cout << "found " << analysis_result.banana.size() << " banana(s) in the picture" << std::endl;

    for (auto const& [n, banana] : std::ranges::enumerate_view(analysis_result.banana)) {
        auto const& [coeff_0, coeff_1, coeff_2] = banana.center_line_coefficients;
        std::cout << "  Banana #" << n << ":" << std::endl;
        std::cout << "    " << std::format("y = {} + {} * x + {} * x^2", coeff_0, coeff_1, coeff_2) << std::endl;
        std::cout << "    Rotation = " << (banana.rotation_angle * 180 / std::numbers::pi) << " degrees" << std::endl;
        std::cout << std::endl;
    }
}

void ShowAnalysisResult(banana::AnnotatedAnalysisResult const& analysis_result) {
    std::string const windowName = "analysis result | press q to quit";
    cv::namedWindow(windowName, cv::WINDOW_KEEPRATIO);
    cv::imshow(windowName, analysis_result.annotated_image);
    cv::resizeWindow(windowName, kWindowSize);
    cv::waitKey();
}

int main(int const argc, char const * const argv[]) {
    banana::Analyzer const analyzer{true};
    try {
        auto const path = GetPathFromArgs(argc, argv);
        auto const img = cv::imread(path.string());

        auto const analysisResult = analyzer.AnalyzeAndAnnotateImage(img);

        if(analysisResult) {
            PrintAnalysisResult(*analysisResult);
            ShowAnalysisResult(*analysisResult);
        } else {
            std::cerr << "failed to analyse the image: " << analysisResult.error().ToString() << std::endl;
            return 1;
        }
    } catch (std::exception const& ex) {
        std::cerr << ex.what() << std::endl;
        std::cerr << "Usage: " << argv[0] << " [image_path]" << std::endl;
        return 1;
    }

    return 0;
}
