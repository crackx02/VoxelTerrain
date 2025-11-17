
#define TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_USE_MAPBOX_EARCUT
#include "tiny_obj_loader.h"

#include "MeshVoxelizer.hpp"
#include "VoxelUtils.hpp"
#include "VoxelizerUtils.hpp"
#include "SM/DirectoryManager.hpp"
#include "SM/VoxelTerrainManager.hpp"
#include "SM/Console.hpp"

using namespace DLL;
using namespace VoxelizerUtils;
using namespace SM;

MeshVoxelizer* DLL::gVoxelizer = nullptr;

MeshVoxelizer::~MeshVoxelizer() {
	gVoxelizer = nullptr;
	initialize(false);
}

void MeshVoxelizer::update() {
	std::scoped_lock lock(m_outputMutex);
	if ( m_vecProcessedChunks.empty() )
		return;

	VoxelTerrainManager* pVoxelManager = VoxelTerrainManager::Get();
	if ( pVoxelManager == nullptr )
		return;

	VoxelTerrainWorld* pCurrentWorld = nullptr;
	uint16 currentWorld = 0;
	IntBounds bounds = {{0, 0, 0}, {0, 0, 0}};
	for ( const ProcessedChunk& chunk : m_vecProcessedChunks ) {
		if ( chunk.world != currentWorld ) {
			currentWorld = chunk.world;
			pCurrentWorld = pVoxelManager->getWorld(currentWorld);
			if ( pCurrentWorld == nullptr )
				continue;
			bounds = pCurrentWorld->getChunkBounds();
		}
		if ( pCurrentWorld == nullptr )
			continue;

		if ( glm::any(glm::lessThan(chunk.index, bounds.min)) || glm::any(glm::greaterThan(chunk.index, bounds.max)) )
			continue;

		VoxelTerrainChunk* pChunk = pCurrentWorld->getOrCreateChunk(chunk.index, chunk.bSubtractive);
		if ( pChunk == nullptr )
			continue;

		if ( chunk.bSubtractive ) {
			for ( uint16 i = 0; i < sizeof(VoxelTerrainChunk::voxels); ++i ) {
				const uint8 thisVox = pChunk->voxels[i];
				const uint8 newVox = chunk.chunk.voxels[i];
				if ( (thisVox & 0b00111111) > (newVox & 0b00111111) )
					pChunk->voxels[i] = (thisVox & 0b11000000) | (newVox & 0b00111111);
			}
		} else {
			for ( uint16 i = 0; i < sizeof(VoxelTerrainChunk::voxels); ++i ) {
				const uint8 thisVox = pChunk->voxels[i];
				const uint8 newVox = chunk.chunk.voxels[i];
				if ( (thisVox & 0b00111111) < (newVox & 0b00111111) )
					pChunk->voxels[i] = newVox;
			}

			const glm::bvec3 vMinEdges = {
				chunk.index.x == bounds.min.x,
				chunk.index.y == bounds.min.y,
				chunk.index.z == bounds.min.z
			};
			const glm::bvec3 vMaxEdges = {
				chunk.index.x == bounds.max.x,
				chunk.index.y == bounds.max.y,
				chunk.index.z == bounds.max.z
			};
			ClearChunkEdgeVoxels(*pChunk, vMinEdges, vMaxEdges);
		}

		pCurrentWorld->updateChunk(chunk.index);
	}
	m_vecProcessedChunks.clear();
}

void MeshVoxelizer::initialize(bool createThread) {
	m_voxelThreadData.bShutdown = !createThread;
	m_voxelThreadData.cv.notify_one();
	if ( m_voxelThreadData.handle.joinable() )
		if (m_voxelThreadData.closeThread)
			m_voxelThreadData.handle.join();
		else
			// Thread is already dead on process shutdown, detach to avoid destructor deadlock
			m_voxelThreadData.handle.detach();

	m_vecProcessedChunks.clear();
	m_importQueue.clear();
	m_mapWorldChunkBounds.clear();
	m_mapLoadedTextures.clear();
	for ( auto& [path, mesh] : m_mapLoadedMeshes ) {
		delete mesh.pShape;
		delete mesh.pTriMesh;
	}
	m_mapLoadedMeshes.clear();

	if ( createThread )
		m_voxelThreadData.handle = std::thread(
			[this](){
				try {
					SM_LOG("Voxelizer START");
					m_voxelThread();
				} catch ( const std::exception& e ) {
					SM_ERROR("Voxelizer EXCEPTION: {}", e.what());
				}
			}
		);
}

