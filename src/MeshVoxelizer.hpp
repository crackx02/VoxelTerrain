#pragma once

#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <thread>
#include <future>

#include "BulletCollision/CollisionShapes/btTriangleMesh.h"
#include "BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h"

#include "Types.hpp"
#include "StringHash.hpp"
#include "Texture.hpp"
#include "SM/XXHash.hpp"
#include "SM/VoxelTerrainChunk.hpp"
#include "SM/Bounds.hpp"

namespace DLL {
	class MeshVoxelizer {
		public:
			~MeshVoxelizer();

			void update();

			void initialize(bool createThread = true);

			void importMesh(
				uint16 world,
				std::string& meshPath,
				const Vec3& vCenter,
				const Quat& rotation,
				const Vec3& vHalfSize,
				std::string& texturePath,
				uint8 material,
				bool subtractive,
				float smoothingRadius
			);

			void reloadMesh(const std::string_view& meshPath);
			void reloadTexture(const std::string& texturePath);

		private:
			struct LoadedMesh {
				std::mutex mutex;
				std::vector<Vec3> vecVertices;
				std::vector<Vec2> vecTexCoords;
				btTriangleMesh* pTriMesh = nullptr;
				btBvhTriangleMeshShape* pShape = nullptr;
				std::atomic_bool wantsReload = false;
			};
			struct LoadedTexture {
				std::mutex mutex;
				Texture tex;
				std::atomic_bool wantsReload = false;
			};
			struct QueuedImport {
				std::string meshPath;
				std::string texPath;
				Quat rotation;
				Vec3 vCenter;
				Vec3 vHalfSize;
				float smoothingRadius = 1.0f;
				uint16 world = 0;
				uint8 material = 0;
				bool bSubtractive = 0;
			};
			struct ProcessedChunk {
				SM::VoxelTerrainChunk chunk;
				i32Vec3 index;
				uint16 world = 0;
				bool bSubtractive = false;
			};

			std::mutex m_mapMutex;
			StringHashMap<LoadedMesh> m_mapLoadedMeshes;
			StringHashMap<LoadedTexture> m_mapLoadedTextures;
			std::unordered_map<uint16, SM::IntBounds> m_mapWorldChunkBounds;

			std::mutex m_importQueueMutex;
			std::deque<QueuedImport> m_importQueue;

			std::mutex m_outputMutex;
			std::vector<ProcessedChunk> m_vecProcessedChunks;

			struct {
				std::condition_variable cv;
				std::future<void> handle;
				std::atomic_bool bShutdown = false;
			} m_voxelThreadData;

			LoadedMesh* m_loadMesh(const std::string& path);
			LoadedTexture* m_loadTexture(const std::string& path);
			void m_voxelThread();
	};
	extern MeshVoxelizer* gVoxelizer;
}
