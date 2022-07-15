#define NCINE_INCLUDE_OPENGL
#include "../CommonHeaders.h"

#include "Texture.h"
#include "TextureLoaderRaw.h"
#include "GL/GLTexture.h"
#include "RenderStatistics.h"
#include "../ServiceLocator.h"

namespace nCine {

	GLenum ncFormatToInternal(Texture::Format format)
	{
		switch (format) {
			case Texture::Format::R8:
				return GL_R8;
			case Texture::Format::RG8:
				return GL_RG8;
			case Texture::Format::RGB8:
				return GL_RGB8;
			case Texture::Format::RGBA8:
			default:
				return GL_RGBA8;
		}
	}

	GLenum channelsToFormat(int numChannels)
	{
		switch (numChannels) {
			case 1:
				return GL_RED;
			case 2:
				return GL_RG;
			case 3:
				return GL_RGB;
			case 4:
			default:
				return GL_RGBA;
		}
	}

	///////////////////////////////////////////////////////////
	// CONSTRUCTORS and DESTRUCTOR
	///////////////////////////////////////////////////////////

	Texture::Texture()
		: Object(ObjectType::TEXTURE), glTexture_(std::make_unique<GLTexture>(GL_TEXTURE_2D)),
		width_(0), height_(0), mipMapLevels_(0), isCompressed_(false), numChannels_(0), dataSize_(0),
		minFiltering_(SamplerFilter::Nearest), magFiltering_(SamplerFilter::Nearest), wrapMode_(SamplerWrapping::ClampToEdge)
	{
	}

	/*! \note It specifies a pixel format and it is intended to be used with `loadFromTexels()` */
	Texture::Texture(const char* name, Format format, int mipMapCount, int width, int height)
		: Texture()
	{
		init(name, format, mipMapCount, width, height);
	}

	/*! \note It specifies a pixel format and it is intended to be used with `loadFromTexels()` */
	Texture::Texture(const char* name, Format format, int mipMapCount, Vector2i size)
		: Texture(name, format, mipMapCount, size.X, size.Y)
	{
	}

	/*! \note It specifies a pixel format and it is intended to be used with `loadFromTexels()` */
	Texture::Texture(const char* name, Format format, int width, int height)
		: Texture(name, format, 1, width, height)
	{
	}

	/*! \note It specifies a pixel format and it is intended to be used with `loadFromTexels()` */
	Texture::Texture(const char* name, Format format, Vector2i size)
		: Texture(name, format, 1, size.X, size.Y)
	{
	}

	/*! \note It needs a `bufferName` with a valid file extension as it loads compressed data from a file in memory */
	Texture::Texture(const char* bufferName, const unsigned char* bufferPtr, unsigned long int bufferSize)
		: Texture()
	{
		const bool hasLoaded = loadFromMemory(bufferName, bufferPtr, bufferSize);
		//if (hasLoaded == false)
		//	LOGE_X("Texture \"%s\" cannot be loaded", bufferName);
	}

	Texture::Texture(const char* filename)
		: Texture()
	{
		const bool hasLoaded = loadFromFile(filename);
		//if (hasLoaded == false)
		//	LOGE_X("Texture \"%s\" cannot be loaded", filename);
	}

	Texture::~Texture()
	{
		// Don't remove data from statistics if this is a moved out object
		if (dataSize_ > 0 && glTexture_)
			RenderStatistics::removeTexture(dataSize_);
	}

	Texture::Texture(Texture&&) = default;

	Texture& Texture::operator=(Texture&&) = default;

	///////////////////////////////////////////////////////////
	// PUBLIC FUNCTIONS
	///////////////////////////////////////////////////////////

	void Texture::init(const char* name, Format format, int mipMapCount, int width, int height)
	{

		//if (name)
		//{
		//	// When Tracy is disabled the statement body is empty and braces are needed
		//	ZoneText(name, nctl::strnlen(name, std::string::MaxCStringLength));
		//}

		TextureLoaderRaw texLoader(width, height, mipMapCount, ncFormatToInternal(format));

		if (dataSize_ > 0)
			RenderStatistics::removeTexture(dataSize_);

		glTexture_->bind();
		//setName(name);
		setGLTextureLabel(name);
		initialize(texLoader);

		RenderStatistics::addTexture(dataSize_);
	}

