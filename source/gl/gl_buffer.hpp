#pragma once
#include "../fix_vscode.h"
#include <glad/glad.h>
#include <vector>
#include <iostream>

namespace gl {
	template<GLenum slot>
	class Buffer {
	protected:
		GLuint id;
		GLsizeiptr size;
	public:
		Buffer() : id(0), size(0) {

		}
		Buffer(const Buffer& other) = delete;
		const Buffer& operator=(const Buffer& other) = delete;

		template<typename T>
		bool Load(std::vector<T> data, GLenum usage_hint = GL_STATIC_DRAW) {
			if (id != 0)
				return false;

			glGenBuffers(1, &id);
			glBindBuffer(slot, id);
			size = sizeof(T) * data.size();
			std::cout << "Buffer: Generating buffer: " << id << " with size " << size << std::endl;
			glBufferData(slot, size, &data[0], usage_hint);
			
			return true;
		}
		bool Allocate(GLsizeiptr size, GLenum usage_hint = GL_STATIC_DRAW) {
			if (id != 0)
				return false;

			glGenBuffers(1, &id);
			glBindBuffer(slot, id);
			this->size = size;
			glBufferData(slot, size, nullptr, usage_hint);

			return true;
		}
		template<typename T>
		bool Update(std::vector<T> data, size_t offset) {
			if (id == 0)
				return false;

			GLsizeiptr sz = sizeof(T) * data.size();
			if (sz + offset > size) {
				return false;
			}

			glBindBuffer(slot, id);
			glBufferSubData(slot, offset, sz, &data[0]);
		}

		bool Bind() {
			if (id == 0)
				return false;

			glBindBuffer(slot, id);
			return true;
		}

		~Buffer() {
			if (id != 0) {
				std::cout << "Buffer: deleting buffer " << id << std::endl;
				glDeleteBuffers(1, &id);
			}
		}
	};
}