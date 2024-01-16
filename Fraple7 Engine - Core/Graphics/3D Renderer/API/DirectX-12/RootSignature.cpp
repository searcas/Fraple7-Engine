#include "pch.h"
#include "RootSignature.h"
#include <DirectXMath.h>

namespace Fraple7
{
	namespace Core
	{
		RootSignature::RootSignature(const ComPtr<ID3D12Device2>& device) : m_Device(device)
		{
			Init();
		}

		RootSignature::~RootSignature()
		{
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
			m_Device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature)) >> statusCode;
			FPL_LOG("Root Signature has been created", "Noo Error", Logs::INFO);

		}
		void RootSignature::Validation()
		{
			D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
			if (FAILED(m_Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
			{
				featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
			}
			m_HighestVersion = featureData.HighestVersion;
		}
	}
}

