
#ifndef __XE_GRAPHICS_D3D11_TESTAPP_RENDERER_HPP__
#define __XE_GRAPHICS_D3D11_TESTAPP_RENDERER_HPP__

#include <Windows.h>
#include <cassert>
#include <d3d11.h>
#include <dxgi.h>

namespace TestApp {
    /**
     * @brief A pointer that handles COM objects automatically
     */
    template <class T> class COMPtr {
        // static_assert(std::is_base_of<IUnknown, T>)

    public:
        COMPtr() {
        }

        COMPtr(T *object) : m_obj(object) {
        }

        ~COMPtr() {
            if (m_obj) {
                m_obj->Release();
            }
        }

        T **operator&() {
            return &m_obj;
        }

        T *operator->() {
            assert(m_obj);
            return m_obj;
        }

        const T *operator->() const {
            assert(m_obj);
            return m_obj;
        }

        operator T *() {
            return m_obj;
        }

        operator const T *() const {
            return m_obj;
        }

    private:
        T *m_obj = nullptr;
    };

    class Renderer {
    public:
        Renderer();

        ~Renderer();

        void Initialize(HWND hWnd);

        void Terminate();

        void Render();

    private:
        HWND m_hWnd;
        D3D_FEATURE_LEVEL m_featureLevel;
        D3D_DRIVER_TYPE m_driverType;

        ID3D11Device *m_device;
        ID3D11DeviceContext *m_context;
        IDXGISwapChain *m_swapChain;
        ID3D11Texture2D *m_backBufferTexture;
        ID3D11RenderTargetView *m_backBufferTarget;
    };
} // namespace TestApp

#endif
