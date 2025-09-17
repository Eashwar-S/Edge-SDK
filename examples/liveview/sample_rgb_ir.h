#ifndef SAMPLE_RGB_IR_H
#define SAMPLE_RGB_IR_H

#include "image_processor.h"
#include <opencv2/dnn.hpp>

namespace edge_app {

class RgbIrImageProcessor : public ImageProcessor {
 public:
  explicit RgbIrImageProcessor(const std::string& model_path);
  int32_t Init() override;
  void Process(const std::shared_ptr<Image> image) override;

 private:
  cv::dnn::Net net_;
  std::string model_path_;
  cv::Mat homography_;
};

}  // namespace edge_app

#endif  // SAMPLE_RGB_IR_H
