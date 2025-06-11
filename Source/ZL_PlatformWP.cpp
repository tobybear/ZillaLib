/*
  ZillaLib
  Copyright (C) 2010-2019 Bernhard Schelling

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#if defined(WINAPI_FAMILY) && WINAPI_FAMILY==WINAPI_FAMILY_PHONE_APP
#include "ZL_Platform.h"
#include "ZL_Impl.h"
#include "ZL_Signal.h"
#include "ZL_Application.h"
#include "ZL_Display.h"
#include "ZL_Display_Impl.h"
#include "ZL_Data.h"

#include <wrl/client.h>
#include <d3d11_1.h>
#include <DirectXMath.h>
#include <memory>
#include <agile.h>
#include <wrl/client.h>
#include <ppl.h>
#include <ppltasks.h>
#include <Xaudio2.h>

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::ViewManagement;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Windows::Storage;
using namespace Concurrency;
using namespace DirectX;

#ifdef ZL_DOUBLE_PRECISCION
#error The Direct3D renderer is only implemented for float preciscion
#endif

namespace ZLD3D
{
	#include <ColorPixelShader.h>
	#include <ColorVertexShader.h>
	#include <TexturePixelShader.h>
	#include <TextureVertexShader.h>

	static ID3D11Buffer* ConstantBuffers[3] = { NULL, NULL, NULL };
	static ID3D11InputLayout*  COLOR_INPUTLAYOUT  = NULL, *TEXTURE_INPUTLAYOUT  = NULL;
	static ID3D11VertexShader* COLOR_VERTEXSHADER = NULL, *TEXTURE_VERTEXSHADER = NULL;
	static ID3D11PixelShader*  COLOR_PIXELSHADER  = NULL, *TEXTURE_PIXELSHADER  = NULL;
	static ID3D11Buffer *VertexPosBuffer = NULL, *VertexColorBuffer = NULL, *VertexTexCoordBuffer = NULL, *IndexBuffer = NULL;
	static GLsizei VertexBufferSize = 0, IndexBufferSize = 0;
	static IDXGISwapChain1* SwapChain = NULL;
	static ID3D11RenderTargetView* RenderTargetView = NULL;
	static ID3D11BlendState* BlendState = NULL;
	static ID3D11RasterizerState *RasterStateScissor = NULL, *RasterStateFull = NULL;
	static ID3D11RasterizerState **RasterStateActive = &RasterStateFull;
	static ID3D11SamplerState *SamplerStateClamp = NULL, *SamplerStateWrap = NULL;
	static struct Texture { ID3D11ShaderResourceView* shaderresource; ID3D11SamplerState** samplerstate; } *TextureBound = NULL;
	static std::vector<Texture*> Textures;
	static bool USE_GLOBAL_COLOR = true;
	static GLsizei INDEX_BUFFER_APPLIED = 0;
	static const GLvoid *ATTR_POSITION_ptr = NULL, *ATTR_COLOR_ptr = NULL, *ATTR_TEXCOORD_ptr = NULL;
	static       GLint   ATTR_POSITION_size = 0,    ATTR_COLOR_size = 0,    ATTR_TEXCOORD_size = 0;
};

// Cached renderer properties.
Windows::Foundation::Size m_renderTargetSize;
Windows::Foundation::Rect m_windowBounds;
Platform::Agile<Windows::UI::Core::CoreWindow> m_window;

// Direct3D Objects.
ID3D11Device* m_d3dDevice = NULL;
ID3D11DeviceContext* m_d3dContext = NULL;

bool m_windowClosed = false;
bool m_windowVisible = true;

static LARGE_INTEGER ticks_startTime;
static LONGLONG ticks_frequencyMs;

static unsigned int WP_WindowFlags = ZL_WINDOW_FULLSCREEN | ZL_WINDOW_INPUT_FOCUS | ZL_WINDOW_MOUSE_FOCUS;
#define MAX_SIMULTANEOUS_TOUCHES 10
struct WP_Touch { int lastx, lasty; uint32 pointerId; };
static WP_Touch WP_Touches[10];

static ZL_String jsonConfigFile;
static ZL_Json jsonConfig;

// --------------------------------------------------------------------------------------------------------------------------------------------

namespace ZLGLSL { void MatrixApply(); };
void OnKey(int VirtualKey, bool down);

inline void ThrowIfFailed(HRESULT hr) { if (FAILED(hr)) {
	HRESULT bla = m_d3dDevice->GetDeviceRemovedReason();
	throw Platform::Exception::CreateException(hr); } }

ref class _ZLWindowsApplication sealed : public Windows::ApplicationModel::Core::IFrameworkView
{
public:
	virtual void Load(Platform::String^ entryPoint) { }
	virtual void Uninitialize() { }

	virtual void Initialize(CoreApplicationView^ applicationView)
	{
		//ZL_LOG0("WP8", "App::Initialize\n");
		applicationView->Activated += ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &_ZLWindowsApplication::OnActivated);
		CoreApplication::Suspending += ref new EventHandler<SuspendingEventArgs^>(this, &_ZLWindowsApplication::OnSuspending);
		CoreApplication::Resuming += ref new EventHandler<Platform::Object^>(this, &_ZLWindowsApplication::OnResuming);
	}

	virtual void SetWindow(CoreWindow^ window)
	{
		//ZL_LOG0("WP8", "App::SetWindow\n");
		window->VisibilityChanged += ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &_ZLWindowsApplication::OnVisibilityChanged);
		window->Closed += ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &_ZLWindowsApplication::OnWindowClosed);
		window->PointerPressed += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &_ZLWindowsApplication::OnPointerPressed);
		window->PointerMoved += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &_ZLWindowsApplication::OnPointerMoved);
		window->PointerReleased += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &_ZLWindowsApplication::OnPointerReleased);
		window->SizeChanged += ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &_ZLWindowsApplication::OnSizeChanged);
		window->Activated += ref new TypedEventHandler<CoreWindow^, WindowActivatedEventArgs^>(this, &_ZLWindowsApplication::OnWindowActivationChanged);
		window->KeyDown += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &_ZLWindowsApplication::OnKeyDown);
		window->KeyUp += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &_ZLWindowsApplication::OnKeyUp);
		window->CharacterReceived += ref new TypedEventHandler<CoreWindow^, CharacterReceivedEventArgs^>(this, &_ZLWindowsApplication::OnCharacterReceived);
		InputPane::GetForCurrentView()->Hiding += ref new TypedEventHandler<InputPane^, InputPaneVisibilityEventArgs^>(this, &_ZLWindowsApplication::OnInputPaneHiding);
		window->IsInputEnabled = true;
		Windows::Phone::UI::Input::HardwareButtons::BackPressed += ref new EventHandler<Windows::Phone::UI::Input::BackPressedEventArgs^>(this, &_ZLWindowsApplication::OnBackPressed);
		DisplayProperties::AutoRotationPreferences = DisplayOrientations::Landscape | DisplayOrientations::LandscapeFlipped;
		m_window = window;
		CreateDeviceResources();
		CreateWindowSizeDependentResources();
		memset(WP_Touches, 0, sizeof(WP_Touches));
		for (int i = 0; i < MAX_SIMULTANEOUS_TOUCHES; i++) WP_Touches[i].lastx = WP_Touches[i].lasty = -1;
	}

	virtual void Run()
	{
		ZillaLibInit(0, NULL);
		while (!m_windowClosed)
		{
			if (m_windowVisible)
			{
				m_window->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
				ZL_MainApplication->Frame();
				HRESULT hr = ZLD3D::SwapChain->Present(1, 0); // Present, this call is synchronized to the display frame rate.
				if (hr == DXGI_ERROR_DEVICE_REMOVED)
				{
					// Recreate all device resources and set them back to the current state. Reset these member variables to ensure that UpdateForWindowSizeChange recreates all resources.
					m_windowBounds.Width = m_windowBounds.Height = 0;
					CreateDeviceResources();
					UpdateForWindowSizeChange();
				}
				else ThrowIfFailed(hr);
			}
			else m_window->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
		}
	}

	void CreateDeviceResources()
	{
		UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
		#if defined(_DEBUG)
		creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
		#endif

		// Create the Direct3D 11 API device object and a corresponding context.
		D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_9_3 };
		if (m_d3dDevice) { m_d3dDevice->Release(); m_d3dDevice = NULL; }
		ThrowIfFailed(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &m_d3dDevice, NULL, &m_d3dContext));

		// create the blend state.
		D3D11_BLEND_DESC blendDesc = {0};
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		if (ZLD3D::BlendState) { ZLD3D::BlendState->Release(); ZLD3D::BlendState = NULL; }
		ThrowIfFailed(m_d3dDevice->CreateBlendState(&blendDesc, &ZLD3D::BlendState));

		// create the sampler
		D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
		samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		if (ZLD3D::SamplerStateClamp) { ZLD3D::SamplerStateClamp->Release(); ZLD3D::SamplerStateClamp = NULL; }
		ThrowIfFailed(m_d3dDevice->CreateSamplerState(&samplerDesc, &ZLD3D::SamplerStateClamp));
		samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		if (ZLD3D::SamplerStateWrap) { ZLD3D::SamplerStateWrap->Release(); ZLD3D::SamplerStateWrap = NULL; }
		ThrowIfFailed(m_d3dDevice->CreateSamplerState(&samplerDesc, &ZLD3D::SamplerStateWrap));

		if (ZLD3D::COLOR_INPUTLAYOUT) { ZLD3D::COLOR_INPUTLAYOUT->Release(); ZLD3D::COLOR_INPUTLAYOUT = NULL; }
		if (ZLD3D::COLOR_VERTEXSHADER) { ZLD3D::COLOR_VERTEXSHADER->Release(); ZLD3D::COLOR_VERTEXSHADER = NULL; }
		if (ZLD3D::COLOR_PIXELSHADER) { ZLD3D::COLOR_PIXELSHADER->Release(); ZLD3D::COLOR_PIXELSHADER = NULL; }
		if (ZLD3D::TEXTURE_INPUTLAYOUT) { ZLD3D::TEXTURE_INPUTLAYOUT->Release(); ZLD3D::TEXTURE_INPUTLAYOUT = NULL; }
		if (ZLD3D::TEXTURE_VERTEXSHADER) { ZLD3D::TEXTURE_VERTEXSHADER->Release(); ZLD3D::TEXTURE_VERTEXSHADER = NULL; }
		if (ZLD3D::TEXTURE_PIXELSHADER) { ZLD3D::TEXTURE_PIXELSHADER->Release(); ZLD3D::TEXTURE_PIXELSHADER = NULL; }
		if (ZLD3D::ConstantBuffers[0]) { ZLD3D::ConstantBuffers[0]->Release(); ZLD3D::ConstantBuffers[0] = NULL; }
		if (ZLD3D::ConstantBuffers[1]) { ZLD3D::ConstantBuffers[1]->Release(); ZLD3D::ConstantBuffers[1] = NULL; }
		if (ZLD3D::ConstantBuffers[2]) { ZLD3D::ConstantBuffers[2]->Release(); ZLD3D::ConstantBuffers[2] = NULL; }

		const D3D11_INPUT_ELEMENT_DESC vertexDescColor[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		ThrowIfFailed(m_d3dDevice->CreateInputLayout(vertexDescColor, ARRAYSIZE(vertexDescColor), ZLD3D::c_ColorVertexShader, sizeof(ZLD3D::c_ColorVertexShader), &ZLD3D::COLOR_INPUTLAYOUT));
		ThrowIfFailed(m_d3dDevice->CreateVertexShader(ZLD3D::c_ColorVertexShader, sizeof(ZLD3D::c_ColorVertexShader), nullptr, &ZLD3D::COLOR_VERTEXSHADER));
		ThrowIfFailed(m_d3dDevice->CreatePixelShader(ZLD3D::c_ColorPixelShader, sizeof(ZLD3D::c_ColorPixelShader), nullptr, &ZLD3D::COLOR_PIXELSHADER));

		const D3D11_INPUT_ELEMENT_DESC vertexDescTexture[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		ThrowIfFailed(m_d3dDevice->CreateInputLayout(vertexDescTexture, ARRAYSIZE(vertexDescTexture), ZLD3D::c_TextureVertexShader, sizeof(ZLD3D::c_TextureVertexShader), &ZLD3D::TEXTURE_INPUTLAYOUT));
		ThrowIfFailed(m_d3dDevice->CreateVertexShader(ZLD3D::c_TextureVertexShader, sizeof(ZLD3D::c_TextureVertexShader), nullptr, &ZLD3D::TEXTURE_VERTEXSHADER));
		ThrowIfFailed(m_d3dDevice->CreatePixelShader(ZLD3D::c_TexturePixelShader, sizeof(ZLD3D::c_TexturePixelShader), nullptr, &ZLD3D::TEXTURE_PIXELSHADER));

		CD3D11_BUFFER_DESC constantBuffer0Desc(sizeof(XMMATRIX), D3D11_BIND_CONSTANT_BUFFER);
		ThrowIfFailed(m_d3dDevice->CreateBuffer(&constantBuffer0Desc, nullptr, &ZLD3D::ConstantBuffers[0]));
		CD3D11_BUFFER_DESC constantBuffer1Desc(sizeof(XMFLOAT4), D3D11_BIND_CONSTANT_BUFFER);
		ThrowIfFailed(m_d3dDevice->CreateBuffer(&constantBuffer1Desc, nullptr, &ZLD3D::ConstantBuffers[1]));
		CD3D11_BUFFER_DESC constantBuffer2Desc(16, D3D11_BIND_CONSTANT_BUFFER);
		ThrowIfFailed(m_d3dDevice->CreateBuffer(&constantBuffer2Desc, nullptr, &ZLD3D::ConstantBuffers[2]));
		ZLGLSL::MatrixApply();  //resets ConstantBuffers[0]
		glVertexAttrib4f(ZLGLSL::ATTR_COLOR, 1, 1, 1, 1); //resets ConstantBuffers[1]
		glDisableVertexAttribArray(ZLGLSL::ATTR_COLOR); //resets ConstantBuffers[2]

		m_d3dContext->OMSetBlendState(ZLD3D::BlendState, NULL, 0xffffffff);
		m_d3dContext->VSSetConstantBuffers(0, 3, ZLD3D::ConstantBuffers);
	}

	// Method to convert a length in device-independent pixels (DIPs) to a length in physical pixels.
	float ConvertDipsToPixels(float dips)
	{
		static const float dipsPerInch = 96.0f;
		return floor(dips * DisplayProperties::LogicalDpi / dipsPerInch + 0.5f); // Round to nearest integer.
	}

	void CreateWindowSizeDependentResources()
	{
		m_windowBounds = m_window->Bounds;
		Windows::UI::Core::CoreWindow^ window = m_window.Get();

		// Calculate the necessary swap chain and render target size in pixels.
		m_renderTargetSize.Width = ConvertDipsToPixels(m_windowBounds.Width);
		m_renderTargetSize.Height = ConvertDipsToPixels(m_windowBounds.Height);
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};
		swapChainDesc.Width = static_cast<UINT>(m_renderTargetSize.Width); // Match the size of the window.
		swapChainDesc.Height = static_cast<UINT>(m_renderTargetSize.Height);
		swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // This is the most common swap chain format.
		swapChainDesc.Stereo = false;
		swapChainDesc.SampleDesc.Count = 1; // Don't use multi-sampling.
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1; // On phone, only single buffering is supported.
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH; // On phone, only stretch and aspect-ratio stretch scaling are allowed.
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // On phone, no swap effects are supported.
		swapChainDesc.Flags = 0;

		if (ZLD3D::SwapChain) { ZLD3D::SwapChain->Release(); ZLD3D::SwapChain = NULL; }
		IDXGIDevice1* dxgiDevice;
		IDXGIAdapter* dxgiAdapter;
		IDXGIFactory2* dxgiFactory;
		m_d3dDevice->QueryInterface(__uuidof(IDXGIDevice1), (void**)&dxgiDevice);
		dxgiDevice->GetAdapter(&dxgiAdapter);
		dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&dxgiFactory);
		dxgiFactory->CreateSwapChainForCoreWindow(m_d3dDevice, reinterpret_cast<IUnknown*>(window), &swapChainDesc, nullptr, &ZLD3D::SwapChain);
		dxgiDevice->SetMaximumFrameLatency(1); // Ensure that DXGI does not queue more than one frame at once (reduces latency and ensures that the application will only render after each VSync, minimizing power consumption)
		dxgiFactory->Release();
		dxgiAdapter->Release();
		dxgiDevice->Release();

		// Create a render target view of the swap chain back buffer.
		ID3D11Texture2D* backBuffer;
		ZLD3D::SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
		if (ZLD3D::RenderTargetView) { ZLD3D::RenderTargetView->Release(); ZLD3D::RenderTargetView = NULL; }
		m_d3dDevice->CreateRenderTargetView(backBuffer, nullptr, &ZLD3D::RenderTargetView);
		backBuffer->Release();

		// Set the rendering viewport to target the entire window.
		CD3D11_VIEWPORT viewport(0.0f, 0.0f, m_renderTargetSize.Width, m_renderTargetSize.Height);

		// Setup the raster description which will determine how and what polygons will be drawn.
		D3D11_RASTERIZER_DESC rasterDesc;
		rasterDesc.AntialiasedLineEnable = false;
		rasterDesc.CullMode = D3D11_CULL_NONE;
		rasterDesc.DepthBias = 0;
		rasterDesc.DepthBiasClamp = 0.0f;
		rasterDesc.DepthClipEnable = true;
		rasterDesc.FillMode = D3D11_FILL_SOLID;
		rasterDesc.FrontCounterClockwise = false;
		rasterDesc.MultisampleEnable = false;
		rasterDesc.ScissorEnable = false;
		rasterDesc.SlopeScaledDepthBias = 0.0f;
		if (ZLD3D::RasterStateFull) { ZLD3D::RasterStateFull->Release(); ZLD3D::RasterStateFull = NULL; }
		if (ZLD3D::RasterStateScissor) { ZLD3D::RasterStateScissor->Release(); ZLD3D::RasterStateScissor = NULL; }
		m_d3dDevice->CreateRasterizerState(&rasterDesc, &ZLD3D::RasterStateFull);
		rasterDesc.ScissorEnable = true;
		m_d3dDevice->CreateRasterizerState(&rasterDesc, &ZLD3D::RasterStateScissor);

		m_d3dContext->RSSetViewports(1, &viewport);
		m_d3dContext->RSSetState(*ZLD3D::RasterStateActive);
		m_d3dContext->OMSetRenderTargets(1, &ZLD3D::RenderTargetView, NULL);
	}

	void UpdateForWindowSizeChange()
	{
		// This method is called in the event handler for the SizeChanged event.
		if (m_window->Bounds.Width  == m_windowBounds.Width && m_window->Bounds.Height == m_windowBounds.Height) return;
		ID3D11RenderTargetView* nullViews[] = {nullptr};
		m_d3dContext->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);
		if (ZLD3D::RenderTargetView) { ZLD3D::RenderTargetView->Release(); ZLD3D::RenderTargetView = NULL; }
		m_d3dContext->Flush();
		CreateWindowSizeDependentResources();
	}

	// Event Handlers.
	void OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
	{
		//ZL_LOG0("WP8", "App::OnVisibilityChanged\n");
		m_windowVisible = args->Visible;
	}

	void OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
	{
		//ZL_LOG0("WP8", "App::OnWindowClosed\n");
		m_windowClosed = true;
	}

	void OnSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
	{
		//ZL_LOG0("WP8", "App::OnSizeChanged\n");
		m_windowBounds.Width = args->Size.Width;
		m_windowBounds.Height = args->Size.Height;
		UpdateForWindowSizeChange();
	}

	void OnWindowActivationChanged(CoreWindow^ sender, WindowActivatedEventArgs^ args)
	{
		//ZL_LOG0("WP8", "App::OnWindowActivationChanged\n");
		//Focused = (args->WindowActivationState != CoreWindowActivationState::Deactivated)
	}

	void OnPointerPressed(CoreWindow^ sender, PointerEventArgs^ args)
	{
		//ZL_LOG1("WP", "PointerPressed - TouchID: %d", args->CurrentPoint->PointerId);
		WP_Touch* p = WP_Touches;
		while (p->pointerId && (p-WP_Touches) < MAX_SIMULTANEOUS_TOUCHES) p++;
		if ((p-WP_Touches) == MAX_SIMULTANEOUS_TOUCHES) return;
		p->pointerId = args->CurrentPoint->PointerId;
		ZL_Event e;
		e.type = ZL_EVENT_MOUSEBUTTONDOWN;
		e.button.which = (p-WP_Touches);
		e.button.button = ZL_BUTTON_LEFT;
		e.button.is_down = true;
		e.button.x = p->lastx = (int)ConvertDipsToPixels(args->CurrentPoint->Position.Y);
		e.button.y = p->lasty = (int)(m_renderTargetSize.Width - ConvertDipsToPixels(args->CurrentPoint->Position.X));
		ZL_Display_Process_Event(e);
	}

	void OnPointerMoved(CoreWindow^ sender, PointerEventArgs^ args)
	{
		unsigned int argPointerId = args->CurrentPoint->PointerId;
		WP_Touch* p = WP_Touches;
		while (p->pointerId != argPointerId && (p-WP_Touches) < MAX_SIMULTANEOUS_TOUCHES) p++;
		if ((p-WP_Touches) == MAX_SIMULTANEOUS_TOUCHES) return;
		ZL_Event e;
		e.type = ZL_EVENT_MOUSEMOTION;
		e.motion.which = (p-WP_Touches);
		e.motion.state = args->CurrentPoint->IsInContact;
		e.motion.x = (int)ConvertDipsToPixels(args->CurrentPoint->Position.Y);
		e.motion.xrel = (e.motion.x - p->lastx);
		e.motion.y = (int)(m_renderTargetSize.Width - ConvertDipsToPixels(args->CurrentPoint->Position.X));
		e.motion.yrel = (e.motion.y - p->lasty);
		ZL_Display_Process_Event(e);
		p->lastx = e.motion.x;
		p->lasty = e.motion.y;
	}

	void OnPointerReleased(CoreWindow^ sender, PointerEventArgs^ args)
	{
		//ZL_LOG1("WP", "PointerReleased - TouchID: %d", args->CurrentPoint->PointerId);
		unsigned int argPointerId = args->CurrentPoint->PointerId;
		WP_Touch* p = WP_Touches;
		while (p->pointerId != argPointerId && (p-WP_Touches) < MAX_SIMULTANEOUS_TOUCHES) p++;
		if ((p-WP_Touches) == MAX_SIMULTANEOUS_TOUCHES) return;
		p->pointerId = 0;
		ZL_Event e;
		e.type = ZL_EVENT_MOUSEBUTTONUP;
		e.button.which = (p-WP_Touches);
		e.button.button = ZL_BUTTON_LEFT;
		e.button.is_down = false;
		e.button.x = p->lastx = (int)ConvertDipsToPixels(args->CurrentPoint->Position.Y);
		e.button.y = p->lasty = (int)(m_renderTargetSize.Width - ConvertDipsToPixels(args->CurrentPoint->Position.X));
		ZL_Display_Process_Event(e);
	}

	void OnKeyDown(CoreWindow^ sender, KeyEventArgs^ args)
	{
		int vk = (int)args->VirtualKey;
		OnKey(vk, true);
		args->Handled = true;
		if (vk == VK_BACK)
		{
			ZL_Event e;
			e.type = ZL_EVENT_TEXTINPUT;
			e.text.text[0] = '\b'; e.text.text[1] = 0;
			ZL_Display_Process_Event(e);
		}
	}

	void OnKeyUp(CoreWindow^ sender, KeyEventArgs^ args)
	{
		OnKey((int)args->VirtualKey, false);
		args->Handled = true;
	}

	void OnBackPressed(Platform::Object^ sender, Windows::Phone::UI::Input::BackPressedEventArgs^ args)
	{
		OnKey(VK_ESCAPE, true);
		OnKey(VK_ESCAPE, false);
		if (!(ZL_MainApplicationFlags & ZL_APPLICATION_DONE)) _ZL_Display_KeepAlive();
		if (!(ZL_MainApplicationFlags & ZL_APPLICATION_DONE)) args->Handled = true;
	}

	void OnCharacterReceived(CoreWindow^ sender, CharacterReceivedEventArgs^ args)
	{
		ZL_Event e;
		e.type = ZL_EVENT_TEXTINPUT;
		unsigned int code = args->KeyCode;
		if (code >= 0x800) { e.text.text[0] = char(0xE0 + ((code >> 12) & 0xF)); e.text.text[1] = char(0x80 + ((code >> 6) & 0x3F)); e.text.text[2] = char(0x80 + ((code) & 0x3F)); e.text.text[3] = 0; }
		else if (code >= 0x80) { e.text.text[0] = char(0xC0 + ((code >> 6) & 0x1F)); e.text.text[1] = char(0x80 + ((code) & 0x3F)); e.text.text[2] = 0; }
		else { e.text.text[0] = char(code == '\r' ? '\n' : code); e.text.text[1] = 0; }
		ZL_Display_Process_Event(e);
	}

	void OnInputPaneHiding(InputPane^ sender, InputPaneVisibilityEventArgs^ args)
	{
		//ZL_LOG0("WP8", "App::OnInputPaneHiding\n");
		m_window->IsKeyboardInputEnabled = false;
	}

	void OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
	{
		//ZL_LOG0("WP8", "App::OnActivated\n");
		//m_currentOrientation = DisplayOrientations::Landscape;
		DisplayProperties::OrientationChanged += ref new DisplayPropertiesEventHandler(this, &_ZLWindowsApplication::OnOrientationChanged);
		DisplayProperties::AutoRotationPreferences = DisplayOrientations::Landscape | DisplayOrientations::LandscapeFlipped;
		CoreWindow::GetForCurrentThread()->Activate();
	}

	void OnOrientationChanged(Platform::Object^ sender)
	{
		//ZL_LOG0("WP8", "App::OnOrientationChanged\n");
		//ZL_LOG2("WP8", "CurrentOrientation: %d - NativeOrientation: %d", (int)DisplayProperties::CurrentOrientation, (int)DisplayProperties::NativeOrientation);
	}

	void OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
	{
		//ZL_LOG0("WP8", "App::OnSuspending\n");
		SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();
		if (ZLD3D::SwapChain) { ZLD3D::SwapChain->Release(); ZLD3D::SwapChain = NULL; }
		if (ZLD3D::RenderTargetView) { ZLD3D::RenderTargetView->Release(); ZLD3D::RenderTargetView = NULL; }
		create_task([this, deferral]() { deferral->Complete(); });
	}

	void OnResuming(Platform::Object^ sender, Platform::Object^ args)
	{
		//ZL_LOG0("WP8", "App::OnResuming\n");
		CreateWindowSizeDependentResources();
		memset(WP_Touches, 0, sizeof(WP_Touches));
		for (int i = 0; i < MAX_SIMULTANEOUS_TOUCHES; i++) WP_Touches[i].lastx = WP_Touches[i].lasty = -1;
	}
};

//main
ref class _ZLWindowsApplicationSource sealed : Windows::ApplicationModel::Core::IFrameworkViewSource
{ public: virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView() { return ref new _ZLWindowsApplication(); } };
[Platform::MTAThread] int main(Platform::Array<Platform::String^>^) { CoreApplication::Run(ref new _ZLWindowsApplicationSource()); return 0; }

void __wp8_log(const char* logtag, const char* logtext)
{
	char loglog[1024];
	sprintf(loglog, "[%4d.%03d] [%10s] %s\n", 0, 0, logtag, logtext);
	OutputDebugStringA(loglog);
}

void OnKey(int VirtualKey, bool down)
{
	static const ZL_Key win_zlkey_table[] = {
		ZLK_UNKNOWN,    ZLK_UNKNOWN,      ZLK_UNKNOWN,     ZLK_CANCEL,      ZLK_UNKNOWN,      ZLK_UNKNOWN,    ZLK_UNKNOWN,        ZLK_UNKNOWN,      ZLK_BACKSPACE,    ZLK_TAB,
		ZLK_KP_ENTER,   ZLK_UNKNOWN,      ZLK_CLEAR,       ZLK_RETURN,      ZLK_UNKNOWN,      ZLK_UNKNOWN,    ZLK_LSHIFT,         ZLK_LCTRL,        ZLK_APPLICATION,  ZLK_PAUSE,
		ZLK_CAPSLOCK,   ZLK_LANG3,        ZLK_UNKNOWN,     ZLK_LANG1,       ZLK_LANG6,        ZLK_LANG2,      ZLK_UNKNOWN,        ZLK_ESCAPE,       ZLK_UNKNOWN,      ZLK_UNKNOWN,
		ZLK_UNKNOWN,    ZLK_MODE,         ZLK_SPACE,       ZLK_PAGEUP,      ZLK_PAGEDOWN,     ZLK_END,        ZLK_HOME,           ZLK_LEFT,         ZLK_UP,           ZLK_RIGHT,
		ZLK_DOWN,       ZLK_SELECT,       ZLK_PRINTSCREEN, ZLK_EXECUTE,     ZLK_PRINTSCREEN,  ZLK_INSERT,     ZLK_DELETE,         ZLK_HELP,         ZLK_0,            ZLK_1,
		ZLK_2,          ZLK_3,            ZLK_4,           ZLK_5,           ZLK_6,            ZLK_7,          ZLK_8,              ZLK_9,            ZLK_UNKNOWN,      ZLK_UNKNOWN,
		ZLK_UNKNOWN,    ZLK_UNKNOWN,      ZLK_UNKNOWN,     ZLK_UNKNOWN,     ZLK_UNKNOWN,      ZLK_A,          ZLK_B,              ZLK_C,            ZLK_D,            ZLK_E,
		ZLK_F,          ZLK_G,            ZLK_H,           ZLK_I,           ZLK_J,            ZLK_K,          ZLK_L,              ZLK_M,            ZLK_N,            ZLK_O,
		ZLK_P,          ZLK_Q,            ZLK_R,           ZLK_S,           ZLK_T,            ZLK_U,          ZLK_V,              ZLK_W,            ZLK_X,            ZLK_Y,
		ZLK_Z,          ZLK_LGUI,         ZLK_RGUI,        ZLK_APPLICATION, ZLK_UNKNOWN,      ZLK_UNKNOWN,    ZLK_KP_0,           ZLK_KP_1,         ZLK_KP_2,         ZLK_KP_3,
		ZLK_KP_4,       ZLK_KP_5,         ZLK_KP_6,        ZLK_KP_7,        ZLK_KP_8,         ZLK_KP_9,       ZLK_KP_MULTIPLY,    ZLK_KP_PLUS,      ZLK_SEPARATOR,    ZLK_KP_MINUS,
		ZLK_KP_DECIMAL, ZLK_KP_DIVIDE,    ZLK_F1,          ZLK_F2,          ZLK_F3,           ZLK_F4,         ZLK_F5,             ZLK_F6,           ZLK_F7,           ZLK_F8,
		ZLK_F9,         ZLK_F10,          ZLK_F11,         ZLK_F12,         ZLK_F13,          ZLK_F14,        ZLK_F15,            ZLK_F16,          ZLK_AUDIONEXT,    ZLK_AUDIOPREV,
		ZLK_AUDIOSTOP,  ZLK_AUDIOPLAY,    ZLK_MAIL,        ZLK_MEDIASELECT, ZLK_WWW,          ZLK_CALCULATOR, ZLK_UNKNOWN,        ZLK_UNKNOWN,      ZLK_UNKNOWN,      ZLK_UNKNOWN,
		ZLK_UNKNOWN,    ZLK_UNKNOWN,      ZLK_UNKNOWN,     ZLK_UNKNOWN,     ZLK_NUMLOCKCLEAR, ZLK_SCROLLLOCK, ZLK_KP_EQUALS,      ZLK_UNKNOWN,      ZLK_UNKNOWN,      ZLK_UNKNOWN,
		ZLK_UNKNOWN,    ZLK_UNKNOWN,      ZLK_UNKNOWN,     ZLK_UNKNOWN,     ZLK_UNKNOWN,      ZLK_UNKNOWN,    ZLK_UNKNOWN,        ZLK_UNKNOWN,      ZLK_UNKNOWN,      ZLK_UNKNOWN,
		ZLK_LSHIFT,     ZLK_RSHIFT,       ZLK_LCTRL,       ZLK_RCTRL,       ZLK_LALT,         ZLK_RALT,       ZLK_AC_BACK,        ZLK_AC_FORWARD,   ZLK_AC_REFRESH,   ZLK_AC_STOP,
		ZLK_AC_SEARCH,  ZLK_AC_BOOKMARKS, ZLK_AC_HOME,     ZLK_AUDIOMUTE,   ZLK_VOLUMEDOWN,   ZLK_VOLUMEUP,   ZLK_UNKNOWN,        ZLK_KP_000,       ZLK_KP_EQUALS,    ZLK_KP_00,
		ZLK_UNKNOWN,    ZLK_UNKNOWN,      ZLK_UNKNOWN,     ZLK_UNKNOWN,     ZLK_UNKNOWN,      ZLK_UNKNOWN,    ZLK_SEMICOLON,      ZLK_EQUALS,       ZLK_COMMA,        ZLK_MINUS,
		ZLK_PERIOD,     ZLK_SLASH,        ZLK_GRAVE,       ZLK_UNKNOWN,     ZLK_UNKNOWN,      ZLK_UNKNOWN,    ZLK_UNKNOWN,        ZLK_UNKNOWN,      ZLK_UNKNOWN,      ZLK_UNKNOWN,
		ZLK_UNKNOWN,    ZLK_UNKNOWN,      ZLK_UNKNOWN,     ZLK_UNKNOWN,     ZLK_UNKNOWN,      ZLK_UNKNOWN,    ZLK_UNKNOWN,        ZLK_UNKNOWN,      ZLK_UNKNOWN,      ZLK_UNKNOWN,
		ZLK_UNKNOWN,    ZLK_UNKNOWN,      ZLK_UNKNOWN,     ZLK_UNKNOWN,     ZLK_UNKNOWN,      ZLK_UNKNOWN,    ZLK_UNKNOWN,        ZLK_UNKNOWN,      ZLK_UNKNOWN,      ZLK_LEFTBRACKET,
		ZLK_BACKSLASH,  ZLK_RIGHTBRACKET, ZLK_APOSTROPHE,  ZLK_UNKNOWN,     ZLK_UNKNOWN,      ZLK_UNKNOWN,    ZLK_NONUSBACKSLASH, ZLK_UNKNOWN,      ZLK_UNKNOWN,      ZLK_UNKNOWN,
		ZLK_UNKNOWN,    ZLK_UNKNOWN,      ZLK_UNKNOWN,     ZLK_UNKNOWN,     ZLK_UNKNOWN,      ZLK_UNKNOWN,    ZLK_UNKNOWN,        ZLK_UNKNOWN,      ZLK_UNKNOWN,      ZLK_UNKNOWN,
		ZLK_UNKNOWN,    ZLK_UNKNOWN,      ZLK_UNKNOWN,     ZLK_UNKNOWN,     ZLK_UNKNOWN,      ZLK_UNKNOWN,    ZLK_SYSREQ,         ZLK_CRSEL,        ZLK_EXSEL,        ZLK_UNKNOWN,
		ZLK_AUDIOPLAY,  ZLK_UNKNOWN,      ZLK_UNKNOWN,     ZLK_UNKNOWN,     ZLK_CLEAR,        ZLK_UNKNOWN
	};
	static unsigned short keymodstatus = 0;
	ZL_Key key = win_zlkey_table[VirtualKey & 255];
	ZL_Event e;
	e.type = (down ? ZL_EVENT_KEYDOWN : ZL_EVENT_KEYUP);
	e.key.is_down = down;
	e.key.key = key;
	if      (key == ZLK_LSHIFT)   (down ? keymodstatus |= ZLKMOD_LSHIFT : keymodstatus &= ~ZLKMOD_LSHIFT);
	else if (key == ZLK_RSHIFT)   (down ? keymodstatus |= ZLKMOD_RSHIFT : keymodstatus &= ~ZLKMOD_RSHIFT);
	else if (key == ZLK_LCTRL)    (down ? keymodstatus |= ZLKMOD_LCTRL  : keymodstatus &= ~ZLKMOD_LCTRL );
	else if (key == ZLK_RCTRL)    (down ? keymodstatus |= ZLKMOD_RCTRL  : keymodstatus &= ~ZLKMOD_RCTRL );
	else if (key == ZLK_LALT)     (down ? keymodstatus |= ZLKMOD_LALT   : keymodstatus &= ~ZLKMOD_LALT  );
	else if (key == ZLK_RALT)     (down ? keymodstatus |= ZLKMOD_RALT   : keymodstatus &= ~ZLKMOD_RALT  );
	else if (key == ZLK_RSHIFT)   (down ? keymodstatus |= ZLKMOD_NUM    : keymodstatus &= ~ZLKMOD_NUM   );
	else if (key == ZLK_CAPSLOCK) (down ? keymodstatus |= ZLKMOD_CAPS   : keymodstatus &= ~ZLKMOD_CAPS  );
	else if (key == ZLK_MODE)     (down ? keymodstatus |= ZLKMOD_MODE   : keymodstatus &= ~ZLKMOD_MODE  );
	e.key.mod = keymodstatus;
	ZL_Display_Process_Event(e);
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

namespace ZLGLSL
{
	GLuint _COLOR_PROGRAM = 10;
	GLuint _TEXTURE_PROGRAM = 20;
	GLint UNI_MVP = 1;
	GLint ATTR_POSITION = 2;
	GLint ATTR_COLOR = 3;
	GLint ATTR_TEXCOORD = 4;

	extern GLfloat *mvp_matrix_;
	XMMATRIX mvp_orientation(0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f); // 90-degree Z-rotation

	void MatrixApply()
	{
		//m_d3dContext->UpdateSubresource(ZLD3D::ConstantBuffers[0], 0, NULL, mvp_matrix_, 0, 0);
		XMMATRIX m = (XMLoadFloat4x4((XMFLOAT4X4*)mvp_matrix_) *= mvp_orientation);
		m_d3dContext->UpdateSubresource(ZLD3D::ConstantBuffers[0], 0, NULL, &m, 0, 0);
	}

	void _COLOR_PROGRAM_ACTIVATE()
	{
		ActiveProgram = COLOR;
		m_d3dContext->IASetInputLayout(ZLD3D::COLOR_INPUTLAYOUT);
		m_d3dContext->VSSetShader(ZLD3D::COLOR_VERTEXSHADER, nullptr, 0);
		m_d3dContext->PSSetShader(ZLD3D::COLOR_PIXELSHADER, nullptr, 0);
	}

	void _TEXTURE_PROGRAM_ACTIVATE()
	{
		ActiveProgram = TEXTURE;
		m_d3dContext->IASetInputLayout(ZLD3D::TEXTURE_INPUTLAYOUT);
		m_d3dContext->VSSetShader(ZLD3D::TEXTURE_VERTEXSHADER, nullptr, 0);
		m_d3dContext->PSSetShader(ZLD3D::TEXTURE_PIXELSHADER, nullptr, 0);
	}
}

float gl_ClearColor[4] = { 0, 0, 0, 1 };
void glClear(int mask)
{
	m_d3dContext->ClearRenderTargetView(ZLD3D::RenderTargetView, gl_ClearColor);
}

void glClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
	gl_ClearColor[0] = r; gl_ClearColor[1] = g; gl_ClearColor[2] = b; gl_ClearColor[3] = a;
}

const GLubyte* glGetString(GLenum name)
{
	return (const GLubyte*)"";
}

void glGetIntegerv(GLenum pname, GLint* params)
{
	*params = 2048;
}

void glScissor(int x, int y, int width, int height)
{
	//D3D11_RECT scissorrect = { x, y, x + width, y + height }; //no rotation
	D3D11_RECT scissorrect = { y, x, y + height, x + width }; //90 deg rotation
	m_d3dContext->RSSetScissorRects(1, &scissorrect);
}

void glEnable(GLenum what)
{
	if (what != GL_SCISSOR_TEST) return;
	ZLD3D::RasterStateActive = &ZLD3D::RasterStateScissor;
	m_d3dContext->RSSetState(*ZLD3D::RasterStateActive);
}

void glDisable(GLenum what)
{
	if (what != GL_SCISSOR_TEST) return;
	ZLD3D::RasterStateActive = &ZLD3D::RasterStateFull;
	m_d3dContext->RSSetState(*ZLD3D::RasterStateActive);
}

void glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
}

void glDepthFunc(GLenum which)
{
}

void glDepthMask(GLenum which)
{
}

void glBlendFunc(GLenum sfactor, GLenum dfactor)
{
}

void glLineWidth(GLfloat width)
{
}

void glVertexAttrib4f(GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	XMFLOAT4 c(x, y, z, w);
	m_d3dContext->UpdateSubresource(ZLD3D::ConstantBuffers[1], 0, NULL, &c, 0, 0);
}

void glVertexAttrib4fv(GLuint indx, const GLfloat* values)
{
	m_d3dContext->UpdateSubresource(ZLD3D::ConstantBuffers[1], 0, NULL, values, 0, 0);
}

void glVertexAttribPointer(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr)
{
	ZLD3D::INDEX_BUFFER_APPLIED = 0;
	if (indx == ZLGLSL::ATTR_POSITION)      { ZLD3D::ATTR_POSITION_ptr = ptr; ZLD3D::ATTR_POSITION_size = size; }
	else if (indx == ZLGLSL::ATTR_COLOR)    { ZLD3D::ATTR_COLOR_ptr    = ptr; ZLD3D::ATTR_COLOR_size    = size; }
	else if (indx == ZLGLSL::ATTR_TEXCOORD) { ZLD3D::ATTR_TEXCOORD_ptr = ptr; ZLD3D::ATTR_TEXCOORD_size = size; }
}

void glEnableVertexAttribArray(GLuint index)
{
	if (index == ZLGLSL::ATTR_COLOR)
	{
		const unsigned char use_global_color[16] = { ZLD3D::USE_GLOBAL_COLOR = 0 };
		m_d3dContext->UpdateSubresource(ZLD3D::ConstantBuffers[2], 0, NULL, use_global_color, 0, 0);
	}
}

void glDisableVertexAttribArray(GLuint index)
{
	if (index == ZLGLSL::ATTR_COLOR)
	{
		const unsigned char use_global_color[16] = { ZLD3D::USE_GLOBAL_COLOR = 1 };
		m_d3dContext->UpdateSubresource(ZLD3D::ConstantBuffers[2], 0, NULL, use_global_color, 0, 0);
	}
}

void glDrawPrepare(GLint first, GLsizei count)
{
	const UINT offset = 0;

	if (count > ZLD3D::VertexBufferSize)
	{
		ZLD3D::VertexBufferSize = count;
		ZL_LOG1("WP8", "Increase Draw Vertex Buffer Size: %d", count);

		CD3D11_BUFFER_DESC vertexPosBufferDescGPU(sizeof(XMFLOAT3)*count, D3D11_BIND_VERTEX_BUFFER);
		if (ZLD3D::VertexPosBuffer) { ZLD3D::VertexPosBuffer->Release(); ZLD3D::VertexPosBuffer = NULL; }
		ThrowIfFailed(m_d3dDevice->CreateBuffer(&vertexPosBufferDescGPU, NULL, &ZLD3D::VertexPosBuffer));

		CD3D11_BUFFER_DESC vertexColorBufferDescGPU(sizeof(XMFLOAT4)*count, D3D11_BIND_VERTEX_BUFFER);
		if (ZLD3D::VertexColorBuffer) { ZLD3D::VertexColorBuffer->Release(); ZLD3D::VertexColorBuffer = NULL; }
		ThrowIfFailed(m_d3dDevice->CreateBuffer(&vertexColorBufferDescGPU, NULL, &ZLD3D::VertexColorBuffer));
		if (ZLD3D::USE_GLOBAL_COLOR) //set buffer to null as it is unused for now
			m_d3dContext->IASetVertexBuffers(1, 1, (ID3D11Buffer**)&offset, &offset, &offset);

		CD3D11_BUFFER_DESC vertexTexCoordBufferDescGPU(sizeof(XMFLOAT2)*count, D3D11_BIND_VERTEX_BUFFER);
		if (ZLD3D::VertexTexCoordBuffer) { ZLD3D::VertexTexCoordBuffer->Release(); ZLD3D::VertexTexCoordBuffer = NULL; }
		ThrowIfFailed(m_d3dDevice->CreateBuffer(&vertexTexCoordBufferDescGPU, NULL, &ZLD3D::VertexTexCoordBuffer));
		if (ZLGLSL::ActiveProgram != ZLGLSL::TEXTURE || !ZLD3D::TextureBound) //set buffer to null as it is unused for now
			m_d3dContext->IASetVertexBuffers(2, 1, (ID3D11Buffer**)&offset, &offset, &offset);
	}

	const UINT stridePos = sizeof(float)*ZLD3D::ATTR_POSITION_size;
	D3D11_BOX boxPos = { 0, 0, 0, stridePos*count, 1, 1 };
	m_d3dContext->UpdateSubresource(ZLD3D::VertexPosBuffer, 0, &boxPos, (char*)ZLD3D::ATTR_POSITION_ptr+(stridePos*(UINT)first), 0, 0);
	m_d3dContext->IASetVertexBuffers(0, 1, &ZLD3D::VertexPosBuffer, &stridePos, &offset);

	if (!ZLD3D::USE_GLOBAL_COLOR)
	{
		const UINT strideColor = sizeof(float)*ZLD3D::ATTR_COLOR_size;
		D3D11_BOX boxColor = { 0, 0, 0, strideColor*count, 1, 1 };
		m_d3dContext->UpdateSubresource(ZLD3D::VertexColorBuffer, 0, &boxColor, (char*)ZLD3D::ATTR_COLOR_ptr+(strideColor*(UINT)first), 0, 0);
		m_d3dContext->IASetVertexBuffers(1, 1, &ZLD3D::VertexColorBuffer, &strideColor, &offset);
	}

	if (ZLGLSL::ActiveProgram == ZLGLSL::TEXTURE && ZLD3D::TextureBound)
	{
		const UINT strideTexCoord = sizeof(float)*ZLD3D::ATTR_TEXCOORD_size;
		D3D11_BOX boxTexCoord = { 0, 0, 0, strideTexCoord*count, 1, 1 };
		m_d3dContext->UpdateSubresource(ZLD3D::VertexTexCoordBuffer, 0, &boxTexCoord, (char*)ZLD3D::ATTR_TEXCOORD_ptr+(strideTexCoord*(UINT)first), 0, 0);
		m_d3dContext->IASetVertexBuffers(2, 1, &ZLD3D::VertexTexCoordBuffer, &strideTexCoord, &offset);

		m_d3dContext->PSSetShaderResources(0, 1, &ZLD3D::TextureBound->shaderresource);
		m_d3dContext->PSSetSamplers(0, 1, ZLD3D::TextureBound->samplerstate);
	}
}

void glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
	assert(mode != GL_TRIANGLE_FAN);

	glDrawPrepare(first, count);

	m_d3dContext->IASetPrimitiveTopology(
			(mode == GL_TRIANGLES      ? D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST :
			(mode == GL_TRIANGLE_STRIP ? D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP :
			(mode == GL_POINTS         ? D3D11_PRIMITIVE_TOPOLOGY_POINTLIST :
			(mode == GL_LINES          ? D3D11_PRIMITIVE_TOPOLOGY_LINELIST :
			(mode == GL_LINE_LOOP      ? D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ :
			(mode == GL_LINE_STRIP     ? D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP :
			(mode == GL_TRIANGLE_FAN   ? D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST :
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST))))))));

	m_d3dContext->Draw(count, 0);
}

void glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices)
{
	assert(type == GL_UNSIGNED_SHORT);

	GLsizei max = 0, countdown = count;
	for (GLushort* p = (GLushort*)indices; countdown--; p++) if (*p > max) max = *p;
	if (max+1 > ZLD3D::INDEX_BUFFER_APPLIED)
	{
		ZLD3D::INDEX_BUFFER_APPLIED = max+1;
		glDrawPrepare(0, max+1);
	}

	if (mode == GL_TRIANGLE_FAN) count = (count-2)*3;

	if (count > ZLD3D::IndexBufferSize)
	{
		ZLD3D::IndexBufferSize = count;
		ZL_LOG1("WP8", "Increase Draw Index Buffer Size: %d", count);

		CD3D11_BUFFER_DESC indexBufferDescGPU(sizeof(GLushort)*count, D3D11_BIND_INDEX_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
		if (ZLD3D::IndexBuffer) { ZLD3D::IndexBuffer->Release(); ZLD3D::IndexBuffer = NULL; }
		ThrowIfFailed(m_d3dDevice->CreateBuffer(&indexBufferDescGPU, NULL, &ZLD3D::IndexBuffer));
	}

	D3D11_MAPPED_SUBRESOURCE map;
	m_d3dContext->Map(ZLD3D::IndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
	if (mode == GL_TRIANGLE_FAN)
	{
		GLushort *in = (GLushort*)indices, *out = (GLushort*)map.pData;
		for (GLsizei i = 0; i < count; i+=3)
		{
			out[i+0] = in[0];
			out[i+1] = in[1+(i/3)];
			out[i+2] = in[2+(i/3)];
		}
	}
	else
	{
		memcpy(map.pData, indices, sizeof(GLushort)*count);
		////UpdateSubresource only works with D3D11_USAGE_DEFAULT not with D3D11_USAGE_DYNAMIC
		//D3D11_BOX box = { 0, 0, 0, sizeof(GLushort)*count, 1, 1 };
		//m_d3dContext->UpdateSubresource(ZLD3D::IndexBuffer, 0, &box, indices, 0, 0);
	}
	m_d3dContext->Unmap(ZLD3D::IndexBuffer, 0);
	m_d3dContext->IASetIndexBuffer(ZLD3D::IndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	m_d3dContext->IASetPrimitiveTopology(
			(mode == GL_TRIANGLES      ? D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST :
			(mode == GL_TRIANGLE_STRIP ? D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP :
			(mode == GL_TRIANGLE_FAN   ? D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST :
			(mode == GL_POINTS         ? D3D11_PRIMITIVE_TOPOLOGY_POINTLIST :
			(mode == GL_LINES          ? D3D11_PRIMITIVE_TOPOLOGY_LINELIST :
			(mode == GL_LINE_LOOP      ? D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ :
			(mode == GL_LINE_STRIP     ? D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP :
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST))))))));

	m_d3dContext->DrawIndexed(count, 0, 0);
}

void glBindTexture(GLenum target, GLuint texture)
{
	if (!texture || texture > ZLD3D::Textures.size()) return;
	ZLD3D::TextureBound = ZLD3D::Textures[texture-1];
}

void glGenTextures(GLsizei n, GLuint* textures)
{
	for (GLsizei i = 0; i < n; i++)
	{
		unsigned int t = 0; while (t < ZLD3D::Textures.size() && ZLD3D::Textures[t]) t++;
		if (t == ZLD3D::Textures.size()) ZLD3D::Textures.push_back(new ZLD3D::Texture());
		else ZLD3D::Textures[t] = new ZLD3D::Texture();
		ZLD3D::Textures[t]->shaderresource = nullptr;
		ZLD3D::Textures[t]->samplerstate = &ZLD3D::SamplerStateClamp;
		textures[i] = 1+t;
	}
}

void glTexParameteri(GLenum target, GLenum pname, GLint param)
{
	if (target != GL_TEXTURE_2D || !ZLD3D::TextureBound) return;
	if (pname == GL_TEXTURE_WRAP_S || pname == GL_TEXTURE_WRAP_T)
		ZLD3D::TextureBound->samplerstate = (param == GL_CLAMP_TO_EDGE ? &ZLD3D::SamplerStateClamp : &ZLD3D::SamplerStateWrap);
}

void glPixelStorei(GLenum pname, GLint param)
{
}

void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
	if (format != GL_RGBA) ThrowIfFailed(E_NOTIMPL);
	if (!ZLD3D::TextureBound) return;
	CD3D11_TEXTURE2D_DESC desc(DXGI_FORMAT_R8G8B8A8_UNORM, width, height);
	#if 0
	//if using mipmaps
	desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
	desc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
	#else
	desc.MipLevels = 1;
	#endif
	ID3D11Texture2D* texture2D;
	ThrowIfFailed(m_d3dDevice->CreateTexture2D( &desc, NULL, &texture2D));
	CD3D11_SHADER_RESOURCE_VIEW_DESC viewDesc(D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM);
	ThrowIfFailed(m_d3dDevice->CreateShaderResourceView(texture2D, &viewDesc, &ZLD3D::TextureBound->shaderresource));
	texture2D->Release();
	if (pixels) glTexSubImage2D(target, level, 0, 0, width, height, format, type, pixels);
}

void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels)
{
	if (format != GL_RGBA) ThrowIfFailed(E_NOTIMPL);
	if (!ZLD3D::TextureBound) return;
	D3D11_BOX box = { xoffset, yoffset, 0, xoffset+width, yoffset+height, 1 };
	ID3D11Resource* texture2D;
	ZLD3D::TextureBound->shaderresource->GetResource(&texture2D);
	m_d3dContext->UpdateSubresource(texture2D, 0, &box, pixels, width*4, 0);
	#if 0
	//if using mipmaps
	m_d3dContext->GenerateMips(textureBound);
	#endif
}

void glDeleteTextures(GLsizei n, const GLuint* textures)
{
	for (GLsizei i = 0; i < n; i++)
	{
		if (!textures[i] || textures[i] > ZLD3D::Textures.size() || !ZLD3D::Textures[textures[i]-1]) continue;
		if (ZLD3D::Textures[textures[i]-1]->shaderresource) ZLD3D::Textures[textures[i]-1]->shaderresource->Release();
		if (ZLD3D::TextureBound == ZLD3D::Textures[textures[i]-1]) ZLD3D::TextureBound = NULL;
		delete ZLD3D::Textures[textures[i]-1];
		ZLD3D::Textures[textures[i]-1] = NULL;
	}
}

//settings
void ZL_SettingsInit(const char* FallbackConfigFilePrefix)
{
	auto folder = ApplicationData::Current->LocalFolder->Path;
	char fname[1024];
	fname[WideCharToMultiByte(CP_UTF8, 0, folder->Begin(), folder->Length(), fname, 1023, NULL, NULL)] = 0;

	jsonConfigFile = ((ZL_String(fname) += FallbackConfigFilePrefix) += ".cfg");
	if (ZL_File::Exists(jsonConfigFile)) jsonConfig = ZL_Json(ZL_File(jsonConfigFile));
}

const ZL_String ZL_SettingsGet(const char* Key)
{
	ZL_Json field = jsonConfig.GetByKey(Key);
	return (!field ? ZL_String::EmptyString : field.GetString());
}

void ZL_SettingsSet(const char* Key, const ZL_String& Value)
{
	jsonConfig[Key].SetString(Value);
}

void ZL_SettingsDel(const char* Key)
{
	jsonConfig.Erase(Key);
}

bool ZL_SettingsHas(const char* Key)
{
	return jsonConfig.HasKey(Key);
}

void ZL_SettingsSynchronize()
{
	ZL_File(jsonConfigFile, "w").SetContents(jsonConfig);
}

//application
void ZL_StartTicks()
{
	LARGE_INTEGER frequency;
	if (!QueryPerformanceFrequency(&frequency)) throw ref new Platform::FailureException();
	if (!QueryPerformanceCounter(&ticks_startTime)) throw ref new Platform::FailureException();
	ticks_frequencyMs = (frequency.QuadPart / 1000);
}

ticks_t ZL_GetTicks()
{
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);
	return (ticks_t)((currentTime.QuadPart - ticks_startTime.QuadPart) / ticks_frequencyMs);
}

void ZL_Delay(ticks_t ms)
{
	Concurrency::wait(ms);
}

//windows phone specific
void ZL_SoftKeyboardToggle()
{
	m_window->IsKeyboardInputEnabled ^= 1;
}

void ZL_SoftKeyboardShow()
{
	m_window->IsKeyboardInputEnabled = true;
}

void ZL_SoftKeyboardHide()
{
	m_window->IsKeyboardInputEnabled = false;
}

bool ZL_SoftKeyboardIsShown()
{
	return m_window->IsKeyboardInputEnabled;
}

//thread
int ZL_CreateThread(void *(*start_routine) (void *p), void *arg)
{
	auto workItemDelegate = [start_routine, arg](IAsyncAction^ workItem)
	{
		start_routine(arg);
	};
	auto workItemHandler = ref new Windows::System::Threading::WorkItemHandler(workItemDelegate);
	Windows::Foundation::IAsyncAction^ a = Windows::System::Threading::ThreadPool::RunAsync(workItemHandler);
	return a->Id;
}

void ZL_WaitThread(int hthread, int *pstatus)
{
}

//display
bool ZL_CreateWindow(const char* windowtitle, int width, int height, int displayflags)
{
	pZL_WindowFlags = &WP_WindowFlags;
	return true;
}

void ZL_GetWindowSize(int *w, int *h)
{
	//no rotation
	//*w = (int)m_renderTargetSize.Width;
	//*h = (int)m_renderTargetSize.Height;

	//90 deg rotation
	*w = (int)m_renderTargetSize.Height;
	*h = (int)m_renderTargetSize.Width;
}

//Misc
void ZL_OpenExternalUrl(const char* url)
{
	wchar_t wcUrl[1024];
	wcUrl[MultiByteToWideChar(CP_UTF8, 0, url, strlen(url), wcUrl, 1024)] = 0;
	Launcher::LaunchUriAsync(ref new Uri(ref new Platform::String(wcUrl)));
}

//Joystick
int ZL_NumJoysticks()
{
	return 0;
}

ZL_JoystickData* ZL_JoystickHandleOpen(int index)
{
	return NULL;
}

void ZL_JoystickHandleClose(ZL_JoystickData* joystick)
{
}

//Audio
bool ZL_AudioOpen(unsigned int /*buffer_length*/)
{
	static bool done; if (done) return true; done = true; // cannot restart on wp
	auto workItemDelegate = [](IAsyncAction^ workItem)
	{
		struct : public IXAudio2VoiceCallback
		{
			BYTE *pAudioBuffers, *pAudioBuffersCur;
			IXAudio2SourceVoice* pSourceVoice;
			XAUDIO2_BUFFER sAudioBuffer;
			STDMETHOD_(void, OnVoiceProcessingPassStart) (UINT32 BytesRequired) {}
			STDMETHOD_(void, OnVoiceProcessingPassEnd) () {}
			STDMETHOD_(void, OnStreamEnd) () {}
			STDMETHOD_(void, OnBufferStart) (void *pBufferContext) {}
			STDMETHOD_(void, OnLoopEnd) (void *pBufferContext) {}
			STDMETHOD_(void, OnVoiceError) (void *pBufferContext, HRESULT Error) {};
			STDMETHOD_(void, OnBufferEnd) (void *pBufferContext)
			{
				pSourceVoice->SubmitSourceBuffer(&sAudioBuffer); // send buffer to queue
				pAudioBuffersCur = pAudioBuffers + (pAudioBuffersCur == pAudioBuffers ? sAudioBuffer.AudioBytes : 0);
				ZL_PlatformAudioMix((short*)pAudioBuffersCur, (unsigned int)sAudioBuffer.AudioBytes);
				sAudioBuffer.pAudioData = pAudioBuffersCur;
			}
		} v;

		IXAudio2* pXAudio2 = NULL;
		IXAudio2MasteringVoice* pMasterVoice = NULL;
		WAVEFORMATEX wfx;
		memset(&wfx, 0, sizeof(wfx));
		wfx.cbSize = sizeof(wfx);
		wfx.wFormatTag = WAVE_FORMAT_PCM;
		wfx.nChannels = 2;											// stereo
		wfx.nSamplesPerSec = 44100;									// sample rate
		wfx.wBitsPerSample = 16;									// 16-bit
		wfx.nBlockAlign = wfx.wBitsPerSample * wfx.nChannels / 8;	// bytes per sample
		wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

		ThrowIfFailed(XAudio2Create(&pXAudio2));
		ThrowIfFailed(pXAudio2->CreateMasteringVoice(&pMasterVoice, wfx.nChannels, wfx.nSamplesPerSec, 0, nullptr, nullptr, AudioCategory_GameEffects));
		ThrowIfFailed(pXAudio2->CreateSourceVoice(&v.pSourceVoice, &wfx, 0, XAUDIO2_DEFAULT_FREQ_RATIO, &v, NULL, NULL));
		ThrowIfFailed(pXAudio2->StartEngine());
		ThrowIfFailed(v.pSourceVoice->Start(0));

		int cbLoopLen = 40*wfx.nChannels*wfx.wBitsPerSample/8;
		v.pAudioBuffersCur = v.pAudioBuffers = (BYTE *)malloc(cbLoopLen);
		ZeroMemory(v.pAudioBuffers, cbLoopLen);
		ZeroMemory(&v.sAudioBuffer, sizeof(v.sAudioBuffer));
		v.sAudioBuffer.AudioBytes = cbLoopLen/2;
		v.sAudioBuffer.pAudioData = v.pAudioBuffersCur;
		v.pSourceVoice->SubmitSourceBuffer(&v.sAudioBuffer);
		while (true)
		{
			Concurrency::wait(100);
		}

		if (v.pSourceVoice) { v.pSourceVoice->Stop(0); v.pSourceVoice->DestroyVoice(); v.pSourceVoice = NULL; }
		if (pMasterVoice) { pMasterVoice->DestroyVoice(); pMasterVoice = NULL; }
		if (pXAudio2) { pXAudio2->Release(); pXAudio2 = NULL; }
		if (v.pAudioBuffers) { free(v.pAudioBuffers); v.pAudioBuffers = NULL; }
	};
	auto workItemHandler = ref new Windows::System::Threading::WorkItemHandler(workItemDelegate);
	Windows::Foundation::IAsyncAction^ a = Windows::System::Threading::ThreadPool::RunAsync(workItemHandler);
	return true;
}

//TODO CUSTOM SHADER IMPLEMENTATION...
struct ZL_Shader_Impl : ZL_Impl { };
ZL_Shader::ZL_Shader(const char*, const char*, const char*, const char*, int, ...) : impl(NULL) { }
ZL_IMPL_OWNER_DEFAULT_IMPLEMENTATIONS(ZL_Shader)
void ZL_Shader::Activate() { }
void ZL_Shader::SetUniform(scalar, scalar, ...) { }
void ZL_Shader::Deactivate() { }
struct ZL_PostProcess_Impl : ZL_Impl { };
ZL_PostProcess::ZL_PostProcess(const char*, bool, const char*, const char*, int, ...) : impl(NULL) { }
ZL_IMPL_OWNER_DEFAULT_IMPLEMENTATIONS(ZL_PostProcess)
void ZL_PostProcess::Start(bool) { }
void ZL_PostProcess::Apply() { }
void ZL_PostProcess::Apply(scalar, double, ...) { }

#endif
