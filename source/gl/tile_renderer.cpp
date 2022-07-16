#include "tile_renderer.hpp"
#include <iostream>
#include "tile_vs.h"
#include "tile_fs.h"
#include <vector>
#include <string>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

namespace gl {
	GLuint compileShader(const char* vs_source, size_t vs_length, const char* fs_source, size_t fs_length) {
		auto vsId = glCreateShader(GL_VERTEX_SHADER);

		GLint vs_size = static_cast<GLint>(vs_length);
		glShaderSource(vsId, 1, &vs_source, &vs_size);

		glCompileShader(vsId);

		GLint info_log_size = 0;
		glGetShaderiv(vsId, GL_INFO_LOG_LENGTH, &info_log_size);
		if (info_log_size > 0) {
			std::vector<char> data(0, info_log_size);
			glGetShaderInfoLog(vsId, data.size(), nullptr, &data[0]);
			std::cout << "[GL] VS Shader info log: " << std::endl << std::string(data.begin(), data.end()) << std::endl;
		}

		GLint shader_compile_status = GL_FALSE;
		glGetShaderiv(vsId, GL_COMPILE_STATUS, &shader_compile_status);
		if (shader_compile_status != GL_TRUE) {
			return 0;
		}

		auto fsId = glCreateShader(GL_FRAGMENT_SHADER);
		GLint fs_size = static_cast<GLint>(fs_length);
		glShaderSource(fsId, 1, &fs_source, &fs_size);

		glCompileShader(fsId);

		glGetShaderiv(fsId, GL_INFO_LOG_LENGTH, &info_log_size);
		if (info_log_size > 0) {
			std::vector<char> data(0, info_log_size);
			glGetShaderInfoLog(fsId, data.size(), nullptr, &data[0]);
			std::cout << "[GL] FS Shader info log: " << std::endl << std::string(data.begin(), data.end()) << std::endl;
		}

		glGetShaderiv(fsId, GL_COMPILE_STATUS, &shader_compile_status);
		if (shader_compile_status != GL_TRUE) {
			return 0;
		}

		GLuint id = glCreateProgram();
		glAttachShader(id, vsId);
		glAttachShader(id, fsId);

		glLinkProgram(id);

		glGetProgramiv(id, GL_INFO_LOG_LENGTH, &info_log_size);
		if (info_log_size > 0) {
			std::vector<char> data(0, info_log_size);
			glGetProgramInfoLog(id, data.size(), nullptr, &data[0]);
			std::cout << "[GL] Program info log: " << std::endl << std::string(data.begin(), data.end()) << std::endl;
		}

		glGetProgramiv(id, GL_LINK_STATUS, &shader_compile_status);
		if (shader_compile_status != GL_TRUE) {
			return 0;
		}

		glDeleteShader(vsId);
		glDeleteShader(fsId);

		return id;
	}

	TileShader::TileShader() : id(0) {

	}

	bool TileShader::Load() {
		id = compileShader(reinterpret_cast<const char*>(tile_vs), tile_vs_size, reinterpret_cast<const char*>(tile_fs), tile_fs_size);

		if (id != 0) {
			aPosLoc = glGetAttribLocation(id, "aPos");
			aUvLoc = glGetAttribLocation(id, "aUv");

			uMvpLoc = glGetUniformLocation(id, "uMvp");
			uColorLoc = glGetUniformLocation(id, "uColor");
			uTextureLoc = glGetUniformLocation(id, "uTexture");
			uTextureEnabledLoc = glGetUniformLocation(id, "uTextureEnabled");


			std::cout << "[GL] Shader Locs: " << aPosLoc << " " << aUvLoc << " / " << uMvpLoc << " " << uColorLoc << " " << uTextureLoc << " " << uTextureEnabledLoc << std::endl;

			return true;
		}
		std::cout << "[GL] Shader compilation failed. " << std::endl;
		return false;
	}

	void TileShader::Draw(Buffer<GL_ARRAY_BUFFER>& pos, Buffer<GL_ARRAY_BUFFER>& uv, Buffer<GL_ELEMENT_ARRAY_BUFFER>& indices, std::shared_ptr<Texture> texture, glm::vec4 color, glm::mat4 mvp, GLint amount) {
		if (id == 0) {
			return;
		}

		if (vao == 0) {
			glGenVertexArrays(1, &vao);
		}
		glBindVertexArray(vao);

		glUseProgram(id);

		glUniformMatrix4fv(uMvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));
		glUniform4f(uColorLoc, color.r, color.g, color.b, color.a);