	void Texture::init(const char* name, Format format, int mipMapCount, Vector2i size)
	{
		init(name, format, mipMapCount, size.X, size.Y);
	}

	void Texture::init(const char* name, Format format, int width, int height)
	{
		init(name, format, 1, width, height);
	}

	void Texture::init(const char* name, Format format, Vector2i size)
	{
		init(name, format, 1, size.X, size.Y);
	}

	/*! \note It needs a `bufferName` with a valid file extension as it loads compressed data from a file in memory */
	bool Texture::loadFromMemory(const char* bufferName, const unsigned char* bufferPtr, unsigned long int bufferSize)
	{

		//if (bufferName)
		//{
		//	// When Tracy is disabled the statement body is empty and braces are needed
		//	ZoneText(bufferName, nctl::strnlen(bufferName, std::string::MaxCStringLength));
		//}

		std::unique_ptr<ITextureLoader> texLoader = ITextureLoader::createFromMemory(bufferName, bufferPtr, bufferSize);
		if (texLoader->hasLoaded() == false)
			return false;

		if (dataSize_ > 0)
			RenderStatistics::removeTexture(dataSize_);

		glTexture_->bind();
		//setName(bufferName);
		setGLTextureLabel(bufferName);
		initialize(*texLoader);
		load(*texLoader);

		RenderStatistics::addTexture(dataSize_);
		return true;
	}

	bool Texture::loadFromFile(const char* filename)
	{

		//ZoneText(filename, nctl::strnlen(filename, std::string::MaxCStringLength));

		std::unique_ptr<ITextureLoader> texLoader = ITextureLoader::createFromFile(filename);
		if (!texLoader->hasLoaded())
			return false;

		if (dataSize_ > 0)
			RenderStatistics::removeTexture(dataSize_);

		glTexture_->bind();
		//setName(filename);
		setGLTextureLabel(filename);
		initialize(*texLoader);
		load(*texLoader);

		RenderStatistics::addTexture(dataSize_);
		return true;
	}

	/*! \note It loads uncompressed pixel data from memory using the `Format` specified in the constructor */
	bool Texture::loadFromTexels(const unsigned char* bufferPtr)
	{
		return loadFromTexels(bufferPtr, 0, 0, 0, width_, height_);
	}

	/*! \note It loads uncompressed pixel data from memory using the `Format` specified in the constructor */
	bool Texture::loadFromTexels(const unsigned char* bufferPtr, unsigned int x, unsigned int y, unsigned int width, unsigned int height)
	{
		return loadFromTexels(bufferPtr, 0, x, y, width, height);
	}

	/*! \note It loads uncompressed pixel data from memory using the `Format` specified in the constructor */
	bool Texture::loadFromTexels(const unsigned char* bufferPtr, Recti region)
	{
		return loadFromTexels(bufferPtr, 0, region.X, region.Y, region.W, region.H);
	}

	/*! \note It loads uncompressed pixel data from memory using the `Format` specified in the constructor */
	bool Texture::loadFromTexels(const unsigned char* bufferPtr, unsigned int level, unsigned int x, unsigned int y, unsigned int width, unsigned int height)
	{
		const unsigned char* data = bufferPtr;

		const GLenum format = channelsToFormat(numChannels_);
		glGetError();
		glTexture_->texSubImage2D(level, x, y, width, height, format, GL_UNSIGNED_BYTE, data);
		const GLenum error = glGetError();

		return (error == GL_NO_ERROR);
	}

	/*! It loads uncompressed pixel data from memory using the `Format` specified in the constructor */
	bool Texture::loadFromTexels(const unsigned char* bufferPtr, unsigned int level, Recti region)
	{
		return loadFromTexels(bufferPtr, level, region.X, region.Y, region.W, region.H);
	}

	bool Texture::saveToMemory(unsigned char* bufferPtr)
	{
		return saveToMemory(bufferPtr, 0);
	}

