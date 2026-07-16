#pragma once
#include "view/img.h"
#include <memory>

namespace realtime {
class RealTimeArbiter;
}

namespace view {

class ProcessRenderer {
private:
    struct ProcessItem {
        std::string text;
        cv::Scalar color;
    };
    void draw_process_list(Img& canvas, const std::vector<ProcessItem>& processes, int x_offset);

public:
    void draw(Img& canvas, const realtime::RealTimeArbiter* arbiter);
};

}
