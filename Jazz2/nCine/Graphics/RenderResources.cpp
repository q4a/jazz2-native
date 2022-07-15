#include "GL/GLShaderProgram.h"
#include "RenderResources.h"
#include "RenderBuffersManager.h"
#include "RenderVaoPool.h"
#include "RenderCommandPool.h"
#include "RenderBatcher.h"
#include "Camera.h"
#include "../Application.h"
#include "../Base/HashMapIterator.h"

#ifdef WITH_EMBEDDED_SHADERS
#include "shader_strings.h"
#else
#include "../IO/FileSystem.h" // for dataPath()
#endif

namespace nCine {

	///////////////////////////////////////////////////////////
	// STATIC DEFINITIONS
	///////////////////////////////////////////////////////////

	std::unique_ptr<RenderBuffersManager> RenderResources::buffersManager_;
	std::unique_ptr<RenderVaoPool> RenderResources::vaoPool_;
	std::unique_ptr<RenderCommandPool> RenderResources::renderCommandPool_;
	std::unique_ptr<RenderBatcher> RenderResources::renderBatcher_;

	std::unique_ptr<GLShaderProgram> RenderResources::spriteShaderProgram_;
	std::unique_ptr<GLShaderProgram> RenderResources::spriteGrayShaderProgram_;
	std::unique_ptr<GLShaderProgram> RenderResources::spriteNoTextureShaderProgram_;
	std::unique_ptr<GLShaderProgram> RenderResources::meshSpriteShaderProgram_;
	std::unique_ptr<GLShaderProgram> RenderResources::meshSpriteGrayShaderProgram_;
	std::unique_ptr<GLShaderProgram> RenderResources::meshSpriteNoTextureShaderProgram_;
	std::unique_ptr<GLShaderProgram> RenderResources::textnodeAlphaShaderProgram_;
	std::unique_ptr<GLShaderProgram> RenderResources::textnodeRedShaderProgram_;
	std::unique_ptr<GLShaderProgram> RenderResources::batchedSpritesShaderProgram_;
	std::unique_ptr<GLShaderProgram> RenderResources::batchedSpritesGrayShaderProgram_;
	std::unique_ptr<GLShaderProgram> RenderResources::batchedSpritesNoTextureShaderProgram_;
	std::unique_ptr<GLShaderProgram> RenderResources::batchedMeshSpritesShaderProgram_;
	std::unique_ptr<GLShaderProgram> RenderResources::batchedMeshSpritesGrayShaderProgram_;
	std::unique_ptr<GLShaderProgram> RenderResources::batchedMeshSpritesNoTextureShaderProgram_;
	std::unique_ptr<GLShaderProgram> RenderResources::batchedTextnodesRedShaderProgram_;
	std::unique_ptr<GLShaderProgram> RenderResources::batchedTextnodesAlphaShaderProgram_;

	unsigned char RenderResources::cameraUniformsBuffer_[UniformsBufferSize];
	// The `RenderCommand` class is handling insertions and rehashing
	HashMap<GLShaderProgram*, RenderResources::CameraUniformData> RenderResources::cameraUniformDataMap_(32);

	Camera* RenderResources::currentCamera_ = nullptr;
	std::unique_ptr<Camera> RenderResources::defaultCamera_;

	///////////////////////////////////////////////////////////
	// PUBLIC FUNCTIONS
	///////////////////////////////////////////////////////////

	void RenderResources::createMinimal()
	{
		//LOGI("Creating a minimal set of rendering resources...");

		const AppConfiguration& appCfg = theApplication().appConfiguration();
		buffersManager_ = std::make_unique<RenderBuffersManager>(appCfg.useBufferMapping, appCfg.vboSize, appCfg.iboSize);
		vaoPool_ = std::make_unique<RenderVaoPool>(appCfg.vaoPoolSize);

		//LOGI("Minimal rendering resources created");
	}

	///////////////////////////////////////////////////////////
	// PRIVATE FUNCTIONS
	///////////////////////////////////////////////////////////

	namespace {

		struct ShaderLoad
		{
			std::unique_ptr<GLShaderProgram>& shaderProgram;
			const char* vertexShader;
			const char* fragmentShader;
			GLShaderProgram::Introspection introspection;
		};

	}

