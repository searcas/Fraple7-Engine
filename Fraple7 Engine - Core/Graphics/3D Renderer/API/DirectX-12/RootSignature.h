#pragma once
#include "Utilities/Common/Common.h"
#include "directx/d3dx12.h"
namespace Fraple7
{
	namespace Core
	{
#define ROOT_PARAMETERS 2
		class RootSignature
		{
		public:
			RootSignature(const ComPtr<ID3D12Device2>& device);
			~RootSignature();
			
			const ComPtr<ID3D12RootSignature>& GetSignature() { return m_RootSignature; }
			void Init();
			void Create();
			void Validation();
		private:
			ComPtr<ID3D12RootSignature> m_RootSignature;
			CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC m_SigDesc;
			CD3DX12_ROOT_PARAMETER1 m_RootParameters[ROOT_PARAMETERS]{};
			CD3DX12_STATIC_SAMPLER_DESC m_StaticSampler;
			CD3DX12_DESCRIPTOR_RANGE1 m_DescRange;
			const ComPtr<ID3D12Device2>& m_Device;
			D3D_ROOT_SIGNATURE_VERSION m_HighestVersion;

		};

	}
}