	bool Texture::saveToMemory(unsigned char* bufferPtr, unsigned int level)
	{
#if !defined(WITH_OPENGLES) && !defined(__EMSCRIPTEN__)
		const GLenum format = channelsToFormat(numChannels_);
		glGetError();
		glTexture_->getTexImage(level, format, GL_UNSIGNED_BYTE, bufferPtr);
		const GLenum error = glGetError();

		return (error == GL_NO_ERROR);
#else
		return false;
#endif
	}

	void Texture::setMinFiltering(SamplerFilter filter)
	{
		GLenum glFilter = GL_NEAREST;
		// clang-format off
		switch (filter) {
			case SamplerFilter::Nearest:				glFilter = GL_NEAREST; break;
			case SamplerFilter::Linear:					glFilter = GL_LINEAR; break;
			case SamplerFilter::NearestMipmapNearest:	glFilter = GL_NEAREST_MIPMAP_NEAREST; break;
			case SamplerFilter::LinearMipmapNearest:	glFilter = GL_LINEAR_MIPMAP_NEAREST; break;
			case SamplerFilter::NearestMipmapLinear:	glFilter = GL_NEAREST_MIPMAP_LINEAR; break;
			case SamplerFilter::LinearMipmapLinear:		glFilter = GL_LINEAR_MIPMAP_LINEAR; break;
			default:									glFilter = GL_NEAREST; break;
		}
		// clang-format on

		glTexture_->bind();
		glTexture_->texParameteri(GL_TEXTURE_MIN_FILTER, glFilter);
		minFiltering_ = filter;
	}

	void Texture::setMagFiltering(SamplerFilter filter)
	{
		GLenum glFilter = GL_NEAREST;
		// clang-format off
		switch (filter) {
			case SamplerFilter::Nearest:			glFilter = GL_NEAREST; break;
			case SamplerFilter::Linear:				glFilter = GL_LINEAR; break;
			default:								glFilter = GL_NEAREST; break;
		}
		// clang-format on

		glTexture_->bind();
		glTexture_->texParameteri(GL_TEXTURE_MAG_FILTER, glFilter);
		magFiltering_ = filter;
	}

	void Texture::setWrap(SamplerWrapping wrapMode)
	{
		GLenum glWrap = GL_CLAMP_TO_EDGE;
		// clang-format off
		switch (wrapMode) {
			case SamplerWrapping::ClampToEdge:		glWrap = GL_CLAMP_TO_EDGE; break;
			case SamplerWrapping::MirroredRepeat:	glWrap = GL_MIRRORED_REPEAT; break;
			case SamplerWrapping::Repeat:			glWrap = GL_REPEAT; break;
			default:								glWrap = GL_CLAMP_TO_EDGE; break;
		}
		// clang-format on

		glTexture_->bind();
		glTexture_->texParameteri(GL_TEXTURE_WRAP_S, glWrap);
		glTexture_->texParameteri(GL_TEXTURE_WRAP_T, glWrap);
		wrapMode_ = wrapMode;
	}

	/*! The pointer is an opaque handle to be used only by ImGui or Nuklear.
	 *  It is considered immutable from an user point of view and thus retrievable by a constant method. */
	void* Texture::guiTexId() const
	{
		return const_cast<void*>(reinterpret_cast<const void*>(glTexture_.get()));
	}

	///////////////////////////////////////////////////////////
	// PRIVATE FUNCTIONS
	///////////////////////////////////////////////////////////

