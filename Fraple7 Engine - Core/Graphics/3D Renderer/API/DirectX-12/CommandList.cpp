#include "pch.h"
#include "CommandList.h"
#include "Device.h"
#include "ResourceStateTracker.h"
#include "RootSignature.h"
#include "Memory/UploadBuffer.h"
#include "Memory/DynamicDescriptorHeap.h"
#include "Memory/StructuredBuffer.h"
#include "Memory/VertexBuffer.h"
#include "Memory/IndexBuffer.h"
#include "GenerateMipsPSO.h"
#include "PanoToCubeMapPSO.h"
#include "RenderTarget.h"

#include "Utilities/Math/Math.h"
#include "CommandMgr.h"
#include "CommandQueue.h"
namespace Fraple7
{
	namespace Core
	{

		CommandList::CommandList(D3D12_COMMAND_LIST_TYPE type)
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

		CommandList::~CommandList()
		{
		}

		void CommandList::TransitionBarrier(ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource, bool flushBarriers)
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
		void CommandList::TransitionBarrier(const Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource, bool flushBarriers)
		{
			TransitionBarrier(resource.GetD3D12Resource(), stateAfter, subResource, flushBarriers);
		}

		void CommandList::CopyResource(Resource& dstRes, const Resource& srcRes)
		{
			CopyResource(dstRes.GetD3D12Resource(), srcRes.GetD3D12Resource());
		}
		void CommandList::CopyResource(ComPtr<ID3D12Resource> dstResource, ComPtr<ID3D12Resource> srcResource)
		{
			TransitionBarrier(dstResource, D3D12_RESOURCE_STATE_COPY_DEST);
			TransitionBarrier(srcResource, D3D12_RESOURCE_STATE_COPY_SOURCE);

			FlushResourceBarriers();
			m_CommandList->CopyResource(dstResource.Get(), srcResource.Get());
			TrackResource(dstResource);
			TrackResource(srcResource);
		}
		void CommandList::UAVBarrier(ComPtr<ID3D12Resource> resource, bool flushBarriers)
		{
			auto barrier = CD3DX12_RESOURCE_BARRIER::UAV(resource.Get());
			m_ResourceStateTracker->ResourceBarrier(barrier);

			if (flushBarriers)
			{
				FlushResourceBarriers();
			}
		}
		void CommandList::UAVBarrier(const Resource& resource, bool flushBarriers)
		{
			UAVBarrier(resource.GetD3D12Resource(), flushBarriers);
		}
		void CommandList::AliasingBarrier(const Resource& beforeResource, const Resource& afterResource, bool flushBarriers)
		{
			AliasingBarrier(beforeResource.GetD3D12Resource(), afterResource.GetD3D12Resource(), flushBarriers);
		}
		void CommandList::AliasingBarrier(ComPtr<ID3D12Resource> beforeRes, ComPtr<ID3D12Resource> afterRes, bool flushBarriers)
		{
			auto barrier = CD3DX12_RESOURCE_BARRIER::Aliasing(beforeRes.Get(), afterRes.Get());
			m_ResourceStateTracker->ResourceBarrier(barrier);

			if (flushBarriers)
			{
				FlushResourceBarriers();
			}

		}
		void CommandList::FlushResourceBarriers()
		{
			m_ResourceStateTracker->FlushResourceBarriers(*this);
		}
		void CommandList::SetGraphicsDynamicConstantBuffer(uint32_t rootParameterIndex, size_t sizeInBytes, const void* data)
		{
			auto heapAllocation = m_UploadBuffer->Allocate(sizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
			memcpy(heapAllocation.CPU, data, sizeInBytes);
			m_CommandList->SetGraphicsRootConstantBufferView(rootParameterIndex, heapAllocation.GPU);
		}

		void CommandList::SetGraphics32BitConstants(uint32_t rootParameterIndex, uint32_t numConstants, const void* constants)
		{
			m_CommandList->SetGraphicsRoot32BitConstants(rootParameterIndex, numConstants, constants, 0);
		}

		void CommandList::SetCompute32BitConstants(uint32_t rootParameterIndex, uint32_t numConstants, const void* constants)
		{
			m_CommandList->SetComputeRoot32BitConstants(rootParameterIndex, numConstants, constants, 0);
		}

		void CommandList::SetPipelineState(ComPtr<ID3D12PipelineState> pipelineState)
		{
			m_CommandList->SetPipelineState(pipelineState.Get());
			TrackResource(pipelineState);
		}
		void CommandList::SetGraphicsRootSignature(const std::shared_ptr<RootSignature>& rootSignature)
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
		void CommandList::SetComputeRootSignature(const std::shared_ptr<RootSignature>& rootSignature)
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
		void CommandList::SetViewPort(const D3D12_VIEWPORT& viewport)
		{
			SetViewPorts({ viewport });
		}
		void CommandList::SetViewPorts(const std::vector<D3D12_VIEWPORT>& viewports)
		{
			assert(viewports.size() < D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE);
			m_CommandList->RSSetViewports(static_cast<UINT>(viewports.size()), viewports.data());
		}
		void CommandList::SetScissorRect(const D3D12_RECT& scissorRect)
		{
			SetScissorRect({ scissorRect });
		}
		void CommandList::SetScissorRects(const std::vector<D3D12_RECT>& scissorRects)
		{
			assert(scissorRects.size() < D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE);
			m_CommandList->RSSetScissorRects(static_cast<UINT>(scissorRects.size()), scissorRects.data());
		}
		void CommandList::SetShaderResourceView(uint32_t rootParameterIndex, uint32_t descriptorOffset, const Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT firstSubResource, UINT numSubResources, const D3D12_SHADER_RESOURCE_VIEW_DESC* srv)
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
		void CommandList::SetUnorderedAccessView(uint32_t rootParameterIndex,
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
		void CommandList::SetRenderTarget(const RenderTarget& renderTarget)
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
		void CommandList::Render(uint32_t vertexCount, uint32_t instanceCount, uint32_t startVertex, uint32_t startInstance)
		{
			FlushResourceBarriers();

			for (size_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++)
			{
				m_DynamicDescriptorHeap[i]->CommitStagedDescriptorsRender(*this);
			}
			m_CommandList->DrawInstanced(vertexCount, instanceCount, startVertex, startInstance);
		}
		void CommandList::RenderIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t startIndex, int32_t baseVertex, uint32_t startInstance)
		{
			FlushResourceBarriers();

			for (size_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++)
			{
				m_DynamicDescriptorHeap[i]->CommitStagedDescriptorsRender(*this);
			}
			m_CommandList->DrawIndexedInstanced(indexCount, instanceCount, startIndex, baseVertex, startInstance);
		}
		void CommandList::Dispatch(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ)
		{
			FlushResourceBarriers();

			for (size_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++)
			{
				m_DynamicDescriptorHeap[i]->CommitStagedDescriptorsRender(*this);
			}
			m_CommandList->Dispatch(numGroupsX, numGroupsY, numGroupsZ);
		}
		bool CommandList::Close(CommandList& pendingCommandList)
		{
			FlushResourceBarriers();
			m_CommandList->Close();

			uint32_t numPendingBarriers = m_ResourceStateTracker->FlushPendingResourceBarriers(pendingCommandList);
			m_ResourceStateTracker->CommitFinalResourceStates();

			return numPendingBarriers != 0;
		}
		void CommandList::Close()
		{
			FlushResourceBarriers();
			m_CommandList->Close();
		}
		void CommandList::Reset()
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

		void CommandList::TrackResource(ComPtr<ID3D12Object> object)
		{
			m_TrackedObjects.push_back(object);
		}
		void CommandList::TrackResource(const Resource& resource)
		{
			this->TrackResource(resource.GetD3D12Resource());
		}
		void CommandList::ReleaseTrackedObjects()
		{
			m_TrackedObjects.clear();
		}
		void CommandList::SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, ID3D12DescriptorHeap* heap)
		{
			if (m_DescriptorHeaps[heapType] != heap)
			{
				m_DescriptorHeaps[heapType] = heap;
				BindDescriptorHeaps();
			}
		}
		void CommandList::BindDescriptorHeaps()
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