	void RenderResources::setCamera(Camera* camera)
	{
		if (camera != nullptr)
			currentCamera_ = camera;
		else
			currentCamera_ = defaultCamera_.get();

		// The buffer is shared among every shader program. There is no need to call `setFloatVector()` as `setDirty()` is enough.
		memcpy(cameraUniformsBuffer_, currentCamera_->projection().Data(), 64);
		memcpy(cameraUniformsBuffer_ + 64, currentCamera_->view().Data(), 64);
		for (auto i = cameraUniformDataMap_.begin(); i != cameraUniformDataMap_.end(); ++i) {
			CameraUniformData& cameraUniformData = *i;

			if (cameraUniformData.camera != currentCamera_) {
				(*i).shaderUniforms.setDirty(true);
				cameraUniformData.camera = currentCamera_;
			} else {
				if (cameraUniformData.updateFrameProjectionMatrix < currentCamera_->updateFrameProjectionMatrix())
					(*i).shaderUniforms.uniform("uProjectionMatrix")->setDirty(true);
				if (cameraUniformData.updateFrameViewMatrix < currentCamera_->updateFrameViewMatrix())
					(*i).shaderUniforms.uniform("uViewMatrix")->setDirty(true);
			}

			cameraUniformData.updateFrameProjectionMatrix = currentCamera_->updateFrameProjectionMatrix();
			cameraUniformData.updateFrameViewMatrix = currentCamera_->updateFrameViewMatrix();
		}
	}