		if (texture != nullptr && texture->Id() > 0) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture->Id());
			glUniform1i(uTextureLoc, 0);
			glUniform1i(uTextureEnabledLoc, 1);
		} else {
			glUniform1i(uTextureEnabledLoc, 0);
		}

		if (!pos.Bind()) {
			std::cout << "[GL] Failed to bind position " << std::endl;
			return;
		}

		glEnableVertexAttribArray(aPosLoc);
		glVertexAttribPointer(aPosLoc, 3, GL_FLOAT, false, 0, 0);

		if (aUvLoc >= 0) {
			if (!uv.Bind()) {
				glDisableVertexAttribArray(aPosLoc);
				std::cout << "[GL] Failed to bind uv " << std::endl;
				return;
			}

			glEnableVertexAttribArray(aUvLoc);
			glVertexAttribPointer(aUvLoc, 2, GL_FLOAT, false, 0, 0);
		}

		if (!indices.Bind()) {
			std::cout << "[GL] Failed to bind indices " << std::endl;
			glDisableVertexAttribArray(aUvLoc);
			glDisableVertexAttribArray(aPosLoc);
			return;
		}

		glDrawElements(GL_TRIANGLES, amount, GL_UNSIGNED_SHORT, 0);

		if (aUvLoc >= 0) {
			glDisableVertexAttribArray(aUvLoc);
		}
		glDisableVertexAttribArray(aPosLoc);
	}

	TileShader::~TileShader() {
		if (id != 0) {
			std::cout << "[GL] Shader: deleting shader " << id << std::endl;
			glDeleteProgram(id);
			id = 0;
		}

		if (vao != 0) {
			glDeleteVertexArrays(1, &vao);
			vao = 0;
		}
	}

	Tile::Tile() : position(0, 0, 0), size(10, 10), color(0, 1, 0, 1), texture(nullptr) {

	}

	void Tile::Draw(TileData& data, TileShader& shader, glm::mat4 vp) {
		glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
		model = glm::scale(model, glm::vec3(size.x, size.y, 1.0));
		glm::mat4 mvp = vp * model;

		shader.Draw(data.pos, data.uv, data.indices, texture, color, mvp, data.amount);
	}

	TileRenderer::TileRenderer() {
		data = std::make_shared<TileData>();
		data->pos.Load(std::vector<float>({
			-0.5f, -0.5f, 0.0f,
			0.5f, -0.5f, 0.0f,
			0.5f, 0.5f, 0.0f,
			-0.5f, 0.5f, 0.0f
		}));
		data->uv.Load(std::vector<float>({
			0.0f, 1.0f,
			1.0f, 1.0f,
			1.0f, 0.0f,
			0.0f, 0.0f
		}));
		data->indices.Load(std::vector<unsigned short>({
			0, 1, 2,
			0, 2, 3
		}));
		data->amount = 6;
		shader = std::make_shared<TileShader>();
		shader->Load();
	}

	Tile* TileRenderer::Get(unsigned int x, unsigned int y) {
		if (x < 0 || x >= 3 || y < 0 || y >= 3) {
			return nullptr;
		}

		return &tiles[y][x];
	}

	void TileRenderer::Draw(glm::ivec2 screenSize, int gap) {
		glm::mat4 vp = glm::ortho(-screenSize.x / 2.0f, screenSize.x / 2.0f, -screenSize.y / 2.0f, screenSize.y / 2.0f, 0.1f, 100.0f);

		glm::ivec2 availableSpace = (screenSize - (gap * 4)) / 3;
		glm::ivec2 cellSize = glm::ivec2(glm::min(availableSpace.x, availableSpace.y));

		for(int x = 0; x < 3; x++) {
			for(int y = 0; y < 3; y++) {
				tiles[y][x].size = cellSize;
				glm::vec3 offset;
				offset.x = (cellSize.x + gap) * (x - 1);
				offset.y = (cellSize.y + gap) * (y - 1);
				offset.z = -1;
				tiles[y][x].position = offset;

				tiles[y][x].Draw(*data, *shader, vp);
			}
		}
	}
}