#pragma once
#include "../fix_vscode.h"
#include <glad/glad.h>
#include <glm/glm.hpp>

namespace gl {
	class Texture {
	protected:
		GLuint id;
		glm::ivec2 size;
	public:
		Texture();

		bool LoadPNG(const uint8_t* png_data, const size_t png_data_size);
		bool AllocateRGBA(glm::ivec2 size);

		inline GLuint Id() const { return id; }
		inline glm::ivec2 Size() const { return size; }

		~Texture();
	};
}