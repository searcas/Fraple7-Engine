#include "pch.h"
#include "DynamicDescriptorHeap.h"
#include "Graphics/3D Renderer/API/DirectX-12/RootSignature.h"
#include "../Device.h"
namespace Fraple7
{
	namespace Core
	{
		DynamicDescriptorHeap::DynamicDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptorsPerHeap)
			:	m_DescriptorHeapType(type), 
				m_NumDescriptorsPerHeap(numDescriptorsPerHeap),
				m_DescriptorTableBitMask(0),
				m_StaleDescriptorTableBitMask(0),
				m_CurrentCPUDescriptorHandle(D3D12_DEFAULT),
				m_CurrentGPUDescriptorHandle(D3D12_DEFAULT),
				m_NumFreeHandles(0)

		{
			m_DescriptorHandleIncremenetSize = Device::GetDevice()->GetDescriptorHandleIncrementSize(type);
			m_DescriptorHandleCache = std::make_unique<D3D12_CPU_DESCRIPTOR_HANDLE[]>(m_NumDescriptorsPerHeap);
		}
		DynamicDescriptorHeap::~DynamicDescriptorHeap()
		{
		}
		void DynamicDescriptorHeap::StageDescriptors(uint32_t rootParameterIndex, uint32_t offset, uint32_t numDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE srcDescriptors)
		{
			if (numDescriptors > m_NumDescriptorsPerHeap || rootParameterIndex >= s_MaxDescriptorTables)
			{
				throw std::bad_alloc();
			}
			DescriptorTableCache& descriptorTableCache = m_DescriptorTableCache[rootParameterIndex];

			if ((offset + numDescriptors) > descriptorTableCache.NumDescriptors)
			{
				throw std::length_error("Number of descriptors exceeds the number of descriptors in the descriptor table.");
			}
			D3D12_CPU_DESCRIPTOR_HANDLE* dstDescriptor = (descriptorTableCache.BaseDescriptor + offset);
			for (size_t i = 0; i < numDescriptors; i++)
			{
				dstDescriptor[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(srcDescriptors, i, m_DescriptorHandleIncremenetSize);
			}
			m_StaleDescriptorTableBitMask |= (1 << rootParameterIndex);

		}
		// This command is not designed and meant to be called directly 
		// Should use instead Render or Dispatch
		void DynamicDescriptorHeap::CommitStagedDescriptors(Command::List& list, std::function<void(ID3D12GraphicsCommandList2*, UINT, D3D12_GPU_DESCRIPTOR_HANDLE)> setFunc)
		{
			uint32_t numDescriptorsToCommit = ComputeStaleDescriptorCount();

			if (numDescriptorsToCommit > 0)
			{
				if (!m_CurrentDescriptorHeap || m_NumFreeHandles < numDescriptorsToCommit)
				{
					m_CurrentDescriptorHeap = RequestDescriptorHeap();
					m_CurrentCPUDescriptorHandle = m_CurrentDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
					m_CurrentGPUDescriptorHandle = m_CurrentDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
					m_NumFreeHandles = m_NumDescriptorsPerHeap;
					SetDescriptorHeap(list, m_DescriptorHeapType, m_CurrentDescriptorHeap.Get());

					m_StaleDescriptorTableBitMask = m_DescriptorTableBitMask;
				}
				DWORD rootIndex;

				while (_BitScanForward(&rootIndex, m_StaleDescriptorTableBitMask))
				{
					UINT numSrcDescriptors = m_DescriptorTableCache[rootIndex].NumDescriptors;
					D3D12_CPU_DESCRIPTOR_HANDLE* pSrcDescriptionHandles = m_DescriptorTableCache[rootIndex].BaseDescriptor;

					D3D12_CPU_DESCRIPTOR_HANDLE pDestDescriptorRangeStart[] =
					{
						m_CurrentCPUDescriptorHandle
					};
					UINT pDestDescriptorRangeSizes[] =
					{
						numSrcDescriptors
					};
					Device::GetDevice()->CopyDescriptors(1, pDestDescriptorRangeStart, pDestDescriptorRangeSizes,
						numSrcDescriptors, pSrcDescriptionHandles, nullptr, m_DescriptorHeapType);
					setFunc(list.GetCommandList().Get(), rootIndex, m_CurrentGPUDescriptorHandle);

					m_CurrentCPUDescriptorHandle.Offset(numSrcDescriptors, m_DescriptorHandleIncremenetSize);
					m_CurrentGPUDescriptorHandle.Offset(numSrcDescriptors, m_DescriptorHandleIncremenetSize);
					m_NumFreeHandles -= numSrcDescriptors;

					m_StaleDescriptorTableBitMask ^= (1 << rootIndex);
				}
			}


		}
		void DynamicDescriptorHeap::CommitStagedDescriptorsRender(Command::List& list)
		{
			CommitStagedDescriptors(list, &ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable);
		}
		void DynamicDescriptorHeap::CommitStagedDescriptorsDispatch(Command::List& list)
		{
			CommitStagedDescriptors(list ,&ID3D12GraphicsCommandList::SetComputeRootDescriptorTable);
		}
		D3D12_GPU_DESCRIPTOR_HANDLE DynamicDescriptorHeap::CopyDescriptor(Command::List& list, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor)
		{
			if (!m_CurrentDescriptorHeap || m_NumFreeHandles < 1)
			{
				m_CurrentDescriptorHeap = RequestDescriptorHeap();
				m_CurrentCPUDescriptorHandle = m_CurrentDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
				m_CurrentGPUDescriptorHandle = m_CurrentDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
				m_NumFreeHandles = m_NumDescriptorsPerHeap;

				SetDescriptorHeap(list, m_DescriptorHeapType, m_CurrentDescriptorHeap.Get());
				m_StaleDescriptorTableBitMask = m_DescriptorTableBitMask;
			}
			D3D12_GPU_DESCRIPTOR_HANDLE hGPU = m_CurrentGPUDescriptorHandle;
			Device::GetDevice()->CopyDescriptorsSimple(1, m_CurrentCPUDescriptorHandle, cpuDescriptor, m_DescriptorHeapType);

			m_CurrentCPUDescriptorHandle.Offset(1, m_DescriptorHandleIncremenetSize);
			m_CurrentGPUDescriptorHandle.Offset(1, m_DescriptorHandleIncremenetSize);
			m_NumFreeHandles -= 1;
			return hGPU;
		}
		void DynamicDescriptorHeap::SetDescriptorHeap(Command::List& list, D3D12_DESCRIPTOR_HEAP_TYPE heapType, ID3D12DescriptorHeap* heap)
		{
			if (m_DescriptorHeaps[heapType] != heap)
			{
				m_DescriptorHeaps[heapType] = heap;
				BindDescriptorHeaps(list);
			}
		}
		void DynamicDescriptorHeap::BindDescriptorHeaps(Command::List& list)
		{
			UINT numDescriptorHeaps = 0;

			ID3D12DescriptorHeap* descriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] = {};

			for (size_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++)
			{
				ID3D12DescriptorHeap* descriptorHeap = m_DescriptorHeaps[i];
				if (descriptorHeap)
				{
					descriptorHeaps[numDescriptorHeaps++] = descriptorHeap;
				}
			}
			list.GetCommandList()->SetDescriptorHeaps(numDescriptorHeaps, descriptorHeaps);
		}
		void DynamicDescriptorHeap::ParseRootSignature(const std::shared_ptr<RootSignature>& rootSignature)
		{
			m_StaleDescriptorTableBitMask = 0;

			const auto& rootSignatureDesc = rootSignature->GetRootSignatureDesc();
			m_DescriptorTableBitMask = rootSignature->GetDescriptorTableBitMask(m_DescriptorHeapType);
			uint32_t descriptorTableBitMask = m_DescriptorTableBitMask;


			uint32_t currentOffset = 0;
			DWORD rootIndex;
			while (_BitScanForward(&rootIndex, descriptorTableBitMask) && rootIndex < rootSignatureDesc.NumParameters)
			{
				uint32_t numDescriptors = rootSignature->GetNumDescriptors(rootIndex);
				DescriptorTableCache& descriptorTableCache = m_DescriptorTableCache[rootIndex];
				descriptorTableCache.NumDescriptors = numDescriptors;
				descriptorTableCache.BaseDescriptor = m_DescriptorHandleCache.get() + currentOffset;

				currentOffset += numDescriptors;

				descriptorTableBitMask ^= (1 < rootIndex);
			}

			assert(currentOffset <= m_NumDescriptorsPerHeap && "The root signature requires\
																more than the maximum number \
																of descriptors per descriptor heap. \
																Consider increasing the maximum number \
																of descriptors per descriptor heap.");

		}

