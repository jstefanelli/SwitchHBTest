#pragma once
#include "../fix_vscode.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include "gl_buffer.hpp"
#include "gl_texture.hpp"

namespace gl {

	class TileShader {
	protected:
		GLuint id;
		GLuint vao;

		GLint aPosLoc, aUvLoc;
		GLint uMvpLoc;
		GLint uColorLoc;
		GLint uTextureLoc;
		GLint uTextureEnabledLoc;
	public:
		TileShader();
		TileShader(const TileShader&) = delete;

		TileShader& operator=(const TileShader&) = delete;

		bool Load();

		void Draw(Buffer<GL_ARRAY_BUFFER>& pos, Buffer<GL_ARRAY_BUFFER>& uv, Buffer<GL_ELEMENT_ARRAY_BUFFER>& indices, std::shared_ptr<Texture> texture,  glm::vec4 color, glm::mat4 mvp, GLint amount);

		~TileShader();
	};

	class TileData {
	public: 
		Buffer<GL_ARRAY_BUFFER> pos;
		Buffer<GL_ARRAY_BUFFER> uv;
		Buffer<GL_ELEMENT_ARRAY_BUFFER> indices;
		GLint amount;
	};

	class Tile {
	public:
		glm::vec3 position;
		glm::vec2 size;
		glm::vec4 color;
		std::shared_ptr<Texture> texture;

		Tile();
		void Draw(TileData& data, TileShader& shader, glm::mat4 vp);
	};

	class TileRenderer {
	protected:
		std::shared_ptr<TileData> data;
		std::shared_ptr<TileShader> shader;
		Tile tiles[3][3];

	public:
		TileRenderer();
		Tile* Get(unsigned int x, unsigned int y);

		void Draw(glm::ivec2 screen_size, int gap = 0);
	};
}