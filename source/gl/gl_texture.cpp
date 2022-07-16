#include "gl_texture.hpp"
#include <cstring>
#include <png.h>
#include <switch.h>
#include <iostream>
#include <vector>
#include <memory>
#include <glm/gtx/string_cast.hpp>

#define pot(x) ((x != 0) && ((x & (x - 1)) == 0))

namespace gl {
	struct ReadInfo {
		const u8* buff;
		const size_t buff_size;
		const u8* ptr;

		size_t Read(void* target, size_t size) {
			if (((ptr + size) - buff) > buff_size) {
				size = (buff_size + buff) - ptr;
			}

			if (size == 0) {
				return 0;
			}

			std::memcpy(target, ptr, size);
			ptr += size;

			return size;
		}
	};


	Texture::Texture() : id(0), size(0, 0) {

	}

	void PNGReadData(png_structp png, png_bytep out_bytes, png_size_t byte_count) {
		png_voidp io_ptr = png_get_io_ptr(png);
		if (io_ptr == NULL) {
			return; //TODO: Signal error
		}

		ReadInfo* info = (ReadInfo*) io_ptr;
		const size_t bytes_copied = info->Read(out_bytes, byte_count);

		if (bytes_copied != byte_count) {
			return; //TODO: Signal error
		}
	}

	template<typename T> 
	std::unique_ptr<std::vector<T>> ReadPNGData(u32 width, u32 height, png_structp png, png_infop info, int colorType) {

		auto imageData = std::make_unique<std::vector<T>>();
		imageData->resize(width * height * 4, 0);

		const u32 items_per_row = png_get_rowbytes(png, info) / sizeof(T);
		std::vector<T> rowData;
		rowData.resize(items_per_row, 0);
		size_t itemOffset = 0;
		for(u32 rowIdx = 0; rowIdx < height; rowIdx++) {
			png_read_row(png, (u8*) &rowData[0], NULL);

			u32 i = 0;
			while(i < items_per_row) {
				switch (colorType) {
					case PNG_COLOR_TYPE_GRAY:
						(*imageData)[itemOffset++] = rowData[i];
						(*imageData)[itemOffset++] = rowData[i];
						(*imageData)[itemOffset++] = rowData[i];
						(*imageData)[itemOffset++] = 255U;
						i++;
						break;
					case PNG_COLOR_TYPE_GRAY_ALPHA:
						(*imageData)[itemOffset++] = rowData[i];
						(*imageData)[itemOffset++] = rowData[i];
						(*imageData)[itemOffset++] = rowData[i];
						i++;
						(*imageData)[itemOffset++] = rowData[i];
						i++;
						break;
					case PNG_COLOR_TYPE_RGB:
						(*imageData)[itemOffset++] = rowData[i++];
						(*imageData)[itemOffset++] = rowData[i++];
						(*imageData)[itemOffset++] = rowData[i++];
						(*imageData)[itemOffset++] = 255U;
						break;
					case PNG_COLOR_TYPE_RGB_ALPHA:
						(*imageData)[itemOffset++] = rowData[i++];
						(*imageData)[itemOffset++] = rowData[i++];
						(*imageData)[itemOffset++] = rowData[i++];
						(*imageData)[itemOffset++] = rowData[i++];
						break;
					default:
						std::cerr << "Unsupported PNG color type: " << colorType << std::endl;
						return nullptr;
				}
			}
		}

		return std::move(imageData);
	}

	bool Texture::LoadPNG(const u8* png_data, const size_t png_data_size) {
		if (id != 0)
			return false;

		ReadInfo info {
			png_data,
			png_data_size,
			png_data
		};

		u8 png_sig[8];
		info.Read(png_sig, 8);

		if (!png_check_sig(png_sig, 8)) {
			std::cerr << "Failed to initialize PNG." << std::endl;
			return false;
		}

		png_structp png_ptr = NULL;
		png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

		if (png_ptr == NULL) {
			return false;
		}

		png_infop info_ptr = NULL;
		info_ptr = png_create_info_struct(png_ptr);

		if (info_ptr == NULL) {
			std::cerr << "Failed to create PNG info." << std::endl;
			png_destroy_read_struct(&png_ptr, NULL, NULL);
			return false;
		}

		png_set_read_fn(png_ptr, &info, PNGReadData);

		png_set_sig_bytes(png_ptr, 8);

		png_read_info(png_ptr, info_ptr);

		png_uint_32 width = 0;
		png_uint_32 height = 0;
		int bitDepth = 0;
		int colorType = -1;
		png_uint_32 retval = png_get_IHDR(png_ptr, info_ptr,
			&width,
			&height,
			&bitDepth,
			&colorType,
			NULL, NULL, NULL);

		if (retval != -1) {
			std::cerr << "Failed to read PNG info." << std::endl;
			png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
			return false;
		}

		if (bitDepth != 8 || bitDepth != 16) {
			std::cerr << "Unsupported PNG bit depth:" << bitDepth << std::endl;
			png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
			return false;
		}


		if (bitDepth == 8) {
			auto imgData = ReadPNGData<u8>(width, height, png_ptr, info_ptr, colorType);
			if(imgData == nullptr) {
				png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
				return false;
			}
				
			glGenTextures(1, &id);
			glBindTexture(GL_TEXTURE_2D, id);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(*imgData)[0]);
			size.x = width;
			size.y = height;
			std::cout << "[GL] Loaded 8-bit texture " << id << " with size " << glm::to_string(size) << std::endl;
		} else if (bitDepth == 16) {
			auto imgData = ReadPNGData<u16>(width, height, png_ptr, info_ptr, colorType);
			if(imgData == nullptr) {
				png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
				return false;
			}

			glGenTextures(1, &id);
			glBindTexture(GL_TEXTURE_2D, id);
			size.x = width;
			size.y = height;
			glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_SHORT, &(*imgData)[0]);
			std::cout << "[GL] Loaded 16-bit texture " << id << " with size " << glm::to_string(size) << std::endl;
		}

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		if (pot(width) && pot(height)) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glGenerateMipmap(GL_TEXTURE_2D);
		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return true;
	}

	bool Texture::AllocateRGBA(glm::ivec2 size) {
		if (id != 0)
			return false;
		
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		this->size = size;
		std::cout << "[GL] Allocated texture " << id << " with size " << glm::to_string(size) << std::endl;

		if (pot(size.x) && pot(size.y)) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glGenerateMipmap(GL_TEXTURE_2D);
		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}

		return true;
	}

	Texture::~Texture() {
		if (id != 0) {
			std::cout << "[GL] Deleting texture: " << id << std::endl;
			glDeleteTextures(1, &id);
			id = 0;
			size.x = 0;
			size.y = 0;
		}
	}
}