		void DynamicDescriptorHeap::Reset()
		{
			m_AvailableDescriptorsHeaps = m_DescriptorHeapPool;
			m_CurrentDescriptorHeap.Reset();
			m_CurrentCPUDescriptorHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(D3D12_DEFAULT);
			m_CurrentGPUDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(D3D12_DEFAULT);

			m_NumFreeHandles = 0;
			m_DescriptorTableBitMask = 0;
			m_StaleDescriptorTableBitMask = 0;
			for (size_t i = 0; i < s_MaxDescriptorTables; i++)
			{
				m_DescriptorTableCache[i].Reset();
			}
		}

		ComPtr<ID3D12DescriptorHeap> DynamicDescriptorHeap::RequestDescriptorHeap()
		{
			ComPtr<ID3D12DescriptorHeap> descriptorHeap;
			if (!m_AvailableDescriptorsHeaps.empty())
			{
				descriptorHeap = m_AvailableDescriptorsHeaps.front();
				m_AvailableDescriptorsHeaps.pop();
			}
			else
			{
				descriptorHeap = CreateDescriptorHeap();
				m_DescriptorHeapPool.push(descriptorHeap);
			}
			return descriptorHeap;
		}

		ComPtr<ID3D12DescriptorHeap> DynamicDescriptorHeap::CreateDescriptorHeap()
		{
			D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
			descriptorHeapDesc.Type = m_DescriptorHeapType;
			descriptorHeapDesc.NumDescriptors = m_NumDescriptorsPerHeap;
			descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

			ComPtr<ID3D12DescriptorHeap> descriptorHeap;
			Device::GetDevice()->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
			return descriptorHeap;
		}

		uint32_t DynamicDescriptorHeap::ComputeStaleDescriptorCount() const
		{
			uint32_t numStaleDescriptors = 0;
			DWORD i;
			DWORD staleDescriptorsBitMask = m_StaleDescriptorTableBitMask;

			while (_BitScanForward(&i, staleDescriptorsBitMask))
			{
				numStaleDescriptors += m_DescriptorTableCache[i].NumDescriptors;
				staleDescriptorsBitMask ^= (1 << i);
			}
			return numStaleDescriptors;
		}

	}
}