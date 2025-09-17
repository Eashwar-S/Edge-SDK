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
#include "image_processor_python_stream.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <utility>
#include <vector>

#include <opencv2/imgcodecs.hpp>

#include "logger.h"

namespace edge_app {

ImageProcessorPythonStream::ImageProcessorPythonStream(std::string endpoint)
    : endpoint_(std::move(endpoint)) {}

ImageProcessorPythonStream::~ImageProcessorPythonStream() {
    if (publisher_) {
        std::scoped_lock lock(socket_mutex_);
        try {
            publisher_->close();
        } catch (const zmq::error_t& ex) {
            ERROR("Failed to close python stream publisher: %s", ex.what());
        }
        publisher_.reset();
    }
    if (context_) {
        try {
            context_->close();
        } catch (const zmq::error_t& ex) {
            ERROR("Failed to close python stream context: %s", ex.what());
        }
        context_.reset();
    }
}

int32_t ImageProcessorPythonStream::Init() {
    try {
        context_ = std::make_unique<zmq::context_t>(1);
        publisher_ = std::make_unique<zmq::socket_t>(*context_, zmq::socket_type::pub);
        publisher_->bind(endpoint_);
    } catch (const zmq::error_t& ex) {
        ERROR("Failed to initialize python stream publisher: %s", ex.what());
        publisher_.reset();
        context_.reset();
        return -1;
    }
    return 0;
}

void ImageProcessorPythonStream::Process(const std::shared_ptr<Image> image) {
    if (!publisher_ || !image) {
        return;
    }

    std::vector<uchar> encoded;
    if (!cv::imencode(".jpg", *image, encoded)) {
        ERROR("Failed to encode frame for python stream");
        return;
    }

    uint32_t payload_size = static_cast<uint32_t>(encoded.size());
    std::array<uint8_t, sizeof(uint32_t)> header{};
    std::memcpy(header.data(), &payload_size, sizeof(uint32_t));

    try {
        std::scoped_lock lock(socket_mutex_);
        publisher_->send(zmq::buffer(header), zmq::send_flags::sndmore);
        publisher_->send(zmq::buffer(encoded), zmq::send_flags::none);
    } catch (const zmq::error_t& ex) {
        ERROR("Failed to publish frame to python stream: %s", ex.what());
    }
}

}  // namespace edge_app
