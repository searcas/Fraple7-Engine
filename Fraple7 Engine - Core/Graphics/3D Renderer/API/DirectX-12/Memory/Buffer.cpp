#include "pch.h"
#include "Buffer.h"

namespace Fraple7
{
	namespace Core
	{
		Buffer::Buffer(const std::wstring& name)
			: Resource(name)
		{
		}
		Buffer::Buffer(const D3D12_RESOURCE_DESC& resDesc, size_t numElements, size_t elementSize, const std::wstring& name)
			: Resource(resDesc, nullptr, name)
		{
		}
		D3D12_CPU_DESCRIPTOR_HANDLE Buffer::GetShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc) const
		{
			return D3D12_CPU_DESCRIPTOR_HANDLE();
		}
		D3D12_CPU_DESCRIPTOR_HANDLE Buffer::GetUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc) const
		{
			return D3D12_CPU_DESCRIPTOR_HANDLE();
		}
	}
}