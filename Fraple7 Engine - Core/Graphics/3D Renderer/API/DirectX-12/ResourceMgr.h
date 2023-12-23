#pragma once
#include "Utilities/Common/Common.h"
#include <directx/d3dx12.h>
namespace Fraple7
{
	namespace Core
	{
		class ResourceMgr
		{
			ResourceMgr();
			~ResourceMgr();
		public:
			template<typename Buffer>
			static void Allocate(const ComPtr<ID3D12Device2>& device, Buffer& buffer, size_t size, D3D12_HEAP_TYPE type = D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_FLAGS flags = D3D12_HEAP_FLAG_NONE)
			{
				const CD3DX12_HEAP_PROPERTIES heapProps{ type };
				if (size != NULL)
				{
					const auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(size);
					device->CreateCommittedResource(&heapProps, flags, &resourceDesc,
						state, nullptr, IID_PPV_ARGS(&buffer)) >> statusCode;
				}
			}
			template<typename Buffer>
			static void Allocate(const ComPtr<ID3D12Device2>& device, Buffer& buffer, const D3D12_RESOURCE_DESC& resDesc, D3D12_HEAP_TYPE type = D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_FLAGS flags = D3D12_HEAP_FLAG_NONE)
			{
				const CD3DX12_HEAP_PROPERTIES heapProps{ type };
				device->CreateCommittedResource(&heapProps, flags, &resDesc,
					state, nullptr, IID_PPV_ARGS(&buffer)) >> statusCode;
			}
		};

	}
}
