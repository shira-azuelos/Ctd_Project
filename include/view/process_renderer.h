#pragma once
#include "view/img.h"
#include <memory>

namespace realtime {
class RealTimeArbiter;
}

namespace view {

class ProcessRenderer {
public:
    void draw(Img& canvas, const realtime::RealTimeArbiter* arbiter);
};

}
