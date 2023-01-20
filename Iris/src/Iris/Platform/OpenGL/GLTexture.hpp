#pragma once
#include <glad/glad.h>

namespace Iris {
    class GLTexture {
    public:
        GLTexture();
        void loadFromFile(std::string_view path);
        void bind(GLuint unit) const;
        static void unbind(GLuint unit = 0u);
        void create(glm::ivec2 size);
        void createDepth(glm::ivec2 size);

        [[nodiscard]] GLuint getHandle() const { return m_Handle; };

        [[nodiscard]] glm::uvec2 getSize() const { return m_Size; };
    private:
        GLuint m_Handle;
        glm::uvec2 m_Size{};
    };
}

