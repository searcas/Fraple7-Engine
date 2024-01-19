#pragma once
#include <DirectXMath.h>
#include "Utilities/Common/ForwardDeclarations.h"

namespace Fraple7
{
	namespace Core
	{
		class Projection
		{
		public:
			Projection(const std::shared_ptr<Studio::Window>& window);
			~Projection();
			void SetView();
			const DirectX::XMMATRIX& GetProjectionView() { return _mProjectionView; }
		private:
			DirectX::XMMATRIX _mProjectionView{};
			std::shared_ptr<Studio::WinWindow> m_Window;
		};

	}
}
