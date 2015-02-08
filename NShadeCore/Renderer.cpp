#include "stdafx.h"
#include "renderer.h"

Renderer::Renderer(DeviceResources* pResources)
{
	m_pDeviceResources = shared_ptr<DeviceResources>(pResources);

	m_pInputLayout = 0;
	m_pSwapChain = 0;
	m_pRenderTargetView = 0;
	m_pBackBuffer = 0;
	m_pDepthStencilBuffer = 0;
	m_pDepthStencilState = 0;
	m_pDepthStencilView = 0;
	m_pRasterizerState = 0;

	m_useSwapchain = false;
}

Renderer::~Renderer()
{
	m_pDeviceResources.reset();
	m_vertexShader.reset();
	m_pixelShader.reset();
	m_pShaderSet.reset();

	delete m_pInputLayout;
	delete m_pSwapChain;
	delete m_pRenderTargetView;
	delete m_pBackBuffer;
	delete m_pDepthStencilBuffer;
	delete m_pDepthStencilState;
	delete m_pDepthStencilView;
	delete m_pRasterizerState;
}

HRESULT Renderer::Initialize()
{
	auto result = SetVertexShader(m_standardVertexShader);
	if (FAILED(result))
	{
		return result;
	}

	result = SetPixelShader(m_standardPixelShader);
	if (FAILED(result))
	{
		return result;
	}

	if (m_useSwapchain)
	{
		result = CreateSwapChain();
		if (FAILED(result))
		{
			return result;
		}

		result = CreateDepthBuffer();
		if (FAILED(result))
		{
			return result;
		}

		result = CreateDepthStencil();
		if (FAILED(result))
		{
			return result;
		}

		result = CreateRasterizer();
		if (FAILED(result))
		{
			return result;
		}
	}
	return result;
}

HRESULT Renderer::CreateSwapChainDesciption()
{
	ZeroMemory(&m_pSwapChainDescription, sizeof(m_pSwapChainDescription));

	m_pSwapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	m_pSwapChainDescription.BufferCount = 2;

	m_pSwapChainDescription.BufferDesc.Width = DeviceResource()->ScreenWidth;
	m_pSwapChainDescription.BufferDesc.Height = DeviceResource()->ScreenHeight;
	m_pSwapChainDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_pSwapChainDescription.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	m_pSwapChainDescription.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	m_pSwapChainDescription.BufferDesc.RefreshRate.Numerator = 0;
	m_pSwapChainDescription.BufferDesc.RefreshRate.Denominator = 1;
	m_pSwapChainDescription.SampleDesc.Count = DeviceResource()->SamplesCount;
	m_pSwapChainDescription.SampleDesc.Quality = 0;

	m_pSwapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	m_pSwapChainDescription.Flags = 0;

	auto handle = *DeviceResource()->WindowHandle;
	m_pSwapChainDescription.OutputWindow = handle;

	if (DeviceResource()->FullScreen)
	{
		m_pSwapChainDescription.Windowed = false;
	}
	else
	{
		m_pSwapChainDescription.Windowed = true;
	}
	return 0;
}

HRESULT Renderer::CreateSwapChain()
{
	auto result = CreateSwapChainDesciption();
	if (FAILED(result))
	{
		return result;
	}

	IDXGIDevice* dxgiDevice = 0;
	IDXGIAdapter* dxgiAdapter = 0;
	IDXGIFactory* dxgiFactory = 0;

	result = GetDevice()->QueryInterface(__uuidof(IDXGIDevice), (void **)&dxgiDevice);
	if (FAILED(result))
	{
		return result;
	}

	result = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void **)&dxgiAdapter);
	if (FAILED(result))
	{
		return result;
	}

	result = dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void **)&dxgiFactory);
	if (FAILED(result))
	{
		return result;
	}

	result = dxgiFactory->CreateSwapChain(GetDevice(), &m_pSwapChainDescription, &m_pSwapChain);
	if (FAILED(result))
	{
		return result;
	}

	result = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&m_pBackBuffer));
	if (FAILED(result))
	{
		return result;
	}

	result = GetDevice()->CreateRenderTargetView(m_pBackBuffer, NULL, &m_pRenderTargetView);
	if (FAILED(result))
	{
		return result;
	}

	dxgiDevice->Release();
	dxgiAdapter->Release();
	dxgiFactory->Release();

	return GetDevice()->CreateRenderTargetView(m_pBackBuffer, nullptr, &m_pRenderTargetView);
}


