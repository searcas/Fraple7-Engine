#include "pch.h"
#include "RootSignature.h"
#include "Device.h"
#include <DirectXMath.h>
namespace Fraple7
{
	namespace Core
	{
		RootSignature::RootSignature() 
		{
			Init();
		}

		RootSignature::~RootSignature()
		{
		}
		uint32_t RootSignature::GetDescriptorTableBitMask(D3D12_DESCRIPTOR_HEAP_TYPE type)
		{
			uint32_t descriptorTableMask = 0;

			switch (type)
			{
			case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
				descriptorTableMask = m_DescriptorTableBitMask;
				break;
			case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
				descriptorTableMask = m_SamplerTableBitMask;
				break;
			case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
				break;
			case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
				break;
			case D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES:
				break;
			default:
				break;
			}
			return descriptorTableMask;
		}
		uint32_t RootSignature::GetNumDescriptors(uint32_t rootIndex) const
		{
			assert(rootIndex < s_NumDescPerTable);
			return m_NumDescriptorsPerTable[rootIndex];
		}
		void RootSignature::Init()
		{
			const D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
				| D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT
				| D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED
				| D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;

			m_RootParameters[0].InitAsConstants(sizeof(DirectX::XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
			m_DescRange = CD3DX12_DESCRIPTOR_RANGE1{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0u };
			m_RootParameters[1].InitAsDescriptorTable(1, &m_DescRange);
			m_StaticSampler = CD3DX12_STATIC_SAMPLER_DESC{ 0, D3D12_FILTER_MIN_MAG_MIP_LINEAR };
			m_SigDesc.Init_1_1((UINT)std::size(m_RootParameters), m_RootParameters, 1, &m_StaticSampler, flags);
		}
		void RootSignature::Create()
		{
			ComPtr<ID3D10Blob> signatureBlob;
			ComPtr<ID3D10Blob> errorBlob;
			Validation();
			
			if (const auto hr = D3DX12SerializeVersionedRootSignature(&m_SigDesc, m_HighestVersion, &signatureBlob, &errorBlob); FAILED(hr))
			{
				if (errorBlob)
				{
					auto errorBufferPtr = static_cast<const char*>(errorBlob->GetBufferPointer());
					FPL_LOG(errorBufferPtr, "Error in Root Signature", Logs::API_ERROR);
				}
				hr >> statusCode;
			}
			Device::GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature)) >> statusCode;
			FPL_LOG("Root Signature has been created", "Noo Error", Logs::INFO);

		}
		void RootSignature::Validation()
		{
			D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
			if (FAILED(Device::GetDevice()->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
			{
				featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
			}
			m_HighestVersion = featureData.HighestVersion;
		}
	}
}

