#include "pch.h"
#include "Projection.h"
#include "Studio/Platform/Windows/Window.h"


namespace Fraple7
{
	namespace Core
	{
		Projection::Projection(const std::shared_ptr<Studio::Window>& window) 
			: m_Window(std::dynamic_pointer_cast<Studio::WinWindow>(window))
		{

		}

		Projection::~Projection()
		{
		}
		void Projection::SetView()
		{
			//Set View (camera) matrix
			const auto eyePosition = DirectX::XMVectorSet(0, 0, -10, 1);
			const auto focusPoint = DirectX::XMVectorSet(0, 0, 0, 1);
			const auto upDirection = DirectX::XMVectorSet(0, 1, 0, 0);
			const auto view = DirectX::XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);

			// setup perspective projection matrix
			const auto aspectRatio = float(m_Window->GetWidth()) / float(m_Window->GetHeight());
			const auto projection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(65.f), aspectRatio, 0.1f, 100.0f);

			// combine matrices
			_mProjectionView = view * projection;
		}
	}
}