HRESULT Renderer::CreateDepthBufferDescription()
{
	ZeroMemory(&m_pDepthBufferDesc, sizeof(m_pDepthBufferDesc));

	m_pDepthBufferDesc.Width = DeviceResource()->ScreenWidth;
	m_pDepthBufferDesc.Height = DeviceResource()->ScreenHeight;
	m_pDepthBufferDesc.MipLevels = 1;
	m_pDepthBufferDesc.ArraySize = 1;
	m_pDepthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	m_pDepthBufferDesc.SampleDesc.Count = 1;
	m_pDepthBufferDesc.SampleDesc.Quality = 0;
	m_pDepthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	m_pDepthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	m_pDepthBufferDesc.CPUAccessFlags = 0;
	m_pDepthBufferDesc.MiscFlags = 0;

	return 0;
}

HRESULT Renderer::CreateDepthBuffer()
{
	auto result = CreateDepthBufferDescription();
	if (FAILED(result))
	{
		return result;
	}
	return GetDevice()->CreateTexture2D(&m_pDepthBufferDesc, NULL, &m_pDepthStencilBuffer);
}


HRESULT Renderer::CreateDepthStencilDescription()
{
	ZeroMemory(&m_pDepthStencilDesc, sizeof(m_pDepthStencilDesc));

	m_pDepthStencilDesc.DepthEnable = true;
	m_pDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	m_pDepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	m_pDepthStencilDesc.StencilEnable = true;
	m_pDepthStencilDesc.StencilReadMask = 0xFF;
	m_pDepthStencilDesc.StencilWriteMask = 0xFF;

	m_pDepthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	m_pDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	m_pDepthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	m_pDepthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	m_pDepthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	m_pDepthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	m_pDepthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	m_pDepthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	return GetDevice()->CreateDepthStencilState(&m_pDepthStencilDesc, &m_pDepthStencilState);
}

HRESULT Renderer::CreateDepthStencilViewDescription()
{
	ZeroMemory(&m_pDepthStencilViewDesc, sizeof(m_pDepthStencilViewDesc));

	m_pDepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	m_pDepthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	m_pDepthStencilViewDesc.Texture2D.MipSlice = 0;
	return 0;
}

HRESULT Renderer::CreateDepthStencil()
{
	auto result = CreateDepthStencilDescription();
	if (FAILED(result))
	{
		return result;
	}
	result = CreateDepthStencilViewDescription();
	if (FAILED(result))
	{
		return result;
	}
	result = GetDevice()->CreateDepthStencilView(m_pDepthStencilBuffer, &m_pDepthStencilViewDesc, &m_pDepthStencilView);
	if (FAILED(result))
	{
		return result;
	}
	GetDeviceContext()->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);
	return 0;
}


HRESULT Renderer::CreateRasterizerDescription()
{
	ZeroMemory(&m_pRasterizerDesc, sizeof(m_pRasterizerDesc));

	m_pRasterizerDesc.AntialiasedLineEnable = false;
	m_pRasterizerDesc.CullMode = D3D11_CULL_BACK;
	m_pRasterizerDesc.DepthBias = 0;
	m_pRasterizerDesc.DepthBiasClamp = 0.0f;
	m_pRasterizerDesc.DepthClipEnable = true;
	m_pRasterizerDesc.FillMode = D3D11_FILL_SOLID;
	m_pRasterizerDesc.FrontCounterClockwise = false;
	m_pRasterizerDesc.MultisampleEnable = false;
	m_pRasterizerDesc.ScissorEnable = false;
	m_pRasterizerDesc.SlopeScaledDepthBias = 0.0f;

	return 0;
}

HRESULT Renderer::CreateRasterizer()
{
	auto result = CreateRasterizerDescription();
	if (FAILED(result))
	{
		return result;
	}
	result = GetDevice()->CreateRasterizerState(&m_pRasterizerDesc, &m_pRasterizerState);
	if (FAILED(result))
	{
		return result;
	}
	GetDeviceContext()->RSSetState(m_pRasterizerState);
	return 0;
}


HRESULT Renderer::SetVertexShader(LPCWSTR compiledShaderFile)
{
	auto vsByteCode = File::ReadFileBytes(compiledShaderFile);
	auto shaders = DeviceResource()->Shaders;

	auto result = GetDevice()->CreateVertexShader(vsByteCode->FileBytes, vsByteCode->Length, NULL, &shaders->VertexShader);
	if (FAILED(result))
	{
		return result;
	}

	static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	return GetDevice()->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), vsByteCode->FileBytes, vsByteCode->Length, &m_pInputLayout);
}

