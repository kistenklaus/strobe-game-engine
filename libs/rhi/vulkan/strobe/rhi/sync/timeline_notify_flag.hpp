#pragma once

namespace strobe::rhi {

enum class TimelineNotifyFlag {
  block,        // blocks until the commit has completed.
  backpressure, // may return before the commit has completed.
};

}
