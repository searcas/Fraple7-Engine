#include "pch.h"
#include "ShadersLoader.h"
#include <d3dcompiler.h>
namespace Fraple7
{
	namespace Core
	{

		ShadersLoader::ShadersLoader()
		{
		}

		void ShadersLoader::Invoke(ShaderType type)
		{
			switch (type)
			{
			case Fraple7::Core::ShadersLoader::Vertex:
				VertexShader();
				break;
			case Fraple7::Core::ShadersLoader::Pixel:
				PixelShader();
				break;
			case Fraple7::Core::ShadersLoader::Compute:
				ComputeShader();
				break;
			case Fraple7::Core::ShadersLoader::Geometry:
				GeometryShader();
				break;
			default:
				break;
			}
		}
		
		void ShadersLoader::PixelShader()
		{
			D3DReadFileToBlob(L"PixelShader.cso", &m_Shaders[ShaderType::Pixel]) >> statusCode;
		}

		void ShadersLoader::VertexShader()
		{
			D3DReadFileToBlob(L"VertexShader.cso", &m_Shaders[ShaderType::Vertex]) >> statusCode;
		}
		void ShadersLoader::GeometryShader()
		{
			D3DReadFileToBlob(L"GeometryShader.cso", &m_Shaders[ShaderType::Geometry]) >> statusCode;
		}
		void ShadersLoader::ComputeShader()
		{
			D3DReadFileToBlob(L"ComputeShader.cso", &m_Shaders[ShaderType::Compute]) >> statusCode;
		}

	}
}