#pragma once
#include <DirectXMath.h>
namespace Fraple7
{
	namespace Core
	{
		class Projection
		{
		public:
			Projection(const class Window& window);
			~Projection();
			void SetView();
			const DirectX::XMMATRIX& GetProjectionView() { return _mProjectionView; }
		private:
			DirectX::XMMATRIX _mProjectionView{};
			const Window& window;
		};

	}
}
