#pragma once

namespace strobe::assets {

class AssetPreprocessor {
public:
  template <typename T> void addPreprocessor() {}

  template <typename T> void preprocess();

private:
};

} // namespace strobe::assets
