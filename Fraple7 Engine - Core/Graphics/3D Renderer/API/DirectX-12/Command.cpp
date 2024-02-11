#include "pch.h"
#include "Command.h"
#include "Utilities/Common/Common.h"
#include "Utilities/Math/Math.h"
#include "Resource.h"
#include "Memory/UploadBuffer.h"
#include "ResourceStateTracker.h"
#include "Resource.h"
#include "Memory/DynamicDescriptorHeap.h"
#include "RootSignature.h"
#include "Memory/VertexBuffer.h"
#include "Memory/IndexBuffer.h"
#include "RenderTarget.h"
#include "GenerateMipsPSO.h"
#include "PanoToCubeMapPSO.h"
namespace Fraple7
{
	namespace Core
	{
		CommandMgr::CommandMgr()
		{
			m_CommandQueueCopy = std::make_shared<Command::QueueDx>(D3D12_COMMAND_LIST_TYPE_COPY);
			m_CommandQueueDirect = std::make_shared<Command::QueueDx>(D3D12_COMMAND_LIST_TYPE_DIRECT);
			m_CommandQueueCompute = std::make_shared<Command::QueueDx>(D3D12_COMMAND_LIST_TYPE_COMPUTE);
		}

		CommandMgr::~CommandMgr()
		{
			UnloadAll();
		}
		const std::shared_ptr<Command::QueueDx>& CommandMgr::GetCommandQueue(D3D12_COMMAND_LIST_TYPE type) const
		{
			switch (type)
			{
			case D3D12_COMMAND_LIST_TYPE_DIRECT:
				return m_CommandQueueDirect;
				break;
			case D3D12_COMMAND_LIST_TYPE_COMPUTE:
				return m_CommandQueueCompute;
				break;
			case D3D12_COMMAND_LIST_TYPE_COPY:
				return m_CommandQueueCopy;
				break;
			default:
				throw std::runtime_error{ "Failed Command Queue Type" };
				break;
			}
		}

		void CommandMgr::UnloadAll()
		{
			m_CommandQueueCopy->SignalAndWait();
			m_CommandQueueDirect->SignalAndWait();
			m_CommandQueueCompute->SignalAndWait();
		}


