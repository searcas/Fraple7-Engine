#pragma once
#include "Graphics/3D Renderer/API/DirectX-12/Resource.h"
namespace Fraple7
{
	namespace Core
	{
		class Buffer : public Resource
		{
		public:
			explicit Buffer(const std::wstring& name = L"");
			explicit Buffer(const D3D12_RESOURCE_DESC& resDesc,
				size_t numElements, size_t elementSize,
				const std::wstring& name = L"");
			virtual void CreateViews(size_t numeElements, size_t elementSize) = 0;
		};
	}
}
