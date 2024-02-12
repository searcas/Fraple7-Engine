#pragma once
#include "Graphics/Texture/Texture.h"

namespace Fraple7
{
	namespace Core
	{
		class Resource;
		class VertexBuffer;
		class IndexBuffer;
		class ByteAddressBuffer;
		class StructuredBuffer;
		class RootSignature;
		class RenderTarget;
		class Buffer;
		class UploadBuffer;
		class ResourceStateTracker;
		class DynamicDescriptorHeap;
		class GenerateMipsPSO;
		class PanoToCubeMapPSO;

		class CommandList
		{
		public:
			CommandList(D3D12_COMMAND_LIST_TYPE type);

			D3D12_COMMAND_LIST_TYPE GetCommandListType()const
			{
				return m_CommandListType;
			}
			ComPtr<ID3D12GraphicsCommandList2>GetCommandList() const
			{
				return m_CommandList;
			}
			virtual ~CommandList();

			void TransitionBarrier(const Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, bool flushBarriers = false);
			void TransitionBarrier(ComPtr<ID3D12Resource>resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, bool flushBarriers = false);

			void UAVBarrier(const Resource& resource, bool flushBarriers = false);
			void UAVBarrier(ComPtr<ID3D12Resource> resource, bool flushBarriers = false);

			void AliasingBarrier(const Resource& beforeResource, const Resource& afterResource, bool flushBarriers = false);
			void AliasingBarrier(ComPtr<ID3D12Resource> beforeResource, ComPtr<ID3D12Resource> afterResource, bool flushBarriers = false);
			void FlushResourceBarriers();
			void CopyResource(Resource& dstRest, const Resource& srcRes);
			void CopyResource(ComPtr<ID3D12Resource> dstRes, ComPtr<ID3D12Resource> src);
			void ResolveSubResource(Resource& dstRes, const Resource& srcRes, uint32_t dstSubresource = 0, uint32_t srcSubResource = 0);
			void CopyVertexBuffer(VertexBuffer& vertexBuffer, size_t numVertices, size_t vertexStride, const void* vertexBufferData);
			template<typename T>
			void CopyVertexBuffer(VertexBuffer& vertexBuffer, const std::vector<T>& vertexBufferData)
			{
				static_assert(sizeof(T) == 2 || sizeof(T) == 4);
				DXGI_FORMAT indexFormat = (sizeof(T) == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
				CopyVertexBuffer(vertexBuffer, vertexBufferData.size(), sizeof(T), vertexBufferData);
			}

			void CopyIndexBuffer(IndexBuffer& indexBuffer, size_t numIndices, DXGI_FORMAT indexFormat, const void* indexBufferData);

			template<typename T>
			void CopyIndexBuffer(IndexBuffer& indexBuffer, const std::vector<T>& indexBufferData)
			{
				static_assert(sizeof(T) == 2 || sizeof(T) == 4);
				DXGI_FORMAT indexFormat = (sizeof(T) == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
				CopyIndexBuffer(indexBuffer, indexBufferData.size(), indexFormat, indexBufferData);
			}

			void CopyByteAddressBuffer(ByteAddressBuffer& byteAddressBuffer, size_t bufferSize, const void* bufferData);
			template<typename T>
			void CopyByteAddressBuffer(ByteAddressBuffer& byteAddressBuffer, const T& data)
			{
				CopyByteAddressBuffer(byteAddressBuffer, sizeof(T), &data);
			}

			void CopyStructuredBuffer(StructuredBuffer& structuredBuffer, size_t numElements, size_t elementSize, const void* bufferData);
			template<typename T>
			void CopyStructuredBuffer(StructuredBuffer& structuredBuffer, const std::vector<T>& BufferData)
			{
				CopyStructuredBuffer(structuredBuffer, BufferData.size(), sizeof(T), BufferData.data());
			}
			void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primitiveTopology);
			void LoadTextureFromFile(Texture& texture, const std::wstring& filename, std::shared_ptr<class CommandMgr>& commandMgr, TextureUsage textureUSage = TextureUsage::Albedo);
			void ClearTexture(const Texture& texture, const float clearColor[4]);
			void ClearDepthStencilTexture(const Texture& texture, D3D12_CLEAR_FLAGS clearFlags, float depth = 1.0f, uint8_t stencil = 0);
			void GenerateMips(Texture& texture, const std::shared_ptr<class CommandMgr>& commandMgr);
			void PanoToCubeMap(Texture& cubemap, const Texture& pano, const std::shared_ptr <class CommandMgr>& commandMgr);

			void CopyTextureSubResource(Texture& texture, uint32_t firstSubResource, uint32_t numSubResources, D3D12_SUBRESOURCE_DATA* subResourceData);

			void SetGraphicsDynamicConstantBuffer(uint32_t rootParameterIndex, size_t sizeInBytes, const void* data);

			template<typename T>
			void SetGraphicsDynamicConstantBuffer(uint32_t rootParameterIndex, const T& data)
			{
				SetGraphicsDynamicConstantBuffer(rootParameterIndex, sizeof(T), &data);
			}
			void SetGraphics32BitConstants(uint32_t rootParameterIndex, uint32_t numConstants, const void* constants);
			template<typename T>
			void SetGraphics32BitConstants(uint32_t rootParameterIndex, const T& constants)
			{
				static_assert(sizeof(T) % sizeof(uint32_t) == 0, "Size of type must be multiple of 4 bytes");
				SetGraphics32BitConstants(rootParameterIndex, sizeof(T) / sizeof(uint32_t), &constants);
			}
			void SetCompute32BitConstants(uint32_t rootParameterIndex, uint32_t numConstants, const void* constants);
			template<typename T>
			void SetCompute32BitConstants(uint32_t rootParameterIndex, const T& constants)
			{
				static_assert(sizeof(T) % sizeof(uint32_t) == 0, "Size of tpye must a multiple of 4 bytes");
				SetCompute32BitConstants(rootParameterIndex, sizeof(T) / sizeof(uint32_t), &constants);
			}

			void SetDynamicVertexBuffer(uint32_t slot, size_t numVertices, size_t vertexSize, const void* vertexBufferData);
			template<typename T>
			void SetDynamicVertexBuffer(uint32_t slot, const std::vector<T>& vertexBufferData)
			{
				SetDynamicVertexBuffer(slot, vertexBufferData.size(), sizeof(T), vertexBufferData.data());
			}
			void SetIndexBuffer(const IndexBuffer& indexBuffer);
			void SetDynamicIndexBuffer(size_t numIndices, DXGI_FORMAT indexFormat, const void* IndexBufferData);
			template<typename T>
			void SetDynamicIndexBuffer(std::vector<T>& indexBufferData)
			{
				static_assert(sizeof(T) == 2 || sizeof(T) == 4);
				DXGI_FORMAT indexFormat = (sizeof(T) == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
				SetDynamicIndexBuffer(indexBufferData.size(), indexFormat, indexBufferData.data());
			}
			void SetGraphicsDynamicStructuredBuffer(uint32_t slot, size_t numElements, size_t elementSize, const void* bufferData);
			template <typename T>
			void SetGraphicsDynamicStructuredBuffer(uint32_t slot, const std::vector<T>& bufferData)
			{
				SetGraphicsDnyamicStrucuredBuffer(slot, bufferData.size(), sizeof(T), bufferData.data());
			}
			void SetViewPort(const D3D12_VIEWPORT& viewport);
			void SetViewPorts(const std::vector<D3D12_VIEWPORT>& viewports);
			void SetScissorRect(const D3D12_RECT& scissorRect);
			void SetScissorRects(const std::vector<D3D12_RECT>& scissorRects);
			void SetPipelineState(ComPtr<ID3D12PipelineState> pipelineState);
			void SetGraphicsRootSignature(const std::shared_ptr<RootSignature>& rootSignature);
			void SetComputeRootSignature(const std::shared_ptr<RootSignature>& rootSignature);
			void SetShaderResourceView(uint32_t rootParameterIndex, uint32_t descriptorOffset,
				const Resource& resource,
				D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
				D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
				UINT firstSubResource = 0, UINT numSubResources = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
				const D3D12_SHADER_RESOURCE_VIEW_DESC* srv = nullptr);

			void SetUnorderedAccessView(uint32_t rootParameterIndex,
				uint32_t descriptorOffset,
				const Resource& resource,
				D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
				UINT firstSubResource = 0,
				UINT numSubResources = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
				const D3D12_UNORDERED_ACCESS_VIEW_DESC* uav = nullptr);

			void SetRenderTarget(const RenderTarget& renderTarget);
			void Render(uint32_t vertexCount, uint32_t instanceCount, uint32_t startVertex, uint32_t startInstance);
			void RenderIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t startIndex = 0, int32_t baseVertex = 0, uint32_t startInstance = 0);
			void Dispatch(uint32_t numGroupsX, uint32_t numGroupsY = 1, uint32_t numGroupsZ = 1);
			bool Close(CommandList& pendingCommandList);
			void Close();
			void Reset();
			void ReleaseTrackedObjects();
			void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, ID3D12DescriptorHeap* heap);
			std::shared_ptr<CommandList>GetGeneratedMipsCommandList() const
			{
				return m_ComputeCommandList;
			}
		private:

