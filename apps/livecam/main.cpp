#include <iostream>
#include <filesystem>
#include <stdexcept>
#include <format>

#include <opencv2/opencv.hpp>

#include <banana-lib/lib.hpp>

const cv::Size kWindowSize = {768, 512};

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
    banana::Analyzer const analyzer{};
    try {
        auto cap = GetVideoCaptureFromArgs(argc, argv);
        if(!cap.isOpened()) {
            std::cerr << "can't use camera" << std::endl;
            return 1;
        }

        while (true) {
            cv::Mat frame;
            cap >> frame;
            auto const analysisResult = analyzer.AnalyzeAndAnnotateImage(frame);

            if (analysisResult) {
                ShowAnalysisResult(analysisResult.value());
            } else {
                std::cerr << "failed to analyse the image: " << analysisResult.error().ToString() << std::endl;
                return 1;
            }

            if (cv::pollKey() == 'q') {
                return 0;
            }
        }
    } catch (std::exception const& ex) {
        std::cerr << ex.what() << std::endl;
        std::cerr << "Usage: " << argv[0] << " [capture_device_id|video_path]" << std::endl;
        return 1;
    }
}
