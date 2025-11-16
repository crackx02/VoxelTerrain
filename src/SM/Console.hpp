#pragma once

#include <string>
#include <format>

#include "Types.hpp"

namespace SM {
	class Console {
		public:
			enum class Color : uint16 {
				Black = 0,
				Blue = 1,
				Green = 2,
				Cyan = 3,
				Red = 4,
				Magenta = 5,
				Yellow = 6,
				LightGray = 7,
				DarkGray = 8,
				LightBlue = 9,
				LightGreen = 10,
				LightCyan = 11,
				LightRed = 12,
				LightMagenta = 13,
				LightYellow = 14,
				White = 15
			};

			static Console** _selfPtr;
			inline static Console* Get() {return *_selfPtr;};

			virtual ~Console() {};
			virtual void log(const std::string* message, Color color, int type) {};

			inline void log(const std::string& str, Color color) {log(&str, color, 32768);};
	};
}

static void _SM_Log(const std::string& str, SM::Console::Color color) {
	SM::Console::Get()->log(str, color);
}

#define SM_LOG(fmt, ...) _SM_Log(std::format(" [VoxelTerrain] " fmt, __VA_ARGS__), SM::Console::Color::LightMagenta)
#define SM_INFO(fmt, ...) _SM_Log(std::format(" [VoxelTerrain] Info: " fmt, __VA_ARGS__), SM::Console::Color::White)
#define SM_WARN(fmt, ...) _SM_Log(std::format(" [VoxelTerrain] WARNING: " fmt, __VA_ARGS__), SM::Console::Color::Yellow)
#define SM_ERROR(fmt, ...) _SM_Log(std::format(" [VoxelTerrain] ERROR: " fmt, __VA_ARGS__), SM::Console::Color::Red)
#define SM_ASSERT(expr) \
if ( !(expr) ) { \
	SM_ERROR("ASSERT: '" #expr "' : {}:{}", __FILE__, __LINE__); \
	throw std::runtime_error("ASSERT: '" #expr "'"); \
}
