#pragma once
#include "Utilities/Common/Common.h"
#include <directx/d3dx12.h>
namespace Fraple7
{
	namespace Core
	{
		class ResourceMgr
		{
		public:
			ResourceMgr();
			~ResourceMgr();
			template<typename Buffer>
			void Allocate(const ComPtr<ID3D12Device2>& device, Buffer& buffer, size_t size, D3D12_HEAP_TYPE type, D3D12_RESOURCE_STATES state)
			{
				const CD3DX12_HEAP_PROPERTIES heapProps{ type };
				const auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(size);
				device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, 
					state, nullptr, IID_PPV_ARGS(&buffer)) >> statusCode;
			}
		private:

		};

	}
}
