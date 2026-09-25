
#include "Renderer.h"

#include <array>
#include <cassert>
#include <stdexcept>

namespace TestApp {
    class ComResultChecker {
    public:
        ComResultChecker() {
        }

        ComResultChecker(HRESULT hr) {
            if (FAILED(hr)) {
                throw std::runtime_error("There is a COM error!");
            }
        }

        bool Succeeded() const {
            if (SUCCEEDED(m_hr)) {
                return true;
            } else {
                return false;
            }
        }

        bool Failed() const {
            if (FAILED(m_hr)) {
                return true;
            } else {
                return false;
            }
        }

        operator HRESULT() const {
            return m_hr;
        }

    private:
        HRESULT m_hr = 0;
    };

    // tipos de controladores disponibles
    static std::array<D3D_DRIVER_TYPE, 3> g_driverTypes = {
        D3D_DRIVER_TYPE_HARDWARE, // aceleracion por hardware
        D3D_DRIVER_TYPE_WARP,     // aceleracion por software optimizada
        D3D_DRIVER_TYPE_SOFTWARE  // aceleracion por software
    };

    static std::array<D3D_FEATURE_LEVEL, 3> g_featureLevels = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

    Renderer::Renderer() {
    }

    Renderer::~Renderer() {
        this->Terminate();
    }

    void Renderer::Initialize(HWND hWnd) {
        m_hWnd = hWnd;

        // determinar dimensiones de la pantalla
        RECT rc = {};

        ::GetClientRect(hWnd, &rc);

        const int width = rc.right - rc.left;
        const int height = rc.bottom - rc.top;

        // configurar el swapchain
        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};

        swapChainDesc.BufferCount = 1;
        swapChainDesc.BufferDesc.Width = width;
        swapChainDesc.BufferDesc.Height = height;
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.OutputWindow = hWnd;
        swapChainDesc.Windowed = true;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;

        // crear el dispositivo
        unsigned creationFlags = D3D11_CREATE_DEVICE_DEBUG;
        ComResultChecker resultChecker = 0;

        for (auto driverType : g_driverTypes) {
            resultChecker = D3D11CreateDeviceAndSwapChain(
                nullptr,
                driverType,
                0,
                creationFlags,
                g_featureLevels.data(),
                static_cast<UINT>(g_featureLevels.size()),
                D3D11_SDK_VERSION,
                &swapChainDesc,
                &m_swapChain,
                &m_device,
                &m_featureLevel,
                &m_context
            );

            if (resultChecker.Succeeded()) {
                m_driverType = driverType;
                break;
            }
        }

        // obtener un render target
        resultChecker = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&m_backBufferTexture);
        resultChecker = m_device->CreateRenderTargetView(m_backBufferTexture, nullptr, &m_backBufferTarget);

        if (m_backBufferTexture) {
            m_backBufferTexture->Release();
        }

        m_context->OMSetRenderTargets(1, &m_backBufferTarget, nullptr);

        // establecer el viewport
        D3D11_VIEWPORT viewport;
        viewport.Width = static_cast<float>(width);
        viewport.Height = static_cast<float>(height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;

        m_context->RSSetViewports(1, &viewport);
    }

    void Renderer::Terminate() {
        if (m_backBufferTarget)
            m_backBufferTarget->Release();
        if (m_backBufferTexture)
            m_backBufferTexture->Release();
        if (m_context)
            m_context->Release();
        if (m_swapChain)
            m_swapChain->Release();
        if (m_device)
            m_device->Release();
    }

    void Renderer::Render() {
        float clearColor[] = {0.2f, 0.2f, 0.8f, 1.0f};

        m_context->ClearRenderTargetView(m_backBufferTarget, clearColor);

        m_swapChain->Present(0, 0);
    }
} // namespace TestApp
