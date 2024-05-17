#include <filesystem>
#include <format>
#include <iostream>
#include <stdexcept>

#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/logger.hpp>

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

void ShowAnalysisResult(banana::AnnotatedAnalysisResult const& analysis_result) {
    std::string const windowName = "analysis result | press q to quit";
    cv::namedWindow(windowName, cv::WINDOW_KEEPRATIO);
    cv::imshow(windowName, analysis_result.annotated_image);
    cv::resizeWindow(windowName, kWindowSize);
    cv::waitKey();
}

int main(int const argc, char const * const argv[]) {
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_WARNING);

    banana::Analyzer const analyzer{{
        .verbose_annotations = true,
    }};
    try {
        auto const path = GetPathFromArgs(argc, argv);
        auto const img = cv::imread(path.string());

        auto const analysisResult = analyzer.AnalyzeAndAnnotateImage(img);

        if(analysisResult) {
            std::cout << *analysisResult;
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
