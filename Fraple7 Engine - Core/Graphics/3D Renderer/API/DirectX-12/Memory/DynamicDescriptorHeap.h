#pragma once
#include "directx/d3dx12.h"
#include "Utilities/Common/Common.h"

namespace Fraple7
{
	namespace Core
	{
		class RootSignature;
		class CommandList;
		class DynamicDescriptorHeap
		{
		public:
			DynamicDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptorsPerHeap = 1024);
			~DynamicDescriptorHeap();
			void StageDescriptors(uint32_t rootParameterIndex, uint32_t offset, uint32_t numDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE srcDescriptors);
			void CommitStagedDescriptors(CommandList& list,std::function<void(ID3D12GraphicsCommandList2*, UINT, D3D12_GPU_DESCRIPTOR_HANDLE)> setFunc);
			void CommitStagedDescriptorsRender(CommandList& list);
			void CommitStagedDescriptorsDispatch(CommandList& list);
			D3D12_GPU_DESCRIPTOR_HANDLE CopyDescriptor(CommandList& list, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor);
			void SetDescriptorHeap(CommandList& list, D3D12_DESCRIPTOR_HEAP_TYPE heapType, ID3D12DescriptorHeap* heap);
			void BindDescriptorHeaps(CommandList& list);
			void ParseRootSignature(const std::shared_ptr<RootSignature>& RootSignature);

			void Reset();
		private:
			ComPtr<ID3D12DescriptorHeap> RequestDescriptorHeap();
			ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap();
			uint32_t ComputeStaleDescriptorCount()const;

			inline static constexpr uint32_t s_MaxDescriptorTables = 32;

			struct DescriptorTableCache
			{
				DescriptorTableCache()
					: NumDescriptors(0),
					  BaseDescriptor(nullptr)
				{}
				void Reset()
				{
					NumDescriptors = 0;
					BaseDescriptor = nullptr;
				}
				uint32_t NumDescriptors;
				D3D12_CPU_DESCRIPTOR_HANDLE* BaseDescriptor;
			};

			D3D12_DESCRIPTOR_HEAP_TYPE m_DescriptorHeapType;

			uint32_t m_NumDescriptorsPerHeap;

			uint32_t m_DescriptorHandleIncremenetSize;

			std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE[]> m_DescriptorHandleCache;
			DescriptorTableCache m_DescriptorTableCache[s_MaxDescriptorTables];

			uint32_t m_DescriptorTableBitMask;

			uint32_t m_StaleDescriptorTableBitMask;

			using DescriptorHeapPool = std::queue<ComPtr<ID3D12DescriptorHeap>>;

			DescriptorHeapPool m_DescriptorHeapPool;
			DescriptorHeapPool m_AvailableDescriptorsHeaps;

			ComPtr<ID3D12DescriptorHeap> m_CurrentDescriptorHeap;
			CD3DX12_CPU_DESCRIPTOR_HANDLE m_CurrentCPUDescriptorHandle;
			CD3DX12_GPU_DESCRIPTOR_HANDLE m_CurrentGPUDescriptorHandle;

			ID3D12DescriptorHeap* m_DescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];


			uint32_t m_NumFreeHandles;
		};
	}
}