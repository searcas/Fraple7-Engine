#pragma once
#include "Utilities/Common/Common.h"
#include "directx/d3d12.h"
#include "Graphics/3D Renderer/API/DirectX-12/Device.h"

namespace Fraple7
{
	namespace Core
	{
		class UploadBuffer
		{
		public:
			struct Allocation
			{
				void* CPU;
				D3D12_GPU_VIRTUAL_ADDRESS GPU;
			};
		private:
			struct Page
			{
				Page(size_t sizeInBytes);
				~Page();
				bool HasSpace(size_t sizeInBytes, size_t alignment) const;
				Allocation Allocate(size_t sizeInBytes, size_t alignment);
				void Reset();
				ComPtr<ID3D12Resource> m_d3d12Resource;
				void* m_CPUptr;
				D3D12_GPU_VIRTUAL_ADDRESS m_GPUptr;
				size_t m_PageSize;
				size_t m_Offset;
			};
		using PagePool = std::deque<std::shared_ptr<Page>>;
		public:
			explicit UploadBuffer(size_t pageSize = _2MB);
			~UploadBuffer();
			size_t GetPageSize() const { return m_PageSize; }
			void SetPageSize(size_t newSize) { m_PageSize = newSize; }
			Allocation Allocate(size_t sizeInBytes, size_t alignment);
			void Reset();
			std::shared_ptr<Page> RegustPage();
		private:
			PagePool m_PagePool;
			PagePool m_AvailablePages;
			std::shared_ptr<Page> m_CurrentPage;
			size_t m_PageSize = 0;
		};
	}
}