#pragma once
#include "directx/d3dx12.h"
#include "Utilities/Common/Common.h"
#include "Command.h"

namespace Fraple7
{
	namespace Core
	{
		class Resource;
		class ResourceStateTracker
		{
		public:
			ResourceStateTracker();
			virtual ~ResourceStateTracker();

			void ResourceBarrier(const D3D12_RESOURCE_BARRIER& barrier);
			void TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
			void TransitionResource(const Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
			void UAVBarrier(const Resource* resource = nullptr);
			void AliasBarrier(const Resource* resourceBefore = nullptr, const Resource* resourceAfter = nullptr);
			void FlushResourceBarriers(Command::List&);
			uint32_t FlushPendingResourceBarriers(Command::List& list);
			void CommitFinalResourceStates();

			void Reset();

			static void Lock();
			static void Unlock();

			static void AddGlobalResourceState(ID3D12Resource* resource, D3D12_RESOURCE_STATES state);
			static void RemoveGlobalResourceState(ID3D12Resource* resource);

		private:
			using ResourceBarriers = std::vector<D3D12_RESOURCE_BARRIER>;

			ResourceBarriers m_PendingResourceBarriers;
			ResourceBarriers m_ResourceBarriers;

			struct ResourceState
			{
				explicit ResourceState(D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON)
					: State(state)
				{

				}

				void SetSubResourceState(UINT subresource, D3D12_RESOURCE_STATES state)
				{
					if (subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
					{
						State = state;
						SubresourceState.clear();
					}
					else
					{
						SubresourceState[subresource] = state;
					}
				}
				D3D12_RESOURCE_STATES GetSubResourceState(UINT subresource) const
				{
					D3D12_RESOURCE_STATES state = State;
					const auto iter = SubresourceState.find(subresource);

					if (iter != SubresourceState.end())
					{
						state = iter->second;
					}
					return state;
				}
				D3D12_RESOURCE_STATES State;
				std::map<UINT, D3D12_RESOURCE_STATES> SubresourceState;
			};
			using ResourceStateMap = std::unordered_map<ID3D12Resource*, ResourceState>;
			ResourceStateMap m_FinalResourceState;
			static ResourceStateMap s_GlobalResourceState;
			static std::mutex s_GlobalMutex;
			static std::atomic<bool>s_IsLocked;
		};

	}
}