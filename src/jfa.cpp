#include "jfa.hpp"
#include "logger.hpp"

std::optional<Jfa> Jfa::create(const gl::Vao& fullscreenVao,
                               const gl::Window& window) {
  {
    auto size = window.size();

    auto toUvProgramOpt =
        gl::Program::fromFiles({{"toUv_vert.glsl", gl::Shader::VERTEX},
                                {"toUv_frag.glsl", gl::Shader::FRAGMENT}});
    if (!toUvProgramOpt.has_value()) {
      Logger::error("Failed to load toUv program: {}", toUvProgramOpt.error());
      return std::nullopt;
    }
    auto& toUvProgram = toUvProgramOpt.value();
    auto jumpFloodProgramOpt =
        gl::Program::fromFiles({{"jumpflood_vert.glsl", gl::Shader::VERTEX},
                                {"jumpflood_frag.glsl", gl::Shader::FRAGMENT}});
    if (!jumpFloodProgramOpt.has_value()) {
      Logger::error("Failed to load jumpflood program: {}",
                    jumpFloodProgramOpt.error());
      return std::nullopt;
    }
    auto& jumpFloodProgram = jumpFloodProgramOpt.value();

    auto distanceProgramOpt =
        gl::Program::fromFiles({{"distance_vert.glsl", gl::Shader::VERTEX},
                                {"distance_frag.glsl", gl::Shader::FRAGMENT}});
    if (!distanceProgramOpt.has_value()) {
      Logger::error("Failed to load distance program: {}",
                    distanceProgramOpt.error());
      return std::nullopt;
    }
    auto& distanceProgram = distanceProgramOpt.value();

    uint32_t jfaPasses =
        static_cast<uint32_t>(ceil(log2(std::max(size.width, size.height))));
    uint32_t maxJfaPasses = jfaPasses;

    std::vector<gl::StorageBuffer> ubos;
    ubos.reserve(jfaPasses);
    for (uint32_t i = 0; i < jfaPasses; i++) {
      gl::StorageBuffer ubo(
          sizeof(JfaParams), nullptr,
          gl::Buffer::UsageBitFlag(gl::Buffer::Usage::DYNAMIC) |
              gl::Buffer::Usage::WRITE | gl::Buffer::Usage::PERSISTENT |
              gl::Buffer::Usage::COHERENT);
      ubo.map(gl::Buffer::Mapping::WRITE | gl::Buffer::Mapping::PERSISTENT |
              gl::Buffer::Mapping::COHERENT);
      ubos.push_back(std::move(ubo));
    }

    Programs programs{
        .toUv = std::move(toUvProgram),
        .jumpFlood = std::move(jumpFloodProgram),
        .distance = std::move(distanceProgram),
    };

    gl::Texture jfaResult{};
    jfaResult.storage(1, GL_RGBA32F, {size.width, size.height});
    gl::Framebuffer jfaResultFbo;
    jfaResultFbo.attachTexture(GL_COLOR_ATTACHMENT0, jfaResult);

    JfaResult result{
        .texture = std::move(jfaResult),
        .fbo = std::move(jfaResultFbo),
    };

    gl::Texture distanceResult{};
    distanceResult.storage(1, GL_RGBA32F, {size.width, size.height});
    gl::Framebuffer distanceResultFbo;
    distanceResultFbo.attachTexture(GL_COLOR_ATTACHMENT0, distanceResult);

    DistanceResult distanceRes{
        .texture = std::move(distanceResult),
        .fbo = std::move(distanceResultFbo),
    };

    FlipFlops flipFlops(GL_RGBA32F, size, 2);

    return Jfa(fullscreenVao, std::move(programs), std::move(flipFlops),
               std::move(ubos), std::move(result), std::move(distanceRes),
               jfaPasses, maxJfaPasses, window);
  }
}