	void RenderResources::create()
	{
		//LOGI("Creating rendering resources...");

		const AppConfiguration& appCfg = theApplication().appConfiguration();
		buffersManager_ = std::make_unique<RenderBuffersManager>(appCfg.useBufferMapping, appCfg.vboSize, appCfg.iboSize);
		vaoPool_ = std::make_unique<RenderVaoPool>(appCfg.vaoPoolSize);
		renderCommandPool_ = std::make_unique<RenderCommandPool>(appCfg.vaoPoolSize);
		renderBatcher_ = std::make_unique<RenderBatcher>();
		defaultCamera_ = std::make_unique<Camera>();
		currentCamera_ = defaultCamera_.get();

		ShaderLoad shadersToLoad[] = {
	#ifndef WITH_EMBEDDED_SHADERS
			{ RenderResources::spriteShaderProgram_, "sprite_vs.glsl", "sprite_fs.glsl", GLShaderProgram::Introspection::ENABLED },
			{ RenderResources::spriteGrayShaderProgram_, "sprite_vs.glsl", "sprite_gray_fs.glsl", GLShaderProgram::Introspection::ENABLED },
			{ RenderResources::spriteNoTextureShaderProgram_, "sprite_notexture_vs.glsl", "sprite_notexture_fs.glsl", GLShaderProgram::Introspection::ENABLED },
			{ RenderResources::meshSpriteShaderProgram_, "meshsprite_vs.glsl", "sprite_fs.glsl", GLShaderProgram::Introspection::ENABLED },
			{ RenderResources::meshSpriteGrayShaderProgram_, "meshsprite_vs.glsl", "sprite_gray_fs.glsl", GLShaderProgram::Introspection::ENABLED },
			{ RenderResources::meshSpriteNoTextureShaderProgram_, "meshsprite_notexture_vs.glsl", "sprite_notexture_fs.glsl", GLShaderProgram::Introspection::ENABLED },
			{ RenderResources::textnodeAlphaShaderProgram_, "textnode_vs.glsl", "textnode_alpha_fs.glsl", GLShaderProgram::Introspection::ENABLED },
			{ RenderResources::textnodeRedShaderProgram_, "textnode_vs.glsl", "textnode_red_fs.glsl", GLShaderProgram::Introspection::ENABLED },
			{ RenderResources::batchedSpritesShaderProgram_, "batched_sprites_vs.glsl", "sprite_fs.glsl", GLShaderProgram::Introspection::NO_UNIFORMS_IN_BLOCKS },
			{ RenderResources::batchedSpritesGrayShaderProgram_, "batched_sprites_vs.glsl", "sprite_gray_fs.glsl", GLShaderProgram::Introspection::NO_UNIFORMS_IN_BLOCKS },
			{ RenderResources::batchedSpritesNoTextureShaderProgram_, "batched_sprites_notexture_vs.glsl", "sprite_notexture_fs.glsl", GLShaderProgram::Introspection::NO_UNIFORMS_IN_BLOCKS },
			{ RenderResources::batchedMeshSpritesShaderProgram_, "batched_meshsprites_vs.glsl", "sprite_fs.glsl", GLShaderProgram::Introspection::NO_UNIFORMS_IN_BLOCKS },
			{ RenderResources::batchedMeshSpritesGrayShaderProgram_, "batched_meshsprites_vs.glsl", "sprite_gray_fs.glsl", GLShaderProgram::Introspection::NO_UNIFORMS_IN_BLOCKS },
			{ RenderResources::batchedMeshSpritesNoTextureShaderProgram_, "batched_meshsprites_notexture_vs.glsl", "sprite_notexture_fs.glsl", GLShaderProgram::Introspection::NO_UNIFORMS_IN_BLOCKS },
			{ RenderResources::batchedTextnodesAlphaShaderProgram_, "batched_textnodes_vs.glsl", "textnode_alpha_fs.glsl", GLShaderProgram::Introspection::NO_UNIFORMS_IN_BLOCKS },
			{ RenderResources::batchedTextnodesRedShaderProgram_, "batched_textnodes_vs.glsl", "textnode_red_fs.glsl", GLShaderProgram::Introspection::NO_UNIFORMS_IN_BLOCKS }
	#else
			// Skipping the initial new line character of the raw string literal
			{ RenderResources::spriteShaderProgram_, ShaderStrings::sprite_vs + 1, ShaderStrings::sprite_fs + 1, GLShaderProgram::Introspection::ENABLED },
			{ RenderResources::spriteGrayShaderProgram_, ShaderStrings::sprite_vs + 1, ShaderStrings::sprite_gray_fs + 1, GLShaderProgram::Introspection::ENABLED },
			{ RenderResources::spriteNoTextureShaderProgram_, ShaderStrings::sprite_notexture_vs + 1, ShaderStrings::sprite_notexture_fs + 1, GLShaderProgram::Introspection::ENABLED },
			{ RenderResources::meshSpriteShaderProgram_, ShaderStrings::meshsprite_vs + 1, ShaderStrings::sprite_fs + 1, GLShaderProgram::Introspection::ENABLED },
			{ RenderResources::meshSpriteGrayShaderProgram_, ShaderStrings::meshsprite_vs + 1, ShaderStrings::sprite_gray_fs + 1, GLShaderProgram::Introspection::ENABLED },
			{ RenderResources::meshSpriteNoTextureShaderProgram_, ShaderStrings::meshsprite_notexture_vs + 1, ShaderStrings::sprite_notexture_fs + 1, GLShaderProgram::Introspection::ENABLED },
			{ RenderResources::textnodeAlphaShaderProgram_, ShaderStrings::textnode_vs + 1, ShaderStrings::textnode_alpha_fs + 1, GLShaderProgram::Introspection::ENABLED },
			{ RenderResources::textnodeRedShaderProgram_, ShaderStrings::textnode_vs + 1, ShaderStrings::textnode_red_fs + 1, GLShaderProgram::Introspection::ENABLED },
			{ RenderResources::batchedSpritesShaderProgram_, ShaderStrings::batched_sprites_vs + 1, ShaderStrings::sprite_fs + 1, GLShaderProgram::Introspection::NO_UNIFORMS_IN_BLOCKS },
			{ RenderResources::batchedSpritesGrayShaderProgram_, ShaderStrings::batched_sprites_vs + 1, ShaderStrings::sprite_gray_fs + 1, GLShaderProgram::Introspection::NO_UNIFORMS_IN_BLOCKS },
			{ RenderResources::batchedSpritesNoTextureShaderProgram_, ShaderStrings::batched_sprites_notexture_vs + 1, ShaderStrings::sprite_notexture_fs + 1, GLShaderProgram::Introspection::NO_UNIFORMS_IN_BLOCKS },
			{ RenderResources::batchedMeshSpritesShaderProgram_, ShaderStrings::batched_meshsprites_vs + 1, ShaderStrings::sprite_fs + 1, GLShaderProgram::Introspection::NO_UNIFORMS_IN_BLOCKS },
			{ RenderResources::batchedMeshSpritesGrayShaderProgram_, ShaderStrings::batched_meshsprites_vs + 1, ShaderStrings::sprite_gray_fs + 1, GLShaderProgram::Introspection::NO_UNIFORMS_IN_BLOCKS },
			{ RenderResources::batchedMeshSpritesNoTextureShaderProgram_, ShaderStrings::batched_meshsprites_notexture_vs + 1, ShaderStrings::sprite_notexture_fs + 1, GLShaderProgram::Introspection::NO_UNIFORMS_IN_BLOCKS },
			{ RenderResources::batchedTextnodesAlphaShaderProgram_, ShaderStrings::batched_textnodes_vs + 1, ShaderStrings::textnode_alpha_fs + 1, GLShaderProgram::Introspection::NO_UNIFORMS_IN_BLOCKS },
			{ RenderResources::batchedTextnodesRedShaderProgram_, ShaderStrings::batched_textnodes_vs + 1, ShaderStrings::textnode_red_fs + 1, GLShaderProgram::Introspection::NO_UNIFORMS_IN_BLOCKS }
	#endif
		};

		const GLShaderProgram::QueryPhase queryPhase = appCfg.deferShaderQueries ? GLShaderProgram::QueryPhase::DEFERRED : GLShaderProgram::QueryPhase::IMMEDIATE;
		const unsigned int numShaderToLoad = (sizeof(shadersToLoad) / sizeof(*shadersToLoad));
		for (unsigned int i = 0; i < numShaderToLoad; i++) {
			const ShaderLoad& shaderToLoad = shadersToLoad[i];

			shaderToLoad.shaderProgram = std::make_unique<GLShaderProgram>(queryPhase);
#ifndef WITH_EMBEDDED_SHADERS
			shaderToLoad.shaderProgram->attachShader(GL_VERTEX_SHADER, (fs::dataPath() + "Shaders/" + shaderToLoad.vertexShader).data());
			shaderToLoad.shaderProgram->attachShader(GL_FRAGMENT_SHADER, (fs::dataPath() + "Shaders/" + shaderToLoad.fragmentShader).data());
#else
			shaderToLoad.shaderProgram->attachShaderFromString(GL_VERTEX_SHADER, shaderToLoad.vertexShader);
			shaderToLoad.shaderProgram->attachShaderFromString(GL_FRAGMENT_SHADER, shaderToLoad.fragmentShader);
#endif
			shaderToLoad.shaderProgram->link(shaderToLoad.introspection);
			//FATAL_ASSERT(shaderToLoad.shaderProgram->status() != GLShaderProgram::Status::LINKING_FAILED);
		}

		// Calculating a default projection matrix for all shader programs
		const float width = theApplication().width();
		const float height = theApplication().height();
		//defaultCamera_->setOrthoProjection(0.0f, width, 0.0f, height);
		defaultCamera_->setOrthoProjection(width * (-0.5f), width * (+0.5f), height * (+0.5f), height * (-0.5f));

		//LOGI("Rendering resources created");
	}

	void RenderResources::dispose()
	{
		batchedTextnodesRedShaderProgram_.reset(nullptr);
		batchedTextnodesAlphaShaderProgram_.reset(nullptr);
		batchedMeshSpritesGrayShaderProgram_.reset(nullptr);
		batchedMeshSpritesShaderProgram_.reset(nullptr);
		batchedSpritesGrayShaderProgram_.reset(nullptr);
		batchedSpritesShaderProgram_.reset(nullptr);
		textnodeRedShaderProgram_.reset(nullptr);
		textnodeAlphaShaderProgram_.reset(nullptr);
		meshSpriteGrayShaderProgram_.reset(nullptr);
		meshSpriteShaderProgram_.reset(nullptr);
		spriteGrayShaderProgram_.reset(nullptr);
		spriteShaderProgram_.reset(nullptr);

		defaultCamera_.reset(nullptr);
		renderBatcher_.reset(nullptr);
		renderCommandPool_.reset(nullptr);
		vaoPool_.reset(nullptr);
		buffersManager_.reset(nullptr);

		//LOGI("Rendering resources disposed");
	}

}