HRESULT Renderer::CompileVertexShader(LPCWSTR compiledShaderFile)
{
	ID3DBlob* shaderBlob = 0;
	ID3D11VertexShader* vertexShader = 0;

	auto result = CompileShader(compiledShaderFile, shaderBlob, VS_PROFILE);
	if (FAILED(result))
	{
		return result;
	}
	return GetDevice()->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), NULL, &vertexShader);
}

HRESULT Renderer::SetHullShader(LPCWSTR compiledShaderFile)
{
	auto hsByteCode = File::ReadFileBytes(compiledShaderFile);
	auto shaders = DeviceResource()->Shaders;
	return GetDevice()->CreateHullShader(hsByteCode->FileBytes, hsByteCode->Length, NULL, &shaders->HullShader);
}

HRESULT Renderer::CompileHullShader(LPCWSTR compiledShaderFile)
{
	ID3DBlob* shaderBlob = 0;
	ID3D11HullShader* hullShader = 0;

	auto result = CompileShader(compiledShaderFile, shaderBlob, HS_PROFILE);
	if (FAILED(result))
	{
		return result;
	}
	return GetDevice()->CreateHullShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), NULL, &hullShader);
}

HRESULT Renderer::SetGeometryShader(LPCWSTR compiledShaderFile)
{
	auto gsByteCode = File::ReadFileBytes(compiledShaderFile);
	auto shaders = DeviceResource()->Shaders;
	return GetDevice()->CreateGeometryShader(gsByteCode->FileBytes, gsByteCode->Length, NULL, &shaders->GeometryShader);
}

HRESULT Renderer::CompileGeometryShader(LPCWSTR compiledShaderFile)
{
	ID3DBlob* shaderBlob = 0;
	ID3D11GeometryShader* geometryShader = 0;

	auto result = CompileShader(compiledShaderFile, shaderBlob, GS_PROFILE);
	if (FAILED(result))
	{
		return result;
	}
	return GetDevice()->CreateGeometryShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), NULL, &geometryShader);
}

HRESULT Renderer::SetPixelShader(LPCWSTR compiledShaderFile)
{
	auto psByteCode = File::ReadFileBytes(compiledShaderFile);
	auto shaders = DeviceResource()->Shaders;

	return GetDevice()->CreatePixelShader(psByteCode->FileBytes, psByteCode->Length, NULL, &shaders->PixelShader);
}

HRESULT Renderer::CompilePixelShader(LPCWSTR compiledShaderFile)
{
	ID3DBlob* shaderBlob = 0;
	ID3D11PixelShader* pixelShader = 0;

	auto result = CompileShader(compiledShaderFile, shaderBlob, PS_PROFILE);
	if (FAILED(result))
	{
		return result;
	}
	return GetDevice()->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), NULL, &pixelShader);
}

HRESULT Renderer::CompileShader(LPCWSTR compiledShaderFile, ID3DBlob *blob, LPCSTR shaderProfile)
{
	ID3DBlob* shaderBlob = 0;

	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3DCOMPILE_DEBUG;
#endif

	const D3D_SHADER_MACRO defines[] =
	{
		"EXAMPLE_DEFINE",
		"1",
		NULL,
		NULL
	};

	auto result = D3DCompileFromFile(
		compiledShaderFile,
		NULL, // Defines
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main", // Entrypoint
		shaderProfile,
		flags,
		0,
		&shaderBlob,
		&shaderBlob);

	return result;
}


void Renderer::Render()
{
	if (m_useSwapchain)
	{
		RenderWithSwapchain();
	}
	else
	{
		RenderWithoutSwapchain();
	}
}

void Renderer::RenderWithSwapchain()
{
	m_pSwapChain->Present(1, 0);
}

void Renderer::RenderWithoutSwapchain()
{
	auto context = DeviceResource()->DeviceContext;
	auto constBuffer = DeviceResource()->ConstantBuffer;
	auto bufferData = DeviceResource()->ConstantBufferData;

	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;

	context->UpdateSubresource(constBuffer, 0, NULL, bufferData, 0, 0);
	context->IASetVertexBuffers(0, 1, &DeviceResource()->VertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(DeviceResource()->IndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(m_pInputLayout);
	context->VSSetShader(DeviceResource()->Shaders->VertexShader, nullptr, 0);
	context->PSSetShader(DeviceResource()->Shaders->PixelShader, nullptr, 0);
	context->DrawIndexed(DeviceResource()->IndexCount, 0, 0);
}