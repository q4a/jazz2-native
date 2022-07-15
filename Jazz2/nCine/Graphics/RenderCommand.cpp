#include "RenderCommand.h"
#include "GL/GLShaderProgram.h"
#include "GL/GLScissorTest.h"
#include "RenderResources.h"
#include "Camera.h"

namespace nCine
{
	namespace
	{
		bool isBatchedType(Material::ShaderProgramType type)
		{
			return (type == Material::ShaderProgramType::BATCHED_SPRITES ||
					type == Material::ShaderProgramType::BATCHED_SPRITES_GRAY ||
					type == Material::ShaderProgramType::BATCHED_SPRITES_NO_TEXTURE ||
					type == Material::ShaderProgramType::BATCHED_MESH_SPRITES ||
					type == Material::ShaderProgramType::BATCHED_MESH_SPRITES_GRAY ||
					type == Material::ShaderProgramType::BATCHED_MESH_SPRITES_NO_TEXTURE ||
					type == Material::ShaderProgramType::BATCHED_TEXTNODES_ALPHA ||
					type == Material::ShaderProgramType::BATCHED_TEXTNODES_RED);
		}
	}

	///////////////////////////////////////////////////////////
	// STATIC DEFINITIONS
	///////////////////////////////////////////////////////////

	const float RenderCommand::LayerStep = 1.0f / static_cast<float>(0xFFFF);

	///////////////////////////////////////////////////////////
	// CONSTRUCTORS and DESTRUCTOR
	///////////////////////////////////////////////////////////

	RenderCommand::RenderCommand(CommandTypes profilingType)
		: materialSortKey_(0), layer_(0),
		numInstances_(0), batchSize_(0), transformationCommitted_(false),
		profilingType_(profilingType), modelMatrix_(Matrix4x4f::Identity)
	{
	}

	RenderCommand::RenderCommand()
		: RenderCommand(CommandTypes::UNSPECIFIED)
	{
	}

	///////////////////////////////////////////////////////////
	// PUBLIC FUNCTIONS
	///////////////////////////////////////////////////////////

	void RenderCommand::calculateMaterialSortKey()
	{
		const uint64_t upper = static_cast<uint64_t>(layerSortKey()) << 32;
		const uint32_t lower = material_.sortKey();
		materialSortKey_ = upper + lower;
	}

	void RenderCommand::issue()
	{
		if (geometry_.numVertices_ == 0 && geometry_.numIndices_ == 0)
			return;

		material_.bind();
		material_.commitUniforms();

		GLScissorTest::State scissorTestState = GLScissorTest::state();
		if (scissorRect_.W > 0 && scissorRect_.H > 0)
			GLScissorTest::enable(scissorRect_);

		unsigned int offset = 0;
#if (defined(WITH_OPENGLES) && !GL_ES_VERSION_3_2) || defined(__EMSCRIPTEN__)
		// Simulating missing `glDrawElementsBaseVertex()` on OpenGL ES 3.0
		if (geometry_.numIndices_ > 0)
			offset = geometry_.vboParams().offset + (geometry_.firstVertex_ * geometry_.numElementsPerVertex_ * sizeof(GLfloat));
#endif
		material_.defineVertexFormat(geometry_.vboParams().object, geometry_.iboParams().object, offset);
		geometry_.bind();
		geometry_.draw(numInstances_);

		GLScissorTest::setState(scissorTestState);
	}

	void RenderCommand::setScissor(GLint x, GLint y, GLsizei width, GLsizei height)
	{
		scissorRect_.Set(x, y, width, height);
	}

	void RenderCommand::setTransformation(const Matrix4x4f& modelMatrix)
	{
		modelMatrix_ = modelMatrix;
		transformationCommitted_ = false;
	}

	void RenderCommand::commitNodeTransformation()
	{
		if (transformationCommitted_)
			return;

		const Camera::ProjectionValues cameraValues = RenderResources::currentCamera()->projectionValues();
		modelMatrix_[3][2] = calculateDepth(layer_, cameraValues.near, cameraValues.far);

		if (material_.shaderProgram_ && material_.shaderProgram_->status() == GLShaderProgram::Status::LINKED_WITH_INTROSPECTION) {
			const Material::ShaderProgramType shaderProgramType = material_.shaderProgramType();

			if (shaderProgramType == Material::ShaderProgramType::SPRITE ||
				shaderProgramType == Material::ShaderProgramType::SPRITE_GRAY ||
				shaderProgramType == Material::ShaderProgramType::SPRITE_NO_TEXTURE)
				material_.uniformBlock("SpriteBlock")->uniform("modelMatrix")->setFloatVector(modelMatrix_.Data());
			else if (shaderProgramType == Material::ShaderProgramType::MESH_SPRITE ||
					 shaderProgramType == Material::ShaderProgramType::MESH_SPRITE_GRAY ||
					 shaderProgramType == Material::ShaderProgramType::MESH_SPRITE_NO_TEXTURE)
				material_.uniformBlock("MeshSpriteBlock")->uniform("modelMatrix")->setFloatVector(modelMatrix_.Data());
			else if (shaderProgramType == Material::ShaderProgramType::TEXTNODE_ALPHA ||
					 shaderProgramType == Material::ShaderProgramType::TEXTNODE_RED)
				material_.uniformBlock("TextnodeBlock")->uniform("modelMatrix")->setFloatVector(modelMatrix_.Data());
			else if (!isBatchedType(shaderProgramType) && shaderProgramType != Material::ShaderProgramType::CUSTOM)
				material_.uniform("modelMatrix")->setFloatVector(modelMatrix_.Data());
		}

		transformationCommitted_ = true;
	}

	void RenderCommand::commitCameraTransformation()
	{
		RenderResources::CameraUniformData* cameraUniformData = RenderResources::cameraUniformDataMap().find(material_.shaderProgram_);
		if (cameraUniformData == nullptr) {
			RenderResources::CameraUniformData newCameraUniformData;
			newCameraUniformData.shaderUniforms.setProgram(material_.shaderProgram_, "uProjectionMatrix\0uViewMatrix\0", nullptr);
			if (newCameraUniformData.shaderUniforms.numUniforms() == 2) {
				newCameraUniformData.shaderUniforms.setUniformsDataPointer(RenderResources::cameraUniformsBuffer());
				newCameraUniformData.shaderUniforms.uniform("uProjectionMatrix")->setDirty(true);
				newCameraUniformData.shaderUniforms.uniform("uViewMatrix")->setDirty(true);
				newCameraUniformData.shaderUniforms.commitUniforms();

				if (RenderResources::cameraUniformDataMap().loadFactor() >= 0.8f)
					RenderResources::cameraUniformDataMap().rehash(RenderResources::cameraUniformDataMap().capacity() * 2);
				RenderResources::cameraUniformDataMap().insert(material_.shaderProgram_, std::move(newCameraUniformData));
			}
		} else
			cameraUniformData->shaderUniforms.commitUniforms();
	}

	void RenderCommand::commitAll()
	{
		// Copy the vertices and indices stored in host memory to video memory
		/* This step is not needed if the command uses a custom VBO or IBO
		 * or directly writes into the common one */
		geometry_.commitVertices();
		geometry_.commitIndices();

		// The model matrix should always be updated before committing uniform blocks
		commitNodeTransformation();

		// Commits all the uniform blocks of command's shader program
		material_.commitUniformBlocks();
	}

	float RenderCommand::calculateDepth(uint16_t layer, float near, float far)
	{
		// The layer translates to depth, from near to far
		const float depth = near + LayerStep + (far - near - LayerStep) * layer * LayerStep;
		return depth;
	}

}