		void CommandList::CopyByteAddressBuffer(ByteAddressBuffer& byteAddressBuffer, size_t bufferSize, const void* bufferData)
		{
			CopyBuffer(byteAddressBuffer, 1, bufferSize, bufferData, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		}

		void CommandList::CopyStructuredBuffer(StructuredBuffer& structuredBuffer, size_t numElements, size_t elementSize, const void* bufferData)
		{
			CopyBuffer(structuredBuffer, numElements, elementSize, bufferData, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		}

		void CommandList::SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primitiveTopology)
		{
			m_CommandList->IASetPrimitiveTopology(primitiveTopology);
		}

		void CommandList::LoadTextureFromFile(Texture& texture, const std::wstring& filename, std::shared_ptr<class CommandMgr>& commandMgr, TextureUsage textureUsage)
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
					GenerateMips(texture, commandMgr);
				}
				s_TextureCache[filename] = textureResource.Get();
			}
		}


		void CommandList::ResolveSubResource(Resource& dstRes, const Resource& srcRes, uint32_t dstSubResource, uint32_t srcSubResource)
		{
			TransitionBarrier(dstRes, D3D12_RESOURCE_STATE_RESOLVE_DEST, dstSubResource);
			TransitionBarrier(srcRes, D3D12_RESOURCE_STATE_RESOLVE_SOURCE, srcSubResource);

			FlushResourceBarriers();

			m_CommandList->ResolveSubresource(dstRes.GetD3D12Resource().Get(), dstSubResource, srcRes.GetD3D12Resource().Get(), srcSubResource, dstRes.GetD3D12ResourceDesc().Format);
			TrackResource(srcRes);
			TrackResource(dstRes);
		}

		void CommandList::ClearTexture(const Texture& texture, const float clearColor[4])
		{
			TransitionBarrier(texture, D3D12_RESOURCE_STATE_RENDER_TARGET);
			m_CommandList->ClearRenderTargetView(texture.GetRenderTargetView(), clearColor, 0, nullptr);

			TrackResource(texture);
		}

		void CommandList::ClearDepthStencilTexture(const Texture& texture, D3D12_CLEAR_FLAGS clearFlags, float depth, uint8_t stencil)
		{
			TransitionBarrier(texture, D3D12_RESOURCE_STATE_DEPTH_WRITE);
			m_CommandList->ClearDepthStencilView(texture.GetDepthStencilView(), clearFlags, depth, stencil, 0, nullptr);
		}

		void CommandList::CopyVertexBuffer(VertexBuffer& vertexBuffer, size_t numVertices, size_t vertexStride, const void* vertexBufferData)
		{
			CopyBuffer(vertexBuffer, numVertices, vertexStride, vertexBufferData);
		}
		void CommandList::CopyIndexBuffer(IndexBuffer& indexBuffer, size_t numIndices, DXGI_FORMAT indexFormat, const void* indexBufferData)
		{
			size_t indexSizeInBytes = indexFormat == DXGI_FORMAT_R16_UINT ? 2 : 4;
			CopyBuffer(indexBuffer, numIndices, indexSizeInBytes, indexBufferData);
		}

		void CommandList::SetDynamicVertexBuffer(uint32_t slot, size_t numVertices, size_t vertexSize, const void* vertexBufferData)
		{
			size_t bufferSize = numVertices * vertexSize;
			auto heapAllocation = m_UploadBuffer->Allocate(bufferSize, vertexSize);

			memcpy(heapAllocation.CPU, vertexBufferData, bufferSize);

			D3D12_VERTEX_BUFFER_VIEW vertexBufferView = { };
			vertexBufferView.BufferLocation = heapAllocation.GPU;
			vertexBufferView.SizeInBytes = static_cast<UINT>(bufferSize);
			vertexBufferView.StrideInBytes = static_cast<UINT>(vertexSize);

			m_CommandList->IASetVertexBuffers(slot, 1, &vertexBufferView);
		}

		void CommandList::SetIndexBuffer(const IndexBuffer& indexBuffer)
		{
			TransitionBarrier(indexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER);

			auto& indexBufferView = indexBuffer.GetIndexBufferView();

			m_CommandList->IASetIndexBuffer(&indexBufferView);
			TrackResource(indexBuffer);
		}

		void CommandList::SetDynamicIndexBuffer(size_t numIndices, DXGI_FORMAT indexFormat, const void* IndexBufferData)
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

		void CommandList::SetGraphicsDynamicStructuredBuffer(uint32_t slot, size_t numElements, size_t elementSize, const void* bufferData)
		{
			size_t bufferSize = numElements * elementSize;

			auto heapAllocation = m_UploadBuffer->Allocate(bufferSize, elementSize);

			memcpy(heapAllocation.CPU, bufferData, bufferSize);

			m_CommandList->SetGraphicsRootShaderResourceView(slot, heapAllocation.GPU);
		}

		void CommandList::GenerateMips_UAV(Texture& texture, DXGI_FORMAT format)
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

				generateMipsCB.SrcMipLevel = srcMip;
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
		void CommandList::GenerateMips(Texture& texture, const std::shared_ptr<CommandMgr>& commandMgr)
		{
			if (m_CommandListType == D3D12_COMMAND_LIST_TYPE_COPY)
			{
				if (!m_ComputeCommandList) {
					m_ComputeCommandList = commandMgr->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE)->GetCommandListClass();
				}
				m_ComputeCommandList->GenerateMips(texture, commandMgr);
				return;
			}
			auto resource = texture.GetD3D12Resource();

			if (!resource)
			{
				return;
			}
			auto resourceDesc = resource->GetDesc();

			if (resourceDesc.MipLevels == 1)
			{
				return;
			}
			if (resourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D ||
				resourceDesc.DepthOrArraySize != 1 ||
				resourceDesc.SampleDesc.Count > 1)
			{
				throw std::exception("GenerateMips is only supported for non-multi sampled 2D Textures.");
			}

			ComPtr<ID3D12Resource> uavResource = resource;
			ComPtr<ID3D12Resource> aliasResource;

			if (!texture.CheckUAVSupport() || (resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) == 0)
			{
				auto aliasDesc = resourceDesc;

				aliasDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
				aliasDesc.Flags &= ~(D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

				auto uavDesc = aliasDesc;
				uavDesc.Format = Texture::GetUAVCompatableFormat(resourceDesc.Format);

				D3D12_RESOURCE_DESC resourceDescs[] =
				{
					aliasDesc,
					uavDesc
				};

				auto allocationInfo = Device::GetDevice()->GetResourceAllocationInfo(0, _countof(resourceDescs), resourceDescs);

				D3D12_HEAP_DESC heapDesc = { };
				heapDesc.SizeInBytes = allocationInfo.SizeInBytes;
				heapDesc.Alignment = allocationInfo.Alignment;
				heapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
				heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
				heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
				heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;

				ComPtr<ID3D12Heap>heap;
				Device::GetDevice()->CreateHeap(&heapDesc, IID_PPV_ARGS(&heap)) >> statusCode;

				TrackResource(heap);

				Device::GetDevice()->CreatePlacedResource(heap.Get(), 0, &aliasDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&aliasResource)) >> statusCode;

				ResourceStateTracker::AddGlobalResourceState(aliasResource.Get(), D3D12_RESOURCE_STATE_COMMON);
				TrackResource(aliasResource);

				Device::GetDevice()->CreatePlacedResource(heap.Get(), 0, &uavDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&uavResource)) >> statusCode;

				ResourceStateTracker::AddGlobalResourceState(uavResource.Get(), D3D12_RESOURCE_STATE_COMMON);

				TrackResource(uavResource);
				AliasingBarrier(nullptr, aliasResource);
				CopyResource(aliasResource, resource);
				AliasingBarrier(aliasResource, uavResource);
			}
			Texture texUAV = Texture(uavResource, texture.GetTextureUsage());
			GenerateMips_UAV(texUAV, resourceDesc.Format);

			if (aliasResource)
			{
				AliasingBarrier(uavResource, aliasResource);
				CopyResource(resource, aliasResource);
			}
		}
		void CommandList::PanoToCubeMap(Texture& cubeMapTexture, const Texture& panoTexture, const std::shared_ptr<CommandMgr>& commandMgr)
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

			if ((cubeMapDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) == 0)
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
		void CommandList::CopyTextureSubResource(Texture& texture, uint32_t firstSubResource, uint32_t numSubResources, D3D12_SUBRESOURCE_DATA* subResourceData)
		{
			auto destinationResource = texture.GetD3D12Resource();

			if (destinationResource)
			{
				TransitionBarrier(texture, D3D12_RESOURCE_STATE_COPY_DEST);
				FlushResourceBarriers();

				UINT64 requiredSize = GetRequiredIntermediateSize(destinationResource.Get(), firstSubResource, numSubResources);

				ComPtr<ID3D12Resource> intermediateResource;
				auto properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
				auto buffer = CD3DX12_RESOURCE_DESC::Buffer(requiredSize);
				Device::GetDevice()->CreateCommittedResource(&properties,
					D3D12_HEAP_FLAG_NONE,
					&buffer,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(&intermediateResource)) >> statusCode;
				UpdateSubresources(m_CommandList.Get(), destinationResource.Get(), intermediateResource.Get(), 0, firstSubResource, numSubResources, subResourceData);
				TrackResource(intermediateResource);
				TrackResource(destinationResource);
			}
		}
		void CommandList::CopyBuffer(Buffer& buffer, size_t numElements, size_t elementSize, const void* bufferData, D3D12_RESOURCE_FLAGS flags)
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