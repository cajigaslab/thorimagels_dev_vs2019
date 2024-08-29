#pragma once

struct Publisher {
  virtual void publish_image(int width, int height, int channels, int bytes_per_channel, unsigned char* memory) = 0;
};