void MeshVoxelizer::importMesh(
	uint16 world,
	std::string& meshPath,
	const Vec3& vCenter,
	const Quat& rotation,
	const Vec3& vHalfSize,
	std::string& texturePath,
	uint8 material,
	bool subtractive,
	float smoothingRadius
) {
	VoxelTerrainWorld* pWorld = VoxelTerrainManager::Get()->getWorld(world);
	if ( pWorld != nullptr ) {
		std::scoped_lock lock(m_mapMutex);
		m_mapWorldChunkBounds.try_emplace(world, pWorld->getChunkBounds());
	}
	std::scoped_lock lock(m_importQueueMutex);
	m_importQueue.push_front(QueuedImport(std::move(meshPath), std::move(texturePath), rotation, vCenter, vHalfSize, smoothingRadius, world, material, subtractive));	
	m_voxelThreadData.cv.notify_one();
}

void MeshVoxelizer::reloadMesh(const std::string_view& meshPath) {
	std::scoped_lock lock(m_mapMutex);
	auto it = m_mapLoadedMeshes.find(meshPath);
	if ( it == m_mapLoadedMeshes.end() ) {
		SM_ERROR("reloadMesh failed to find mesh '{}'", meshPath);
		return;
	}
	it->second.wantsReload = true;
}

void MeshVoxelizer::reloadTexture(const std::string& texturePath) {
	std::scoped_lock lock(m_mapMutex);
	auto it = m_mapLoadedTextures.find(texturePath);
	if ( it == m_mapLoadedTextures.end() ) {
		SM_ERROR("reloadTexture failed to find texture '{}'", texturePath);
		return;
	}
	it->second.wantsReload = true;
}



MeshVoxelizer::LoadedMesh* MeshVoxelizer::m_loadMesh(const std::string& path) {
	SM_LOG("Loading mesh {}", path);

	std::string obj;
	if ( !DirectoryManager::Get()->readFile(path, obj) ) {
		SM_ERROR("Failed to read mesh file '{}'", path);
		return nullptr;
	}

	tinyobj::ObjReaderConfig cfg;
	cfg.triangulate = true;
	cfg.triangulation_method = "earcut";
	cfg.vertex_color = false;

	tinyobj::ObjReader reader;
	if ( !reader.ParseFromString(obj, "", cfg) ) {
		SM_ERROR("Failed to parse mesh file '{}': {}", path, reader.Error());
		return nullptr;
	}

	if ( !reader.Warning().empty() )
		SM_WARN("TinyObj: {}", reader.Warning());

	const auto& attrib = reader.GetAttrib();
	const auto& shapes = reader.GetShapes();

	if ( shapes.size() == 0 ) {
		SM_WARN("Mesh '{}' contains no shapes", path);
		return nullptr;
	}

	if ( shapes.size() != 1 )
		SM_WARN("More than 1 shape found in mesh '{}'! This is currently not supported and only shape #1 will be used.", path);

	LoadedMesh newMesh;

	const tinyobj::shape_t& shape = shapes[0];
	for ( uint64 iFace = 0; iFace < shape.mesh.num_face_vertices.size(); ++iFace ) {
		const uint64 vertexBase = iFace * 3;
		for ( uint32 iVert = 0; iVert < 3; ++iVert ) {
			const tinyobj::index_t& idx = shape.mesh.indices[vertexBase + iVert];
			newMesh.vecVertices.emplace_back(Vec3(
				attrib.vertices[3 * idx.vertex_index + 0],
				attrib.vertices[3 * idx.vertex_index + 1],
				attrib.vertices[3 * idx.vertex_index + 2]
			));

			if ( idx.texcoord_index >= 0 ) {
				newMesh.vecTexCoords.emplace_back(glm::clamp(Vec2(
					attrib.texcoords[2 * idx.texcoord_index + 0],
					attrib.texcoords[2 * idx.texcoord_index + 1]
				), Vec2(0.0f), Vec2(1.0f)));
			}
		}
	}

	Bounds bounds = {Vec3(BT_INFINITY), Vec3(-BT_INFINITY)};
	for ( const Vec3& v : newMesh.vecVertices ) {
		bounds.min = glm::min(v, bounds.min);
		bounds.max = glm::max(v, bounds.max);
	}

	// Make sure the mesh is centered around (0, 0, 0)
	const Vec3 vCenterOffset = bounds.min + bounds.max;
	bounds.min -= vCenterOffset;
	bounds.max -= vCenterOffset;
	const Vec3 vSize = bounds.max - bounds.min;
	for ( Vec3& v : newMesh.vecVertices )
		v = ((v - vCenterOffset) / vSize);

	newMesh.pTriMesh = new btTriangleMesh();
	for ( const Vec3* pv = newMesh.vecVertices.data(); pv < newMesh.vecVertices.data() + newMesh.vecVertices.size(); pv += 3 ) {
		newMesh.pTriMesh->addTriangle(
			ToBT(*(pv + 0)),
			ToBT(*(pv + 1)),
			ToBT(*(pv + 2))
		);
	}

	newMesh.pShape = new btBvhTriangleMeshShape(newMesh.pTriMesh, true);

	auto it = m_mapLoadedMeshes.try_emplace(path);
	LoadedMesh& mesh = it.first->second;
	mesh.vecVertices = std::move(newMesh.vecVertices);
	mesh.vecTexCoords = std::move(newMesh.vecTexCoords);
	if ( mesh.pShape != nullptr )
		delete mesh.pShape;
	if ( mesh.pTriMesh != nullptr )
		delete mesh.pTriMesh;
	mesh.pTriMesh = newMesh.pTriMesh;
	mesh.pShape = newMesh.pShape;
	mesh.wantsReload = false;
	return &mesh;
}

