#pragma once

#define NCINE_INCLUDE_OPENGL
#include "../CommonHeaders.h"

#include "../Primitives/Matrix4x4.h"
#include "GL/GLShaderUniforms.h"
#include "../Base/HashMap.h"

#include <memory>

namespace nCine
{
	class RenderBuffersManager;
	class RenderVaoPool;
	class RenderCommandPool;
	class RenderBatcher;
	class GLShaderProgram;
	class Camera;

	/// The class that creates and handles application common OpenGL rendering resources
	class RenderResources
	{
	public:
		/// A vertex format structure for vertices with positions only
		struct VertexFormatPos2
		{
			GLfloat position[2];
		};

		/// A vertex format structure for vertices with positions and texture coordinates
		struct VertexFormatPos2Tex2
		{
			GLfloat position[2];
			GLfloat texcoords[2];
		};

		/// A vertex format structure for vertices with positions and draw indices
		struct VertexFormatPos2Index
		{
			GLfloat position[2];
			int drawindex;
		};

		/// A vertex format structure for vertices with positions, texture coordinates and draw indices
		struct VertexFormatPos2Tex2Index
		{
			GLfloat position[2];
			GLfloat texcoords[2];
			int drawindex;
		};

		struct CameraUniformData
		{
			CameraUniformData()
				: camera(nullptr), updateFrameProjectionMatrix(0), updateFrameViewMatrix(0) {}

			GLShaderUniforms shaderUniforms;
			Camera* camera;
			unsigned long int updateFrameProjectionMatrix;
			unsigned long int updateFrameViewMatrix;
		};

		static inline RenderBuffersManager& buffersManager() {
			return *buffersManager_;
		}
		static inline RenderVaoPool& vaoPool() {
			return *vaoPool_;
		}
		static inline RenderCommandPool& renderCommandPool() {
			return *renderCommandPool_;
		}
		static inline RenderBatcher& renderBatcher() {
			return *renderBatcher_;
		}

		static inline GLShaderProgram* spriteShaderProgram() {
			return spriteShaderProgram_.get();
		}
		static inline GLShaderProgram* spriteGrayShaderProgram() {
			return spriteGrayShaderProgram_.get();
		}
		static inline GLShaderProgram* spriteNoTextureShaderProgram() {
			return spriteNoTextureShaderProgram_.get();
		}
		static inline GLShaderProgram* meshSpriteShaderProgram() {
			return meshSpriteShaderProgram_.get();
		}
		static inline GLShaderProgram* meshSpriteGrayShaderProgram() {
			return meshSpriteGrayShaderProgram_.get();
		}
		static inline GLShaderProgram* meshSpriteNoTextureShaderProgram() {
			return meshSpriteNoTextureShaderProgram_.get();
		}
		static inline GLShaderProgram* textnodeAlphaShaderProgram() {
			return textnodeAlphaShaderProgram_.get();
		}
		static inline GLShaderProgram* textnodeRedShaderProgram() {
			return textnodeRedShaderProgram_.get();
		}
		static inline GLShaderProgram* batchedSpritesShaderProgram() {
			return batchedSpritesShaderProgram_.get();
		}
		static inline GLShaderProgram* batchedSpritesGrayShaderProgram() {
			return batchedSpritesGrayShaderProgram_.get();
		}
		static inline GLShaderProgram* batchedSpritesNoTextureShaderProgram() {
			return batchedSpritesNoTextureShaderProgram_.get();
		}
		static inline GLShaderProgram* batchedMeshSpritesShaderProgram() {
			return batchedMeshSpritesShaderProgram_.get();
		}
		static inline GLShaderProgram* batchedMeshSpritesGrayShaderProgram() {
			return batchedMeshSpritesGrayShaderProgram_.get();
		}
		static inline GLShaderProgram* batchedMeshSpritesNoTextureShaderProgram() {
			return batchedMeshSpritesNoTextureShaderProgram_.get();
		}
		static inline GLShaderProgram* batchedTextnodesAlphaShaderProgram() {
			return batchedTextnodesAlphaShaderProgram_.get();
		}
		static inline GLShaderProgram* batchedTextnodesRedShaderProgram() {
			return batchedTextnodesRedShaderProgram_.get();
		}

		static inline unsigned char* cameraUniformsBuffer() {
			return cameraUniformsBuffer_;
		}
		static inline HashMap<GLShaderProgram*, CameraUniformData>& cameraUniformDataMap() {
			return cameraUniformDataMap_;
		}
		static inline const Camera* currentCamera() {
			return currentCamera_;
		}

		static void createMinimal();

	private:
		static std::unique_ptr<RenderBuffersManager> buffersManager_;
		static std::unique_ptr<RenderVaoPool> vaoPool_;
		static std::unique_ptr<RenderCommandPool> renderCommandPool_;
		static std::unique_ptr<RenderBatcher> renderBatcher_;

		static std::unique_ptr<GLShaderProgram> spriteShaderProgram_;
		static std::unique_ptr<GLShaderProgram> spriteGrayShaderProgram_;
		static std::unique_ptr<GLShaderProgram> spriteNoTextureShaderProgram_;
		static std::unique_ptr<GLShaderProgram> meshSpriteShaderProgram_;
		static std::unique_ptr<GLShaderProgram> meshSpriteGrayShaderProgram_;
		static std::unique_ptr<GLShaderProgram> meshSpriteNoTextureShaderProgram_;
		static std::unique_ptr<GLShaderProgram> textnodeAlphaShaderProgram_;
		static std::unique_ptr<GLShaderProgram> textnodeRedShaderProgram_;
		static std::unique_ptr<GLShaderProgram> batchedSpritesShaderProgram_;
		static std::unique_ptr<GLShaderProgram> batchedSpritesGrayShaderProgram_;
		static std::unique_ptr<GLShaderProgram> batchedSpritesNoTextureShaderProgram_;
		static std::unique_ptr<GLShaderProgram> batchedMeshSpritesShaderProgram_;
		static std::unique_ptr<GLShaderProgram> batchedMeshSpritesGrayShaderProgram_;
		static std::unique_ptr<GLShaderProgram> batchedMeshSpritesNoTextureShaderProgram_;
		static std::unique_ptr<GLShaderProgram> batchedTextnodesAlphaShaderProgram_;
		static std::unique_ptr<GLShaderProgram> batchedTextnodesRedShaderProgram_;

		static constexpr int UniformsBufferSize = 128; // two 4x4 float matrices
		static unsigned char cameraUniformsBuffer_[UniformsBufferSize];
		static HashMap<GLShaderProgram*, CameraUniformData> cameraUniformDataMap_;

		static Camera* currentCamera_;
		static std::unique_ptr<Camera> defaultCamera_;

		static void setCamera(Camera* camera);
		static void create();
		static void dispose();

		/// Static class, deleted constructor
		RenderResources() = delete;
		/// Static class, deleted copy constructor
		RenderResources(const RenderResources& other) = delete;
		/// Static class, deleted assignement operator
		RenderResources& operator=(const RenderResources& other) = delete;

		/// The `Application` class needs to create and dispose the resources
		friend class Application;
		/// The `Viewport` class needs to set the current camera
		friend class Viewport;
	};

}