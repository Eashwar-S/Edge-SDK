#include "sample_rgb_ir.h"

#include <thread>
#include <vector>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "logger.h"
#include "sample_liveview.h"
#include "stream_decoder.h"
#include "image_processor_thread.h"
#include "stream_processor_thread.h"

using namespace edge_sdk;

namespace edge_app {

RgbIrImageProcessor::RgbIrImageProcessor(const std::string& model_path)
    : model_path_(model_path) {
  homography_ = (cv::Mat_<double>(3, 3) << 7.84539671e-01, 3.23869026e-02,
                 3.66465224e+02, 2.86785918e-02, 7.60677032e-01, 2.69386770e+02,
                 4.78052584e-05, 3.51503005e-05, 1.0);
}

int32_t RgbIrImageProcessor::Init() {
  if (!model_path_.empty()) {
    try {
      net_ = cv::dnn::readNetFromONNX(model_path_);
    } catch (const cv::Exception& e) {
      ERROR("Failed to load model %s: %s", model_path_.c_str(), e.what());
    }
  }
  cv::namedWindow("rgb_ir_view", cv::WINDOW_NORMAL);
  cv::resizeWindow("rgb_ir_view", 960, 540);
  return 0;
}

void RgbIrImageProcessor::Process(const std::shared_ptr<Image> image) {
  if (!image) return;
  cv::Mat frame = *image;
  if (frame.channels() == 3) {
    if (!net_.empty()) {
      cv::Mat blob = cv::dnn::blobFromImage(frame, 1.0 / 255.0,
                                            cv::Size(640, 640), cv::Scalar(),
                                            true, false);
      net_.setInput(blob);
      cv::Mat out = net_.forward();
      for (int i = 0; i < out.rows; ++i) {
        float score = out.at<float>(i, 4);
        if (score < 0.5) continue;
        int cx = static_cast<int>(out.at<float>(i, 0) * frame.cols);
        int cy = static_cast<int>(out.at<float>(i, 1) * frame.rows);
        int w = static_cast<int>(out.at<float>(i, 2) * frame.cols);
        int h = static_cast<int>(out.at<float>(i, 3) * frame.rows);
        int x = cx - w / 2;
        int y = cy - h / 2;
        cv::rectangle(frame, cv::Rect(x, y, w, h), cv::Scalar(0, 255, 0), 2);
      }
    }
    cv::imshow("rgb_ir_view", frame);
  } else {
    cv::Mat warped;
    cv::warpPerspective(frame, warped, homography_, cv::Size(1440, 1080));
    cv::Mat hsv;
    cv::cvtColor(warped, hsv, cv::COLOR_BGR2HSV);
    cv::Scalar lower(12.0, 42.0, 39.0);
    cv::Scalar upper(166.0, 255.0, 255.0);
    // Hue range in OpenCV is 0-180, so use given values directly
    cv::Mat mask;
    cv::inRange(hsv, lower, upper, mask);
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL,
                     cv::CHAIN_APPROX_SIMPLE);
    for (const auto& c : contours) {
      cv::Rect box = cv::boundingRect(c);
      cv::rectangle(warped, box, cv::Scalar(0, 0, 255), 2);
    }
    cv::imshow("rgb_ir_view", warped);
  }
  cv::waitKey(1);
}

}  // namespace edge_app

ErrorCode ESDKInit();

int main(int argc, char** argv) {
  using namespace edge_app;
  auto rc = ESDKInit();
  if (rc != kOk) {
    ERROR("pre init failed");
    return -1;
  }

  auto liveview = std::make_shared<LiveviewSample>("RgbIr");
  StreamDecoder::Options decoder_option = {.name = std::string("ffmpeg")};
  auto decoder = CreateStreamDecoder(decoder_option);

  auto processor = std::make_shared<RgbIrImageProcessor>(
      std::string("examples/liveview/model.onnx"));
  processor->Init();

  if (0 != InitLiveviewSample(liveview, Liveview::kCameraTypePayload,
                              Liveview::kStreamQuality1080pHigh, decoder,
                              processor)) {
    ERROR("Init liveview sample failed");
    return -1;
  }
  liveview->Start();

  while (1) std::this_thread::sleep_for(std::chrono::seconds(3));
  return 0;
}
