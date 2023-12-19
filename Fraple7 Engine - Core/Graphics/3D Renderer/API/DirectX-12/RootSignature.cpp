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
			const D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED |
				D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED;

			m_RootParameters[0].InitAsConstants(sizeof(DirectX::XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
			m_RootParameters[1].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_PIXEL);
			m_SigDesc.Init(std::size(m_RootParameters), m_RootParameters, 0, nullptr, flags);
		}
		void RootSignature::Create(const ComPtr<ID3D12Device2>& device)
		{
			ComPtr<ID3D10Blob> signatureBlob;
			ComPtr<ID3D10Blob> errorBlob;

			if (const auto hr = D3D12SerializeRootSignature(&m_SigDesc, D3D_ROOT_SIGNATURE_VERSION_1,&signatureBlob, &errorBlob); FAILED(hr))
			{
				hr >> statusCode;
				if (errorBlob)
				{
					auto errorBufferPtr = static_cast<const char*>(errorBlob->GetBufferPointer());
					FPL_LOG(errorBufferPtr, "Error in Root Signature", Logs::API_ERROR);
				}
				else return;
			}
			device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature)) >> statusCode;
			FPL_LOG("Root Signature has been created", "Noo Error", Logs::INFO);

		}
	}
}

