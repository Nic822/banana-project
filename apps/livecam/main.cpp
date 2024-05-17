#include <filesystem>
#include <format>
#include <iostream>
#include <stdexcept>

#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/logger.hpp>

#include <banana-lib/lib.hpp>

const cv::Size kWindowSize{768, 512};

[[nodiscard]]
auto GetVideoCaptureFromArgs(int const argc, char const * const argv[]) -> cv::VideoCapture {
    if(argc > 2) {
        throw std::runtime_error(std::format("expected 0 or 1 arguments but got {}!", argc-1));
    } else if(argc == 2) {
        try {
            // numeric value => it's the index of a video device
            return cv::VideoCapture{std::stoi(argv[1])};
        } catch (std::invalid_argument const& ex) {
            // non-numeric value => treat it as a path, url or similar and let OpenCV check if it's valid
            return cv::VideoCapture{argv[1]};
        }
    } else { // argc == 1
        return cv::VideoCapture{0};
    }
}

void ShowAnalysisResult(banana::AnnotatedAnalysisResult const& analysis_result) {
    std::string const windowName = "analysis result | press q to quit";
    cv::namedWindow(windowName, cv::WINDOW_KEEPRATIO);
    cv::imshow(windowName, analysis_result.annotated_image);
    cv::resizeWindow(windowName, kWindowSize);
}

int main(int const argc, char const * const argv[]) {
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_WARNING);

    banana::Analyzer const analyzer{{
        .verbose_annotations = true,
    }};
    try {
        auto cap = GetVideoCaptureFromArgs(argc, argv);
        if(!cap.isOpened()) {
            std::cerr << "can't use camera" << std::endl;
            return 1;
        }

        std::cout << R"(
Available action keys:
* press 'i' to show information on the bananas currently visible in the frame
* press 'q' to quit
)" << std::endl;

        while (true) {
            cv::Mat frame;
            cap >> frame;
            auto const analysisResult = analyzer.AnalyzeAndAnnotateImage(frame);

            if (analysisResult) {
                ShowAnalysisResult(*analysisResult);
            } else {
                std::cerr << "failed to analyse the image: " << analysisResult.error().ToString() << std::endl;
                return 1;
            }

            switch (cv::pollKey()) {
                case 'i':
                    std::cout << *analysisResult;
                    break;
                case 'q':
                    return 0;
                default:
                    break;
            }
        }
    } catch (std::exception const& ex) {
        std::cerr << ex.what() << std::endl;
        std::cerr << "Usage: " << argv[0] << " [capture_device_id|video_path]" << std::endl;
        return 1;
    }
}