		Command::QueueDx::~QueueDx()
		{
			m_CommandQueue->Signal(m_Fence.Get(), ++m_FenceVal) >> statusCode;
			m_Fence->SetEventOnCompletion(m_FenceVal, m_FenceEvent) >> statusCode;

			if (WaitForSingleObject(m_FenceEvent, 2000) == WAIT_FAILED)
			{
				GetLastError() >> statusCode;
			}
		}
		Command::QueueDx::QueueDx(D3D12_COMMAND_LIST_TYPE commandListType) : 
			m_CommandListType(commandListType)
		{
			const D3D12_COMMAND_QUEUE_DESC desc = {
				.Type = commandListType,
				.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
				.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
				.NodeMask = 0
			};
			m_Desc = desc;
			CreateCommandQueue();
			CreateFence();
			CreateAnEvent();
		}
		ComPtr<ID3D12GraphicsCommandList2> Command::QueueDx::GetCommandList()
		{
			if (!m_CommandAllocatorQueue.empty() && IsFenceReached(m_CommandAllocatorQueue.front().fenceValue))
			{
				m_CommandAllocator = m_CommandAllocatorQueue.front().commandAllocator;
				m_CommandAllocatorQueue.pop();
				m_CommandAllocator->Reset() >> statusCode;
			}
			else
			{
				CreateCommandAllocatator();
			}
			if (!m_CommandListQueue.empty())
			{
				m_CommandList = m_CommandListQueue.front();
				m_CommandListQueue.pop();
				m_CommandList->Reset(m_CommandAllocator.Get(), nullptr) >> statusCode;
			}
			else
			{
				CreateCommandList();
			}
			// Bind command allocator with command list 
			// so we can get when command list is executed
			m_CommandList->SetPrivateDataInterface(__uuidof(ID3D12CommandAllocator), m_CommandAllocator.Get()) >> statusCode;
			return m_CommandList;
		}
		std::shared_ptr<Command::List> Command::QueueDx::GetCommandListClass()
		{
			std::shared_ptr<Command::List> commandList;
			if (!m_AvailableCommandLists.Empty())
			{
				m_AvailableCommandLists.TryPop(commandList);
			}
			else
			{
				commandList = std::make_shared<Command::List>(m_CommandListType);
			}
			return commandList;
		}
		void Command::QueueDx::CreateAnEvent()
		{
			m_FenceEvent = CreateEventW(nullptr, FALSE, FALSE, FALSE);
			if (!m_FenceEvent)
			{
				GetLastError() >> statusCode;
				throw std::runtime_error{ "Failed to create fence event" };
			}
		}
		void Command::QueueDx::CreateFence()
		{
			Device::GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)) >> statusCode;
		}
		void Command::QueueDx::WaitForFenceCompletion(uint64_t fenceValue)
		{
			if (!IsFenceReached(fenceValue))
			{
				m_Fence->SetEventOnCompletion(m_FenceVal, m_FenceEvent) >> statusCode;
				if (::WaitForSingleObject(m_FenceEvent, DWORD_MAX) == WAIT_FAILED)
				{
					GetLastError() >> statusCode;
				}
			}
		}
		uint64_t Command::QueueDx::Signal() const
		{
			m_CommandQueue->Signal(m_Fence.Get(), ++m_FenceVal) >> statusCode;
			return m_FenceVal;
		}
		uint64_t Command::QueueDx::SignalAndWait()
		{
			uint64_t signal = Signal();
			WaitForFenceCompletion(signal);
			return signal;
		}
		uint64_t Command::QueueDx::ExecuteCommandList(const ComPtr<ID3D12GraphicsCommandList2>& commandList)
		{
			commandList->Close();
			ID3D12CommandAllocator* commandAllocator;
			UINT dataSize = sizeof(commandAllocator);

			commandList->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, &commandAllocator) >> statusCode;

			ID3D12CommandList* const pComandLists[] = { commandList.Get() };
			m_CommandQueue->ExecuteCommandLists(1, pComandLists);

			uint64_t fenceValue = Signal();

			m_CommandAllocatorQueue.emplace(CommandAllocatorBundle{ fenceValue, commandAllocator });
			m_CommandListQueue.push(commandList);

			// We can safe release command allocator
			// Since we transfered in the command allocator queue.
			m_CommandAllocator->Release();
			return fenceValue;
		}
		uint32_t Command::QueueDx::CreateCommandQueue()
		{
			Device::GetDevice()->CreateCommandQueue(&m_Desc, IID_PPV_ARGS(&m_CommandQueue)) >> statusCode;
			return FPL_SUCCESS;
		}

		bool Command::QueueDx::IsFenceReached(uint64_t fenceVal)
		{
			return m_Fence->GetCompletedValue() >= fenceVal;
		}

		void Command::QueueDx::CreateCommandAllocatator()
		{
			Device::GetDevice()->CreateCommandAllocator(m_CommandListType, IID_PPV_ARGS(&m_CommandAllocator)) >> statusCode;
		}
		void Command::QueueDx::CreateCommandList()
		{
			Device::GetDevice()->CreateCommandList(0, m_CommandListType,
				m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_CommandList)) >> statusCode;
		}
		void Command::QueueDx::Join(const ComPtr<ID3D12GraphicsCommandList2>& commandList, const ComPtr<ID3D12Resource>& dst, const ComPtr<ID3D12Resource>& src)
		{
			commandList->CopyResource(dst.Get(), src.Get());
		}
		void Command::QueueDx::Join(const ComPtr<ID3D12GraphicsCommandList2>& commandList, const ComPtr<ID3D12Resource>& dst, const ComPtr<ID3D12Resource>& src, size_t size, const std::vector<D3D12_SUBRESOURCE_DATA>& srcData)
		{
			UpdateSubresources(commandList.Get(), dst.Get(), src.Get(), 0, 0, (UINT)size, srcData.data());
		}
		void Command::QueueDx::Transition(const ComPtr<ID3D12Resource>& buffer, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to)
		{
			const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(buffer.Get(), from, to);
			m_CommandList->ResourceBarrier(1, &barrier);
		}

		Command::List::List(D3D12_COMMAND_LIST_TYPE type)
			: m_CommandListType(type)
		{
			Device::GetDevice()->CreateCommandAllocator(m_CommandListType, IID_PPV_ARGS(&m_CommandAllocator)) >> statusCode;
			Device::GetDevice()->CreateCommandList(0, m_CommandListType, m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_CommandList));

			m_UploadBuffer = std::make_unique<UploadBuffer>();

			m_ResourceStateTracker = std::make_unique<ResourceStateTracker>();

			for (size_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++)
			{
				m_DynamicDescriptorHeap[i] = std::make_unique<DynamicDescriptorHeap>(static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i));
				m_DescriptorHeaps[i] = nullptr;
			}

		}

		Command::List::~List()
		{
		}

		void Command::List::TransitionBarrier(ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource, bool flushBarriers)
		{
			if (resource)
			{
				auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), D3D12_RESOURCE_STATE_COMMON, stateAfter, subResource);
				m_ResourceStateTracker->ResourceBarrier(barrier);
			}
			if (flushBarriers)
			{
				FlushResourceBarriers();
			}
		}
		void Command::List::TransitionBarrier(const Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource, bool flushBarriers)
		{
			TransitionBarrier(resource.GetD3D12Resource(), stateAfter, subResource, flushBarriers);
		}

		void Command::List::CopyResource(Resource& dstRest, const Resource& srcRes)
		{
			TransitionBarrier(dstRest, D3D12_RESOURCE_STATE_COPY_DEST);
			TransitionBarrier(dstRest, D3D12_RESOURCE_STATE_COPY_DEST);

			FlushResourceBarriers();
			m_CommandList->CopyResource(dstRest.GetD3D12Resource().Get(), srcRes.GetD3D12Resource().Get());
			TrackResource(dstRest);
			TrackResource(srcRes);
		}
		void Command::List::CopyResource(ComPtr<ID3D12Resource> dstRes, ComPtr<ID3D12Resource> src)
		{

		}
		void Command::List::UAVBarrier(ComPtr<ID3D12Resource> resource, bool flushBarriers)
		{
			auto barrier = CD3DX12_RESOURCE_BARRIER::UAV(resource.Get());
			m_ResourceStateTracker->ResourceBarrier(barrier);

			if (flushBarriers)
			{
				FlushResourceBarriers();
			}
		}
		void Command::List::UAVBarrier(const Resource& resource, bool flushBarriers)
		{
			UAVBarrier(resource.GetD3D12Resource(), flushBarriers);
		}
		void Command::List::AliasingBarrier(const Resource& beforeResource, const Resource& afterResource, bool flushBarriers)
		{
			AliasingBarrier(beforeResource.GetD3D12Resource(), afterResource.GetD3D12Resource(), flushBarriers);
		}
		void Command::List::AliasingBarrier(ComPtr<ID3D12Resource> beforeRes, ComPtr<ID3D12Resource> afterRes, bool flushBarriers)
		{
			auto barrier = CD3DX12_RESOURCE_BARRIER::Aliasing(beforeRes.Get(), afterRes.Get());
			m_ResourceStateTracker->ResourceBarrier(barrier);

			if (flushBarriers)
			{
				FlushResourceBarriers();
			}

		}
		void Command::List::FlushResourceBarriers()
		{
			m_ResourceStateTracker->FlushResourceBarriers(*this);
		}
		void Command::List::SetGraphicsDynamicConstantBuffer(uint32_t rootParameterIndex, size_t sizeInBytes, const void* data)
		{
			auto heapAllocation = m_UploadBuffer->Allocate(sizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
			memcpy(heapAllocation.CPU, data, sizeInBytes);
			m_CommandList->SetGraphicsRootConstantBufferView(rootParameterIndex, heapAllocation.GPU);
		}

		void Command::List::SetGraphics32BitConstants(uint32_t rootParameterIndex, uint32_t numConstants, const void* constants)
		{
		}

		void Command::List::SetCompute32BitConstants(uint32_t rootParameterIndex, uint32_t numConstants, const void* constants)
		{
		}

		void Command::List::SetPipelineState(ComPtr<ID3D12PipelineState> pipelineState)
		{
			m_CommandList->SetPipelineState(pipelineState.Get());
			TrackResource(pipelineState);
		}
		void Command::List::SetGraphicsRootSignature(const std::shared_ptr<RootSignature>& rootSignature)
		{
			auto d3d12RootSignature = rootSignature->GetRootSignature();

			if (m_RootSignature->GetRootSignature().Get() != d3d12RootSignature.Get())
			{
				m_RootSignature->SetRootSignature(d3d12RootSignature);

				for (size_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++)
				{
					m_DynamicDescriptorHeap[i]->ParseRootSignature(rootSignature);
				}
				m_CommandList->SetGraphicsRootSignature(m_RootSignature->GetRootSignature().Get());
				TrackResource(m_RootSignature->GetRootSignature().Get());
			}

		}
		void Command::List::SetComputeRootSignature(const std::shared_ptr<RootSignature>& rootSignature)
		{
			auto d3d12RootSignature = rootSignature->GetRootSignature();

			if (m_RootSignature->GetRootSignature().Get() != d3d12RootSignature.Get())
			{
				m_RootSignature->SetRootSignature(d3d12RootSignature);

				for (size_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++)
				{
					m_DynamicDescriptorHeap[i]->ParseRootSignature(rootSignature);
				}

				m_CommandList->SetComputeRootSignature(m_RootSignature->GetRootSignature().Get());
				TrackResource(m_RootSignature->GetRootSignature().Get());
			}
		}
		void Command::List::SetViewPort(const D3D12_VIEWPORT& viewport)
		{
		}
		void Command::List::SetViewPorts(const std::vector<D3D12_VIEWPORT>& viewports)
		{
		}
		void Command::List::SetScissorRect(const D3D12_RECT& scissorRect)
		{
			SetScissorRect({ scissorRect });
		}
		void Command::List::SetScissorRects(const std::vector<D3D12_RECT>& scissorRects)
		{
			assert(scissorRects.size() < D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE);
			m_CommandList->RSSetScissorRects(static_cast<UINT>(scissorRects.size()), scissorRects.data());
		}
		void Command::List::SetShaderResourceView(uint32_t rootParameterIndex, uint32_t descriptorOffset, const Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT firstSubResource, UINT numSubResources, const D3D12_SHADER_RESOURCE_VIEW_DESC* srv)
		{
			if (numSubResources < D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
			{
				for (uint32_t i = 0; i < numSubResources; i++)
				{
					TransitionBarrier(resource, stateAfter, firstSubResource + i);
				}
			}
			else
			{
				TransitionBarrier(resource, stateAfter);

			}
			m_DynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageDescriptors(rootParameterIndex, descriptorOffset, 1, resource.GetShaderResourceView(srv));

			TrackResource(resource);
		}
		void Command::List::SetUnorderedAccessView(uint32_t rootParameterIndex,
			uint32_t descriptorOffset,
			const Resource& resource,
			D3D12_RESOURCE_STATES stateAfter,
			UINT firstSubResource,
			UINT numSubResources,
			const D3D12_UNORDERED_ACCESS_VIEW_DESC* uav)
		{
			if (numSubResources < D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
			{
				for (uint32_t i = 0; i < numSubResources; i++)
				{
					TransitionBarrier(resource, stateAfter, firstSubResource + i);
				}
			}
			else
			{
				TransitionBarrier(resource, stateAfter);

			}
			m_DynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageDescriptors(rootParameterIndex, descriptorOffset, 1, resource.GetUnorderedAccessView(uav));

			TrackResource(resource);
		}
		void Command::List::SetRenderTarget(const RenderTarget& renderTarget)
		{
			std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> renderTargetDescriptors;
			renderTargetDescriptors.reserve(AttachmentPoint::NumAttachmentPoints);

			const auto& textures = renderTarget.GetTextures();

			for (size_t i = 0; i < 0x00000008; i++)
			{
				auto& texture = textures[i];
				if (texture.IsValid())
				{
					TransitionBarrier(texture, D3D12_RESOURCE_STATE_RENDER_TARGET);
					renderTargetDescriptors.push_back(texture.GetRenderTargetView());
					TrackResource(texture);
				}
			}
			const auto& depthTexture = renderTarget.GetTexture(AttachmentPoint::DepthStencil);

			CD3DX12_CPU_DESCRIPTOR_HANDLE depthstencilDescriptor(D3D12_DEFAULT);

			if (depthTexture.GetD3D12Resource())
			{
				TransitionBarrier(depthTexture, D3D12_RESOURCE_STATE_DEPTH_WRITE);
				depthstencilDescriptor = depthTexture.GetDepthStencilView();

				TrackResource(depthTexture);
			}
			D3D12_CPU_DESCRIPTOR_HANDLE* pDSV = depthstencilDescriptor.ptr != 0 ? &depthstencilDescriptor : nullptr;
			m_CommandList->OMSetRenderTargets(static_cast<UINT>(renderTargetDescriptors.size()), renderTargetDescriptors.data(), FALSE, pDSV);
		}
		void Command::List::Render(uint32_t vertexCount, uint32_t instanceCount, uint32_t startVertex, uint32_t startInstance)
		{
			FlushResourceBarriers();

			for (size_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++)
			{
				m_DynamicDescriptorHeap[i]->CommitStagedDescriptorsRender(*this);
			}
			m_CommandList->DrawInstanced(vertexCount, instanceCount, startVertex, startInstance);
		}
		void Command::List::RenderIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t startIndex, int32_t baseVertex, uint32_t startInstance)
		{
			FlushResourceBarriers();

			for (size_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++)
			{
				m_DynamicDescriptorHeap[i]->CommitStagedDescriptorsRender(*this);
			}
			m_CommandList->DrawIndexedInstanced(indexCount, instanceCount, startIndex, baseVertex, startInstance);
		}
		void Command::List::Dispatch(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ)
		{
			FlushResourceBarriers();

			for (size_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++)
			{
				m_DynamicDescriptorHeap[i]->CommitStagedDescriptorsRender(*this);
			}
			m_CommandList->Dispatch(numGroupsX, numGroupsY, numGroupsZ);
		}
		bool Command::List::Close(Command::List& pendingCommandList)
		{
			FlushResourceBarriers();
			m_CommandList->Close();

			uint32_t numPendingBarriers = m_ResourceStateTracker->FlushPendingResourceBarriers(pendingCommandList);
			m_ResourceStateTracker->CommitFinalResourceStates();

			return numPendingBarriers != 0;
		}
		void Command::List::Close()
		{
			FlushResourceBarriers();
			m_CommandList->Close();
		}
		void Command::List::Reset()
		{
			m_CommandAllocator->Reset() >> statusCode;
			m_CommandList->Reset(m_CommandAllocator.Get(), nullptr) >> statusCode;

			m_ResourceStateTracker->Reset();
			m_UploadBuffer->Reset();

			ReleaseTrackedObjects();

			for (size_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++)
			{
				m_DynamicDescriptorHeap[i]->Reset();
				m_DescriptorHeaps[i] = nullptr;
			}
			m_RootSignature = nullptr;
			m_ComputeCommandList = nullptr;
		}

		void Command::List::TrackResource(ComPtr<ID3D12Object> object)
		{
			m_TrackedObjects.push_back(object);
		}
		void Command::List::TrackResource(const Resource& resource)
		{
			this->TrackResource(resource.GetD3D12Resource());
		}
		void Command::List::ReleaseTrackedObjects()
		{
			m_TrackedObjects.clear();
		}
		void Command::List::SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, ID3D12DescriptorHeap* heap)
		{
			if (m_DescriptorHeaps[heapType] != heap)
			{
				m_DescriptorHeaps[heapType] = heap;
				BindDescriptorHeaps();
			}
		}
		void Command::List::BindDescriptorHeaps()
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
			m_CommandList->SetDescriptorHeaps(numDescriptorHeaps, descriptorHeaps);
		}

		void Command::List::CopyByteAddressBuffer(ByteAddressBuffer& byteAddressBuffer, size_t bufferSize, const void* bufferData)
		{
		}

		void Command::List::CopyStructuredBuffer(StructuredBuffer& structuredBuffer, size_t numElements, const void* bufferData)
		{
		}

		void Command::List::SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primitiveTopology)
		{
		}

		void Command::List::LoadTextureFromFile(Texture& texture, const std::wstring& filename, TextureUsage textureUsage)
		{
			std::filesystem::path filePath(filename);
			if (!std::filesystem::exists(filePath))
			{
				throw std::exception("File to specified for Texture is not in provided location");
			}
			std::lock_guard<std::mutex> lock(s_TextureCacheMutex);

			auto iter = s_TextureCache.find(filename);
			if (iter != s_TextureCache.end())
			{
				texture.SetTextureUsage(textureUsage);
				texture.SetD3D12Resource(iter->second);
				texture.CreateViews();
				texture.SetName(filename);
			}
			else
			{
				DirectX::TexMetadata metadata;
				DirectX::ScratchImage scratchImage;

				if (filePath.extension() == ".dds")
				{

					DirectX::LoadFromDDSFile(filename.c_str(), DirectX::DDS_FLAGS_NONE, &metadata, scratchImage) >> statusCode;
				}
				else if (filePath.extension() == ".hdr")
				{
					DirectX::LoadFromHDRFile(filename.c_str(), &metadata, scratchImage) >> statusCode;
				}
				else if (filePath.extension() == "tga")
				{
					DirectX::LoadFromTGAFile(filename.c_str(), &metadata, scratchImage) >> statusCode;
				}
				else
				{
					DirectX::LoadFromWICFile(filename.c_str(), DirectX::WIC_FLAGS_NONE, &metadata, scratchImage) >> statusCode;
				}


				D3D12_RESOURCE_DESC textureDesc = {};
				switch (metadata.dimension)
				{
				case DirectX::TEX_DIMENSION_TEXTURE1D:
					textureDesc = CD3DX12_RESOURCE_DESC::Tex1D(metadata.format,
						static_cast<UINT64>(metadata.width),
						static_cast<UINT16>(metadata.arraySize));
					break;
				case DirectX::TEX_DIMENSION_TEXTURE2D:
					textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(metadata.format,
						static_cast<UINT64>(metadata.width),
						static_cast<UINT64>(metadata.height),
						static_cast<UINT16>(metadata.arraySize));
				case DirectX::TEX_DIMENSION_TEXTURE3D:
					textureDesc = CD3DX12_RESOURCE_DESC::Tex3D(metadata.format,
						static_cast<UINT64>(metadata.width),
						static_cast<UINT64>(metadata.height),
						static_cast<UINT16>(metadata.depth));
				default:
					throw std::exception("Invalid texture dimension");
					break;
				}

				ComPtr<ID3D12Resource> textureResource;
				auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
				Device::GetDevice()->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &textureDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&textureResource)) >> statusCode;

				texture.SetTextureUsage(textureUsage);
				texture.SetD3D12Resource(textureResource);
				texture.CreateViews();
				texture.SetName(filename);

				ResourceStateTracker::AddGlobalResourceState(textureResource.Get(), D3D12_RESOURCE_STATE_COMMON);

				std::vector<D3D12_SUBRESOURCE_DATA> subresources(scratchImage.GetImageCount());
				const DirectX::Image* pImages = scratchImage.GetImages();

				for (int i = 0; i < scratchImage.GetImageCount(); i++)
				{
					auto& subresource = subresources[i];
					subresource.RowPitch = pImages[i].rowPitch;
					subresource.SlicePitch = pImages[i].slicePitch;
					subresource.pData = pImages[i].pixels;
				}
				CopyTextureSubResource(texture, 0, static_cast<uint32_t>(subresources.size()), subresources.data());

				if (subresources.size() < textureResource->GetDesc().MipLevels)
				{
					GenerateMips(texture);
				}
				s_TextureCache[filename] = textureResource.Get();
			}
		}


		void Command::List::ResolveSubResource(Resource& dstRes, const Resource& srcRes, uint32_t dstSubResource, uint32_t srcSubResource)
		{
			TransitionBarrier(dstRes, D3D12_RESOURCE_STATE_RESOLVE_DEST, dstSubResource);
			TransitionBarrier(srcRes, D3D12_RESOURCE_STATE_RESOLVE_SOURCE, srcSubResource);

			FlushResourceBarriers();

			m_CommandList->ResolveSubresource(dstRes.GetD3D12Resource().Get(), dstSubResource, srcRes.GetD3D12Resource().Get(), srcSubResource, dstRes.GetD3D12ResourceDesc().Format);
			TrackResource(srcRes);
			TrackResource(dstRes);
		}

		void Command::List::ClearTexture(const Texture& texture, const float clearColor[4])
		{
			TransitionBarrier(texture, D3D12_RESOURCE_STATE_RENDER_TARGET);
			m_CommandList->ClearRenderTargetView(texture.GetRenderTargetView(), clearColor, 0, nullptr);

			TrackResource(texture);
		}

		void Command::List::ClearDepthStencilTexture(const Texture& texture, D3D12_CLEAR_FLAGS clearFlags, float depth, uint8_t stencil)
		{
		}

		void Command::List::CopyVertexBuffer(VertexBuffer& vertexBuffer, size_t numVertices, size_t vertexStride, const void* vertexBufferData)
		{
			CopyBuffer(vertexBuffer, numVertices, vertexStride, vertexBufferData);
		}
		void Command::List::CopyIndexBuffer(IndexBuffer& indexBuffer, size_t numIndices, DXGI_FORMAT indexFormat, const void* indexBufferData)
		{
			size_t indexSizeInBytes = indexFormat == DXGI_FORMAT_R16_UINT ? 2 : 4;
			CopyBuffer(indexBuffer, numIndices, indexSizeInBytes, indexBufferData);
		}

		void Command::List::SetDynamicVertexBuffer(uint32_t slot, size_t numVertices, size_t vertexSize, const void* vertexBufferData)
		{
		}

		void Command::List::SetIndexBuffer(const IndexBuffer& indexBuffer)
		{
		}

		void Command::List::SetDynamicIndexBuffer(size_t numIndices, DXGI_FORMAT indexFormat, const void* IndexBufferData)
		{
			size_t indexSizeInBytes = indexFormat == DXGI_FORMAT_R16_UINT ? 2 : 4;
			size_t bufferSize = numIndices * indexSizeInBytes;

			auto heapAllocation = m_UploadBuffer->Allocate(bufferSize, indexSizeInBytes);
			memcpy(heapAllocation.CPU, IndexBufferData, bufferSize);

			D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
			indexBufferView.BufferLocation = heapAllocation.GPU;
			indexBufferView.SizeInBytes = static_cast<UINT>(bufferSize);
			indexBufferView.Format = indexFormat;

			m_CommandList->IASetIndexBuffer(&indexBufferView);
		}

		void Command::List::SetGraphicsDnyamicStrucuredBuffer(uint32_t slot, size_t numElements, size_t elementSize, const void* bufferData)
		{
		}

		void Command::List::GenerateMips_UAV(Texture& texture, DXGI_FORMAT format)
		{
			if (!m_GenerateMipsPSO)
			{
				m_GenerateMipsPSO = std::make_unique<GenerateMipsPSO>(m_RootSignature);
			}
			m_CommandList->SetPipelineState(m_GenerateMipsPSO->GetPipelineState().Get());
			SetComputeRootSignature(m_GenerateMipsPSO->GetRootSignature());

			GenerateMipsCB generateMipsCB;
			generateMipsCB.IsSRGB = Texture::IsSRGBFormat(format);

			auto resource = texture.GetD3D12Resource();
			auto resourceDesc = resource->GetDesc();

			// Create an SRV that uses the format of the original texture.
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = format;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = resourceDesc.MipLevels;

			for (uint32_t srcMip = 0; srcMip < resourceDesc.MipLevels - 1u;)
			{
				uint64_t srcWidth = resourceDesc.Width >> srcMip;
				uint64_t srcHeight = resourceDesc.Height >> srcMip;
				uint64_t dstWidth = static_cast<uint32_t>(srcWidth >> 1);
				uint64_t dstHeight = srcHeight >> 1;

				generateMipsCB.SrcDimension = (srcHeight & 1) << 1 | (srcWidth & 1);

				DWORD mipCount;

				_BitScanForward(&mipCount, (dstWidth == 1 ? dstHeight : dstWidth) | (dstHeight == 1 ? dstWidth : dstHeight));

				mipCount = std::min<DWORD>(4, mipCount + 1);
				mipCount = (srcMip + mipCount) >= resourceDesc.MipLevels ? resourceDesc.MipLevels - srcMip - 1 : mipCount;


				dstWidth = std::max<DWORD>(1, dstWidth);
				dstHeight = std::max<DWORD>(1, dstHeight);

				generateMipsCB.SrcMipLevel  = srcMip;
				generateMipsCB.NumMipLevels = mipCount;
				generateMipsCB.TexelSize.x = 1.0f / (float)dstWidth;
				generateMipsCB.TexelSize.y = 1.0f / (float)dstHeight;

				SetCompute32BitConstants(GenerateMipsPSO::GenerateMips::_GenerateMipsCB, generateMipsCB);
				SetShaderResourceView(GenerateMipsPSO::GenerateMips::_SrcMip, 0, texture, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, srcMip, 1, &srvDesc);;

				for (size_t mip = 0; mip < mipCount; mip++)
				{
					D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
					uavDesc.Format = resourceDesc.Format;
					uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
					uavDesc.Texture2D.MipSlice = srcMip + mip + 1;

					SetUnorderedAccessView(GenerateMipsPSO::GenerateMips::_OutMip, mip, texture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, srcMip + mip + 1, 1, &uavDesc);
				}
				if (mipCount < 4)
				{
					m_DynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageDescriptors(GenerateMipsPSO::GenerateMips::_OutMip, mipCount, 4 - mipCount, m_GenerateMipsPSO->GetDefaultUAV());
				}
				
				Dispatch(Math::DivideByMultiple(dstWidth, 8), Math::DivideByMultiple(dstHeight, 8));
				UAVBarrier(texture);
				srcMip += mipCount;
			}
		}
		void Command::List::GenerateMips(Texture& texture)
		{
		}
		void Command::List::PanoToCubeMap(Texture& cubeMapTexture, const Texture& panoTexture, const std::shared_ptr<CommandMgr>& commandMgr)
		{
			if (m_CommandListType == D3D12_COMMAND_LIST_TYPE_COPY)
			{
				if (!m_ComputeCommandList)
				{
					m_ComputeCommandList = commandMgr->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE)->GetCommandListClass();
				}
				m_ComputeCommandList->PanoToCubeMap(cubeMapTexture, panoTexture, commandMgr);
				return;
			}
			if (!m_PanoToCubeMapPSO)
			{
				m_PanoToCubeMapPSO = std::make_unique<PanoToCubeMapPSO>();
			}
			auto cubeMapResource = cubeMapTexture.GetD3D12Resource();
			if (!cubeMapResource)
				return;

			CD3DX12_RESOURCE_DESC cubeMapDesc(cubeMapResource->GetDesc());

			auto stagingResource = cubeMapResource;

			Texture stagingTexture(stagingResource);

			if ((cubeMapDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) == 0 )
			{
				auto stagingDesc = cubeMapDesc;
				stagingDesc.Format = Texture::GetUAVCompatableFormat(cubeMapDesc.Format);
				stagingDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

				CD3DX12_HEAP_PROPERTIES properties(D3D12_HEAP_TYPE_DEFAULT);

				Device::GetDevice()->CreateCommittedResource(&properties, D3D12_HEAP_FLAG_NONE, &stagingDesc, D3D12_RESOURCE_STATE_COPY_DEST,
					nullptr, IID_PPV_ARGS(&stagingResource)) >> statusCode;

				ResourceStateTracker::AddGlobalResourceState(stagingResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST);

				stagingTexture.SetD3D12Resource(stagingResource);
				stagingTexture.CreateViews();
				stagingTexture.SetName(L"Pano to Cubemap Staging Texture");

				CopyResource(stagingTexture, cubeMapTexture);
			}

			TransitionBarrier(stagingTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

			m_CommandList->SetPipelineState(m_PanoToCubeMapPSO->GetPipelineState().Get());
			SetComputeRootSignature(m_PanoToCubeMapPSO->GetRootSignature());

			PanoToCubeMapCB panoToCubeMapCB;

			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = { };
			uavDesc.Format = Texture::GetUAVCompatableFormat(cubeMapDesc.Format);
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
			uavDesc.Texture2DArray.FirstArraySlice = 0;
			uavDesc.Texture2DArray.ArraySize = 6;

			for (size_t mipSlice = 0; mipSlice < cubeMapDesc.MipLevels;)
			{
				uint32_t numMips = std::min<uint32_t>(5, cubeMapDesc.MipLevels - mipSlice);

				panoToCubeMapCB.FirstMip = mipSlice;
				panoToCubeMapCB.CubeMapSize = std::max<uint32_t>(static_cast<uint32_t>(cubeMapDesc.Width), cubeMapDesc.Height) >> mipSlice;
				panoToCubeMapCB.NumMips = numMips;

				SetCompute32BitConstants(PanoToCubeMapPSO::PanoToCubeMapRS::_PanoToCubeMapCB, panoToCubeMapCB);
				SetShaderResourceView(PanoToCubeMapPSO::PanoToCubeMapRS::_SrcTexture, 0, panoTexture, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

				for (uint32_t mip = 0; mip < numMips; mip++)
				{
					uavDesc.Texture2DArray.MipSlice = mipSlice + mip;
					SetUnorderedAccessView(PanoToCubeMapPSO::PanoToCubeMapRS::_DstMips, mip, stagingTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
				}

				if (numMips < 5)
				{
					m_DynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageDescriptors(PanoToCubeMapPSO::PanoToCubeMapRS::_DstMips, panoToCubeMapCB.NumMips, 5 - numMips, m_PanoToCubeMapPSO->GetDefaultUAV());
				}
				Dispatch(Math::DivideByMultiple(panoToCubeMapCB.CubeMapSize, 16), Math::DivideByMultiple(panoToCubeMapCB.CubeMapSize, 16), 6);
				mipSlice += numMips;
			}
			if (stagingResource != cubeMapResource)
			{
				CopyResource(cubeMapTexture, stagingTexture);
			}
		}
		void Command::List::CopyTextureSubResource(Texture& texture, uint32_t firstSubResource, uint32_t numSubResources, D3D12_SUBRESOURCE_DATA* subResourceData)
		{
		}
		void Command::List::CopyBuffer(Buffer& buffer, size_t numElements, size_t elementSize, const void* bufferData, D3D12_RESOURCE_FLAGS flags)
		{
			size_t bufferSize = numElements * elementSize;

			ComPtr<ID3D12Resource> d3d12Resource;

			if (bufferSize == 0)
			{
				return;
			}
			else
			{
				auto buffer = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags);
				CD3DX12_HEAP_PROPERTIES propDefault(D3D12_HEAP_TYPE_DEFAULT);
				Device::GetDevice()->CreateCommittedResource(&propDefault, D3D12_HEAP_FLAG_NONE,
					&buffer,
					D3D12_RESOURCE_STATE_COMMON,
					nullptr,
					IID_PPV_ARGS(&d3d12Resource)) >> statusCode;
				ResourceStateTracker::AddGlobalResourceState(d3d12Resource.Get(), D3D12_RESOURCE_STATE_COMMON);

				if (bufferData != nullptr)
				{
					ComPtr<ID3D12Resource> uploadResource;

					CD3DX12_HEAP_PROPERTIES propUpload(D3D12_HEAP_TYPE_UPLOAD);
					Device::GetDevice()->CreateCommittedResource(&propUpload, D3D12_HEAP_FLAG_NONE,
						&buffer,
						D3D12_RESOURCE_STATE_GENERIC_READ,
						nullptr,
						IID_PPV_ARGS(&uploadResource)) >> statusCode;
						
					D3D12_SUBRESOURCE_DATA subResourceData = {};
					subResourceData.pData = bufferData;
					subResourceData.RowPitch = bufferSize;
					subResourceData.SlicePitch = subResourceData.RowPitch;

					m_ResourceStateTracker->TransitionResource(d3d12Resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST);

					FlushResourceBarriers();
					UpdateSubresources(m_CommandList.Get(), d3d12Resource.Get(), uploadResource.Get(), 0, 0, 1, &subResourceData);
					TrackResource(uploadResource);
				}
				TrackResource(d3d12Resource);
			}
			buffer.SetD3D12Resource(d3d12Resource);
			buffer.CreateViews(numElements, elementSize);
		}
	}
}