			void TrackResource(ComPtr<ID3D12Object> object);
			void TrackResource(const Resource& resource);

			void GenerateMips_UAV(Texture& texture, DXGI_FORMAT format);
			void CopyBuffer(Buffer& buffer, size_t numElements, size_t elementSize, const void* bufferData, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
			void BindDescriptorHeaps();

			using TrackedObjects = std::vector<ComPtr<ID3D12Object>>;
			ComPtr<ID3D12GraphicsCommandList2> m_CommandList;
			ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
			D3D12_COMMAND_LIST_TYPE m_CommandListType;

			std::shared_ptr<CommandList>m_ComputeCommandList;
			std::shared_ptr<RootSignature> m_RootSignature;
			std::unique_ptr<UploadBuffer> m_UploadBuffer;
			std::unique_ptr<ResourceStateTracker> m_ResourceStateTracker;
			std::unique_ptr<DynamicDescriptorHeap> m_DynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
			ID3D12DescriptorHeap* m_DescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
			std::unique_ptr<GenerateMipsPSO>m_GenerateMipsPSO;
			std::unique_ptr<PanoToCubeMapPSO>m_PanoToCubeMapPSO;
			TrackedObjects m_TrackedObjects;
			static inline std::map<std::wstring, ID3D12Resource*> s_TextureCache;
			static inline std::mutex s_TextureCacheMutex;
		};
	}
}
