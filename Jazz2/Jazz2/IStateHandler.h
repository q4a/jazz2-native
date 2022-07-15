﻿#pragma once

#include "../nCine/Input/InputEvents.h"

namespace Jazz2
{
	class IStateHandler
	{
	public:
		IStateHandler() { }
		virtual ~IStateHandler() { }

		virtual void OnFrameStart() { }
		virtual void OnRootViewportResized(int width, int height) { }

		virtual void OnKeyPressed(const nCine::KeyboardEvent& event) { }
		virtual void OnKeyReleased(const nCine::KeyboardEvent& event) { }
		
	private:
		/// Deleted copy constructor
		IStateHandler(const IStateHandler&) = delete;
		/// Deleted assignment operator
		IStateHandler& operator=(const IStateHandler&) = delete;

	};
}