MeshVoxelizer::LoadedTexture* MeshVoxelizer::m_loadTexture(const std::string& path) {
	SM_LOG("Loading texture '{}'", path);
	Texture tex(path);
	if ( !tex.valid() ) {
		SM_ERROR("Failed to load texture '{}'", path);
		return nullptr;
	}

	auto it = m_mapLoadedTextures.try_emplace(path);
	it.first->second.tex = std::move(tex);
	it.first->second.wantsReload = false;
	return &it.first->second;
}

void MeshVoxelizer::m_voxelThread() {
	auto& td = m_voxelThreadData;
	while ( !td.bShutdown ) {
		std::unique_lock lock(m_importQueueMutex);
		td.cv.wait(lock, [this, &td]{return !m_importQueue.empty() || td.bShutdown;});

		if ( m_importQueue.empty() )
			continue;

		QueuedImport imp;

		while ( !td.bShutdown ) {
			if ( !lock.owns_lock() )
				lock.lock();
			if ( m_importQueue.empty() )
				break;
			imp = std::move(m_importQueue.back());
			m_importQueue.pop_back();
			lock.unlock();

			const LoadedMesh* pMesh = nullptr;
			const LoadedTexture* pTexture = nullptr;
			IntBounds worldChunkBounds = {i32Vec3(INT_MIN, INT_MIN, INT_MIN), i32Vec3(INT_MAX, INT_MAX, INT_MAX)};
			{
				std::scoped_lock lockMap(m_mapMutex);
				auto itMesh = m_mapLoadedMeshes.find(imp.meshPath);
				if ( itMesh == m_mapLoadedMeshes.end() || itMesh->second.wantsReload )
					pMesh = m_loadMesh(imp.meshPath);
				else
					pMesh = &itMesh->second;
				if ( pMesh == nullptr )
					continue;

				if ( !imp.texPath.empty() ) {
					auto itTex = m_mapLoadedTextures.find(imp.texPath);
					if ( itTex == m_mapLoadedTextures.end() || itTex->second.wantsReload )
						pTexture = m_loadTexture(imp.texPath);
					else
						pTexture = &itTex->second;
					if ( pTexture == nullptr )
						continue;
				}

				auto itBounds = m_mapWorldChunkBounds.find(imp.world);
				SM_ASSERT(itBounds != m_mapWorldChunkBounds.end());
				worldChunkBounds = itBounds->second;
			}

			const Vec3& vHalfSize = imp.vHalfSize;
			const Vec3 vSize = vHalfSize * 2.0f;

			const Mat33 mRot = glm::mat3_cast(imp.rotation);
			const Vec3 vHalfSizeRotated = glm::abs(mRot[0] * vHalfSize.x) + glm::abs(mRot[1] * vHalfSize.y) + glm::abs(mRot[2] * vHalfSize.z);

			const Bounds worldBounds = {
				imp.vCenter - vHalfSizeRotated,
				imp.vCenter + vHalfSizeRotated
			};

			IntBounds chunkBounds = {
				i32Vec3(glm::floor(worldBounds.min / float(VoxelConstants::MetersPerChunkAxis))),
				i32Vec3(glm::floor(worldBounds.max / float(VoxelConstants::MetersPerChunkAxis)))
			};
			chunkBounds.min = glm::clamp(chunkBounds.min, worldChunkBounds.min, chunkBounds.max);
			chunkBounds.max = glm::clamp(chunkBounds.max, chunkBounds.min, worldChunkBounds.max);

			const Vec3 vChunkMinWorld = Vec3(chunkBounds.min) * float(VoxelConstants::MetersPerChunkAxis);
			const Vec3 vOriginNorm = worldBounds.min - vChunkMinWorld;
			const Vec3 vCenterNorm = imp.vCenter - vChunkMinWorld;
			const Quat invRot = -imp.rotation;
			const bool bSubtractive = imp.bSubtractive;
			const float smoothRadius = glm::clamp(imp.smoothingRadius, 0.05f, 2.0f);
			const float surfaceRadius = glm::clamp(smoothRadius, 1.0f, 2.0f);
			const float surfaceRadius2 = surfaceRadius * surfaceRadius;
			const float smoothRadius2 = smoothRadius * smoothRadius;

			pMesh->pShape->setLocalScaling(ToBT(vSize));

			#pragma omp parallel for collapse(3)
			for ( int cx = chunkBounds.min.x; cx <= chunkBounds.max.x; ++cx ) {
				for ( int cy = chunkBounds.min.y; cy <= chunkBounds.max.y; ++cy ) {
					for ( int cz = chunkBounds.min.z; cz <= chunkBounds.max.z; ++cz ) {
						const Vec3 vChunkIndexNorm = i32Vec3(cx, cy, cz) - chunkBounds.min;
				
						ProcessedChunk chunk;
						chunk.index = {cx, cy, cz};
						chunk.bSubtractive = bSubtractive;
						chunk.world = imp.world;
						std::vector<IndexedTriangle> vecTempTriangles;

						for ( uint8 vz = 0; vz < VoxelConstants::VoxelsPerChunkAxis; ++vz ) {
							for ( uint8 vy = 0; vy < VoxelConstants::VoxelsPerChunkAxis; ++vy ) {
								for ( uint8 vx = 0; vx < VoxelConstants::VoxelsPerChunkAxis; ++vx ) {
									const i32Vec3 vVoxelIndex = {vx, vy, vz};
									const Vec3 vVoxelWorldNorm = Vec3(vx, vy, vz) + vChunkIndexNorm * float(VoxelConstants::MetersPerChunkAxis);
									const Vec3 vVoxelLocalNorm = invRot * (vVoxelWorldNorm - vCenterNorm);

									if (
										glm::any(glm::lessThanEqual(vVoxelLocalNorm, -vHalfSize)) ||
										glm::any(glm::greaterThanEqual(vVoxelLocalNorm, vHalfSize))
										) {
										chunk.chunk.voxels[VoxelIndex3To1(vVoxelIndex)] =
											(VoxelConstants::MaxVoxelDensity * uint8(bSubtractive)) | (imp.material & 0b11000000);
										continue;
									}

									uint8 numRays = 0;
									uint8 numInsideHits = 0;
									for ( const Vec3& dir : {
											Vec3(vSize.x + 1.0f, 0, 0),
											Vec3(-vSize.x - 1.0f, 0, 0),
											Vec3(0.0f, vSize.y + 1.0f, 0),
											Vec3(0.0f, -vSize.y - 1.0f, 0),
											Vec3(0.0f, 0.0f, vSize.z + 1.0f),
											Vec3(0.0f, 0.0f, -vSize.z - 1.0f),
										} ) {
										RaycastHitCountCallback rayCallback(
											vVoxelLocalNorm,
											vVoxelLocalNorm + dir
										);
										pMesh->pShape->performRaycast(&rayCallback, rayCallback.vRayStart, rayCallback.vRayEnd);
										++numRays;
										numInsideHits += (rayCallback.numHits % 2 == 1);
									}
									const bool bInside = numInsideHits > numRays / 2;

									TriangleFetchCallback triCallback(vecTempTriangles);
									pMesh->pShape->processAllTriangles(
										&triCallback,
										ToBT(vVoxelLocalNorm - Vec3(surfaceRadius)),
										ToBT(vVoxelLocalNorm + Vec3(surfaceRadius))
									);

									float closestSurfaceDistance2 = surfaceRadius2;
									float closestSmoothingDistance2 = smoothRadius2;
									Vec3 vClosestPoint = {0.0f, 0.0f, 0.0f};
									int closestTriIndex = -1;
									for ( const IndexedTriangle& tri : vecTempTriangles ) {
										vClosestPoint = FindClosestPointOnTriangle(vVoxelLocalNorm, tri.v0, tri.v1, tri.v2);
										float dist2 = glm::abs(glm::length2(vClosestPoint - vVoxelLocalNorm));
										if ( dist2 <= closestSmoothingDistance2 )
											closestSmoothingDistance2 = dist2;
										if ( dist2 <= closestSurfaceDistance2 ) {
											closestSurfaceDistance2 = dist2;
											closestTriIndex = tri.index;
										}
									}
									vecTempTriangles.clear();

									const float smoothDistanceNorm = glm::clamp(closestSmoothingDistance2 / smoothRadius2, 0.0f, 1.0f);

									uint8 density = bSubtractive ? VoxelConstants::MaxVoxelDensity : 0;
									if ( bInside ) {
										density = uint8(
											(bSubtractive ? 1.0f - smoothDistanceNorm : smoothDistanceNorm) *
											float(VoxelConstants::MaxVoxelDensity)
										) & 0b00111111;
									}

									uint8 material = imp.material;
									if ( closestTriIndex != -1 && pTexture ) {
										const uint64 baseVertex = closestTriIndex * 3;
										const uint64 maxVertex = baseVertex + 2;
										if ( pMesh->vecTexCoords.size() > maxVertex ) {
											SM_ASSERT(pMesh->vecVertices.size() > maxVertex);
											SM_ASSERT((baseVertex + 0) < pMesh->vecVertices.size());
											SM_ASSERT((baseVertex + 1) < pMesh->vecVertices.size());
											SM_ASSERT((baseVertex + 2) < pMesh->vecVertices.size());
											const Vec3 v0 = pMesh->vecVertices.at(baseVertex + 0);
											const Vec3 v1 = pMesh->vecVertices.at(baseVertex + 1);
											const Vec3 v2 = pMesh->vecVertices.at(baseVertex + 2);
											const Vec3& t0 = pTexture->tex.getCoord(pMesh->vecTexCoords.at(baseVertex + 0));
											const Vec3& t1 = pTexture->tex.getCoord(pMesh->vecTexCoords.at(baseVertex + 1));
											const Vec3& t2 = pTexture->tex.getCoord(pMesh->vecTexCoords.at(baseVertex + 2));

											const Vec3 a = v1 - v0;
											const Vec3 b = v2 - v0;
											const Vec3 c = vClosestPoint - v0;
											const float d00 = glm::dot(a, a);
											const float d01 = glm::dot(a, b);
											const float d11 = glm::dot(b, b);
											const float d20 = glm::dot(c, a);
											const float d21 = glm::dot(c, b);
											const float denom = d00 * d11 - d01 * d01;
											const float v = (d11 * d20 - d01 * d21) / denom;
											const float w = (d00 * d21 - d01 * d20) / denom;
											const float u = 1.0f - v - w;

											const Vec3 tex = u * t0 + v * t1 + w * t2;

											if ( tex.x >= tex.y && tex.x >= tex.z )
												material = uint8(0) << 6;
											else if ( tex.y >= tex.x && tex.y >= tex.z )
												material = uint8(1) << 6;
											else
												material = uint8(2) << 6;
										}
									}
									chunk.chunk.voxels[VoxelIndex3To1(vVoxelIndex)] = density | (material & 0b11000000);
								}
							}
						}

						{
							std::scoped_lock lock(m_outputMutex);
							m_vecProcessedChunks.emplace_back(chunk);
						}
					}
				}
			}
		}
	}
	td.bShutdown = false;
}
