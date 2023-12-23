#include "pch.h"
#include "RootSignature.h"
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
		void RootSignature::Init()
		{
			const D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
				| D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT
				| D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED
				| D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;

			m_RootParameters[0].InitAsConstants(sizeof(DirectX::XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
			m_DescRange = CD3DX12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0 };
			m_RootParameters[1].InitAsDescriptorTable(1, &m_DescRange);
			m_StaticSampler = CD3DX12_STATIC_SAMPLER_DESC{ 0, D3D12_FILTER_MIN_MAG_MIP_LINEAR };
			m_SigDesc.Init((UINT)std::size(m_RootParameters), m_RootParameters, 1, &m_StaticSampler, flags);
		}
		void RootSignature::Create(const ComPtr<ID3D12Device2>& device)
		{
			ComPtr<ID3D10Blob> signatureBlob;
			ComPtr<ID3D10Blob> errorBlob;

			if (const auto hr = D3D12SerializeRootSignature(&m_SigDesc, D3D_ROOT_SIGNATURE_VERSION_1,&signatureBlob, &errorBlob); FAILED(hr))
			{
				if (errorBlob)
				{
					auto errorBufferPtr = static_cast<const char*>(errorBlob->GetBufferPointer());
					FPL_LOG(errorBufferPtr, "Error in Root Signature", Logs::API_ERROR);
				}
				hr >> statusCode;
			}
			device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature)) >> statusCode;
			FPL_LOG("Root Signature has been created", "Noo Error", Logs::INFO);

		}
	}
}

