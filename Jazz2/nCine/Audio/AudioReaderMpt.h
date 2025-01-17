#pragma once

#include "../../Common.h"

#if defined(DEATH_TARGET_WINDOWS) && !defined(CMAKE_BUILD)
#	define WITH_OPENMPT
#endif

#ifdef WITH_OPENMPT

#include "IAudioReader.h"
#include "../IO/IFileStream.h"

#if defined(_MSC_VER) && defined(__has_include)
#	if __has_include("../../../Libs/libopenmpt/libopenmpt.h")
#		define __HAS_LOCAL_LIBOPENMPT
#	endif
#endif
#ifdef __HAS_LOCAL_LIBOPENMPT
#	include "../../../Libs/libopenmpt/libopenmpt.h"
#else
#	include <libopenmpt.h>
#endif

#include <memory>

namespace nCine
{
	class IFile;

	class AudioReaderMpt : public IAudioReader
	{
	public:
		AudioReaderMpt(std::unique_ptr<IFileStream> fileHandle, int frequency);
		~AudioReaderMpt();

		unsigned long int read(void* buffer, unsigned long int bufferSize) const override;
		void rewind() const override;
		void setLooping(bool isLooping) override;

	private:
		/// Audio file handle
		std::unique_ptr<IFileStream> _fileHandle;
		int _frequency;
		openmpt_module* _module;

		static size_t stream_read_func(void* stream, void* dst, size_t bytes);
		static int stream_seek_func(void* stream, int64_t offset, int whence);
		static int64_t stream_tell_func(void* stream);

		static void InternalLog(const char* message, void* user);

		/// Deleted copy constructor
		AudioReaderMpt(const AudioReaderMpt&) = delete;
		/// Deleted assignment operator
		AudioReaderMpt& operator=(const AudioReaderMpt&) = delete;
	};
}

#endif