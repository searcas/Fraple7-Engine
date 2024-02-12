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
			virtual D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc = nullptr) const override;
			virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc = nullptr) const override;
		};
	}
}
