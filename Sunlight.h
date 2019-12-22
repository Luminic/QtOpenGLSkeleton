#include "Light.h"

class Sunlight : public Light {
  Q_OBJECT

public:
  Sunlight(glm::vec3 position=glm::vec3(0.0f), glm::vec3 scale=glm::vec3(1.0f), glm::vec3 color=glm::vec3(1.0f), float ambient=0.2f, float diffuse=1.0f, float specular=1.0f);
  ~Sunlight();

  void initialize_depth_framebuffer(unsigned int depth_map_width=1024, unsigned int depth_map_height=1024);

  unsigned int depth_framebuffer;
  unsigned int depth_map;

  unsigned int depth_map_width;
  unsigned int depth_map_height;
};
