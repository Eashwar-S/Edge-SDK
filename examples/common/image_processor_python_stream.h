/**
 ********************************************************************
 *
 * @copyright (c) 2023 DJI. All rights reserved.
 *
 * All information contained herein is, and remains, the property of DJI.
 * The intellectual and technical concepts contained herein are proprietary
 * to DJI and may be covered by U.S. and foreign patents, patents in process,
 * and protected by trade secret or copyright law.  Dissemination of this
 * information, including but not limited to data and other proprietary
 * material(s) incorporated within the information, in any form, is strictly
 * prohibited without the express written consent of DJI.
 *
 * If you receive this source code without DJI’s authorization, you may not
 * further disseminate the information, and you must immediately remove the
 * source code and notify DJI of its removal. DJI reserves the right to pursue
 * legal actions against you for any loss(es) or damage(s) caused by your
 * failure to do so.
 *
 *********************************************************************
 */
#ifndef EDGE_SDK_EXAMPLES_COMMON_IMAGE_PROCESSOR_PYTHON_STREAM_H_
#define EDGE_SDK_EXAMPLES_COMMON_IMAGE_PROCESSOR_PYTHON_STREAM_H_

#include <memory>
#include <mutex>
#include <string>

#include <zmq.hpp>

#include "image_processor.h"

namespace edge_app {

class ImageProcessorPythonStream : public ImageProcessor {
   public:
    explicit ImageProcessorPythonStream(std::string endpoint);
    ~ImageProcessorPythonStream() override;

    int32_t Init() override;
    void Process(const std::shared_ptr<Image> image) override;

   private:
    std::string endpoint_;
    std::unique_ptr<zmq::context_t> context_;
    std::unique_ptr<zmq::socket_t> publisher_;
    std::mutex socket_mutex_;
};

}  // namespace edge_app

#endif  // EDGE_SDK_EXAMPLES_COMMON_IMAGE_PROCESSOR_PYTHON_STREAM_H_
