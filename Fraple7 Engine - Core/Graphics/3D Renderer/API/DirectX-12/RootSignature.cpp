#include "pch.h"
#include "RootSignature.h"

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
			m_SigDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
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

