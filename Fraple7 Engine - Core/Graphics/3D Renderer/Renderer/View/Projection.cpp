#include "pch.h"
#include "Projection.h"
#include "Studio/Platform/Abstract/Window.h"

namespace Fraple7
{
	namespace Core
	{
		Projection::Projection(const Window& window) :window(window)
		{

		}

		Projection::~Projection()
		{
		}
		void Projection::SetView()
		{
			//Set View (camera) matrix
			const auto eyePosition = DirectX::XMVectorSet(0, 0, -6, 1);
			const auto focusPoint = DirectX::XMVectorSet(0, 0, 0, 1);
			const auto upDirection = DirectX::XMVectorSet(0, 1, 0, 0);
			const auto view = DirectX::XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);

			// setup perspective projection matrix
			const auto aspectRatio = float(window.GetWidth()) / float(window.GetHeight());
			const auto projection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(65.f), aspectRatio, 0.1f, 100.0f);

			// combine matrices
			_mProjectionView = view * projection;
		}
	}
}