	void Texture::initialize(const ITextureLoader& texLoader)
	{
		const IGfxCapabilities& gfxCaps = theServiceLocator().gfxCapabilities();
		const int maxTextureSize = gfxCaps.value(IGfxCapabilities::GLIntValues::MAX_TEXTURE_SIZE);
		//FATAL_ASSERT_MSG_X(texLoader.width() <= maxTextureSize, "Texture width %d is bigger than device maximum %d", texLoader.width(), maxTextureSize);
		//FATAL_ASSERT_MSG_X(texLoader.height() <= maxTextureSize, "Texture height %d is bigger than device maximum %d", texLoader.height(), maxTextureSize);

		glTexture_->texParameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexture_->texParameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		wrapMode_ = SamplerWrapping::ClampToEdge;

		if (texLoader.mipMapCount() > 1) {
			glTexture_->texParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexture_->texParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			magFiltering_ = SamplerFilter::Linear;
			minFiltering_ = SamplerFilter::LinearMipmapLinear;
			// To prevent artifacts if the MIP map chain is not complete
			glTexture_->texParameteri(GL_TEXTURE_MAX_LEVEL, texLoader.mipMapCount());
		} else {
			glTexture_->texParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexture_->texParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			magFiltering_ = SamplerFilter::Linear;
			minFiltering_ = SamplerFilter::Linear;
		}

		const TextureFormat& texFormat = texLoader.texFormat();
		unsigned int numChannels = texFormat.numChannels();
		GLenum internalFormat = texFormat.internalFormat();
		GLenum format = texFormat.format();
		unsigned long dataSize = texLoader.dataSize();

#if (defined(WITH_OPENGLES) && GL_ES_VERSION_3_0) || defined(__EMSCRIPTEN__)
		const bool withTexStorage = true;
#else
		const bool withTexStorage = gfxCaps.hasExtension(IGfxCapabilities::GLExtensions::ARB_TEXTURE_STORAGE);
#endif

		if (withTexStorage && dataSize_ > 0 &&
			(width_ != texLoader.width() || height_ != texLoader.height() || numChannels_ != numChannels)) {
			// The OpenGL texture needs to be recreated as its storage is immutable
			glTexture_ = std::make_unique<GLTexture>(GL_TEXTURE_2D);
		}

		if (withTexStorage)
			glTexture_->texStorage2D(texLoader.mipMapCount(), internalFormat, texLoader.width(), texLoader.height());
		else if (texFormat.isCompressed() == false) {
			int levelWidth = texLoader.width();
			int levelHeight = texLoader.height();

			for (int i = 0; i < texLoader.mipMapCount(); i++) {
				glTexture_->texImage2D(i, internalFormat, levelWidth, levelHeight, format, texFormat.type(), nullptr);
				levelWidth /= 2;
				levelHeight /= 2;
			}
		}

		width_ = texLoader.width();
		height_ = texLoader.height();
		mipMapLevels_ = texLoader.mipMapCount();
		isCompressed_ = texFormat.isCompressed();
		numChannels_ = numChannels;
		dataSize_ = dataSize;
	}

	void Texture::load(const ITextureLoader& texLoader)
	{
#if (defined(WITH_OPENGLES) && GL_ES_VERSION_3_0) || defined(__EMSCRIPTEN__)
		const bool withTexStorage = true;
#else
		const IGfxCapabilities& gfxCaps = theServiceLocator().gfxCapabilities();
		const bool withTexStorage = gfxCaps.hasExtension(IGfxCapabilities::GLExtensions::ARB_TEXTURE_STORAGE);
#endif

		const TextureFormat& texFormat = texLoader.texFormat();
		int levelWidth = width_;
		int levelHeight = height_;

		GLenum format = texFormat.format();

		for (int mipIdx = 0; mipIdx < texLoader.mipMapCount(); mipIdx++) {
			const unsigned char* data = texLoader.pixels(mipIdx);

			if (texFormat.isCompressed()) {
				if (withTexStorage) {
					glTexture_->compressedTexSubImage2D(mipIdx, 0, 0, levelWidth, levelHeight, texFormat.internalFormat(), texLoader.dataSize(mipIdx), texLoader.pixels(mipIdx));
				} else {
					glTexture_->compressedTexImage2D(mipIdx, texFormat.internalFormat(), levelWidth, levelHeight, texLoader.dataSize(mipIdx), texLoader.pixels(mipIdx));
				}
			} else {
				// Storage has already been created at this point
				glTexture_->texSubImage2D(mipIdx, 0, 0, levelWidth, levelHeight, format, texFormat.type(), data);
			}

			levelWidth /= 2;
			levelHeight /= 2;
		}
	}

	void Texture::setGLTextureLabel(const char* filename)
	{
		glTexture_->setObjectLabel(filename);
	}

}