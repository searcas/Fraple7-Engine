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
			RootSignature();
			~RootSignature();
			
			const ComPtr<ID3D12RootSignature>& GetRootSignature()const { return m_RootSignature; }
			D3D12_ROOT_SIGNATURE_DESC1 GetRootSignatureDesc() const { return m_RootSignatureDesc; }
			void SetRootSignatureDesc(D3D12_ROOT_SIGNATURE_DESC1 signatureDesc, D3D_ROOT_SIGNATURE_VERSION signatureVersion) 
			{
				m_RootSignatureDesc = signatureDesc;
				m_HighestVersion = signatureVersion;
			}
			void SetRootSignature(ComPtr<ID3D12RootSignature> rootSignature)
			{
				m_RootSignature = rootSignature;
			}
			uint32_t GetDescriptorTableBitMask(D3D12_DESCRIPTOR_HEAP_TYPE type);
			uint32_t GetNumDescriptors(uint32_t rootIndex)const;
			void Init();
			void Create();
			void Validation();
		private:
			ComPtr<ID3D12RootSignature> m_RootSignature;
			CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC m_SigDesc;
			D3D12_ROOT_SIGNATURE_DESC1 m_RootSignatureDesc;
			CD3DX12_ROOT_PARAMETER1 m_RootParameters[ROOT_PARAMETERS]{};
			CD3DX12_STATIC_SAMPLER_DESC m_StaticSampler;
			CD3DX12_DESCRIPTOR_RANGE1 m_DescRange;
			D3D_ROOT_SIGNATURE_VERSION m_HighestVersion;
			uint32_t m_DescriptorTableBitMask;
			uint32_t m_SamplerTableBitMask;
			inline constexpr static uint32_t s_NumDescPerTable = 32;
			uint32_t m_NumDescriptorsPerTable[s_NumDescPerTable];

		};

	}
}
