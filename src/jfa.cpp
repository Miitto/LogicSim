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

    uint32_t jfaPasses =
        static_cast<uint32_t>(ceil(log2(std::max(size.width, size.height))));
    uint32_t maxJfaPasses = jfaPasses;

    std::vector<gl::Buffer> ubos;
    ubos.reserve(jfaPasses);
    for (uint32_t i = 0; i < jfaPasses; i++) {
      gl::Buffer ubo(sizeof(JfaParams), nullptr,
                     GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT |
                         GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
      ubo.map(GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
      ubos.push_back(std::move(ubo));
    }

    Programs programs{
        .toUv = std::move(toUvProgram),
        .jumpFlood = std::move(jumpFloodProgram),
    };

    gl::Texture jfaResult{};
    jfaResult.storage(1, GL_RGBA8, {size.width, size.height});
    gl::Framebuffer jfaResultFbo;
    jfaResultFbo.attachTexture(GL_COLOR_ATTACHMENT0, jfaResult);

    JfaResult result{
        .texture = std::move(jfaResult),
        .fbo = std::move(jfaResultFbo),
    };

    FlipFlops<2> flipFlops(GL_RGBA8, size);

    return Jfa(fullscreenVao, std::move(programs), std::move(flipFlops),
               std::move(ubos), std::move(result), jfaPasses, maxJfaPasses,
               window);
  }
}