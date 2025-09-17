#include <unistd.h>

#include "logger.h"
#include "sample_liveview.h"

using namespace edge_sdk;
using namespace edge_app;

ErrorCode ESDKInit();

int main(int argc, char **argv) {
    // 1. Initialize the Edge SDK before using any APIs.
    auto rc = ESDKInit();
    if (rc != kOk) {
        ERROR("pre init failed");
        return -1;
    }

    // 2. Set up liveview for the RGB (wide) camera.
    auto rgb_liveview = std::make_shared<LiveviewSample>("WideCamera");
    StreamDecoder::Options decoder_option = {.name = std::string("ffmpeg")};
    auto rgb_decoder = CreateStreamDecoder(decoder_option);
    ImageProcessor::Options rgb_processor_option = {
        .name = std::string("display"),
        .alias = "RGB",
        .userdata = rgb_liveview
    };
    auto rgb_processor = CreateImageProcessor(rgb_processor_option);
    if (0 != InitLiveviewSample(rgb_liveview, Liveview::kCameraTypePayload,
                                Liveview::kStreamQuality1080p,
                                rgb_decoder, rgb_processor)) {
        ERROR("Init RGB liveview failed");
        return -1;
    }
    rgb_liveview->SetCameraSource(Liveview::kCameraSourceWide);
    rgb_liveview->Start();

    // 3. Set up liveview for the thermal (IR) camera.
    auto ir_liveview = std::make_shared<LiveviewSample>("IRCamera");
    auto ir_decoder = CreateStreamDecoder(decoder_option);
    ImageProcessor::Options ir_processor_option = {
        .name = std::string("display"),
        .alias = "IR",
        .userdata = ir_liveview
    };
    auto ir_processor = CreateImageProcessor(ir_processor_option);
    if (0 != InitLiveviewSample(ir_liveview, Liveview::kCameraTypePayload,
                                Liveview::kStreamQuality1080p,
                                ir_decoder, ir_processor)) {
        ERROR("Init IR liveview failed");
        return -1;
    }
    ir_liveview->SetCameraSource(Liveview::kCameraSourceIR);
    ir_liveview->Start();

    // 4. Keep the application running so both streams remain active.
    while (1) {
        sleep(3);
    }
    return 0;
}
