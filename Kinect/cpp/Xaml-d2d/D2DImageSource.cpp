#include "pch.h"
#include "D2DImageSource.h"
#include "DirectXHelpers.h"

using namespace XamlKinect;

using namespace Platform;
using namespace Microsoft::WRL;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Media::Imaging;

using namespace Windows::Foundation;

D2DImageSource::D2DImageSource(
    int pixelWidth,
    int pixelHeight,
    bool isOpaque) 
    :SurfaceImageSource(pixelWidth, pixelHeight, isOpaque)
{
    m_width = pixelWidth;
    m_height = pixelHeight;
    CreateDeviceIndependentResources();
    CreateDeviceResources();
};


void D2DImageSource::CreateDeviceResources()
{
    HRESULT hr = S_OK;

    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
   // creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    const D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1,
    };

    DX::ThrowIfFailed(D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        creationFlags,
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        &m_d3dDevice,
        nullptr,
        nullptr));

    ComPtr<IDXGIDevice> dxgiDevice;
    DX::ThrowIfFailed(m_d3dDevice.As(&dxgiDevice));
    DX::ThrowIfFailed(D2D1CreateDevice(
        dxgiDevice.Get(),
        nullptr,
        &m_d2dDevice));

    DX::ThrowIfFailed(m_d2dDevice->CreateDeviceContext(
        D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
        &m_d2dContext));

    DX::ThrowIfFailed(m_sisNative->SetDevice(dxgiDevice.Get()));

    DX::ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, m_d2dFactory.GetAddressOf()));
    
}

void D2DImageSource::CreateDeviceIndependentResources()
{
    reinterpret_cast<IUnknown*>(this)->QueryInterface(
        IID_PPV_ARGS(&m_sisNative));
}

void D2DImageSource::BeginDraw(Rect updateRect)
{
    POINT offset;
    ComPtr<IDXGISurface> surface;

    RECT updateRectNative;
    updateRectNative.left = static_cast<LONG>(updateRect.Left);
    updateRectNative.top = static_cast<LONG>(updateRect.Top);
    updateRectNative.right = static_cast<LONG>(updateRect.Right);
    updateRectNative.bottom = static_cast<LONG>(updateRect.Bottom);

    HRESULT beginDrawHR = m_sisNative->BeginDraw(
        updateRectNative, &surface, &offset);

    if (beginDrawHR == DXGI_ERROR_DEVICE_REMOVED ||
        beginDrawHR == DXGI_ERROR_DEVICE_RESET)
    {
        CreateDeviceResources();
        BeginDraw(updateRect);
    }

    ComPtr<ID2D1Bitmap1> bitmap;
    DX::ThrowIfFailed(m_d2dContext->CreateBitmapFromDxgiSurface(
        surface.Get(),
        nullptr,
        &bitmap));

    m_d2dContext->SetTarget(bitmap.Get());

    m_d2dContext->BeginDraw();
    m_d2dContext->PushAxisAlignedClip(
        D2D1::RectF(
        static_cast<float>(offset.x),
        static_cast<float>(offset.y),
        static_cast<float>(offset.x + updateRect.Width),
        static_cast<float>(offset.y + updateRect.Height)),
        D2D1_ANTIALIAS_MODE_ALIASED);
    m_d2dContext->SetTransform(
        D2D1::Matrix3x2F::Translation(
        static_cast<float>(offset.x),
        static_cast<float>(offset.y)
        ));
}

void D2DImageSource::EndDraw()
{
    m_d2dContext->SetTransform(D2D1::IdentityMatrix());
    m_d2dContext->PopAxisAlignedClip();
    DX::ThrowIfFailed(m_d2dContext->EndDraw());
    m_d2dContext->SetTarget(nullptr);
    DX::ThrowIfFailed(m_sisNative->EndDraw());
}

void D2DImageSource::Clear(Windows::UI::Color color)
{
    m_d2dContext->Clear(DX::ConvertToColorF(color));
}

void D2DImageSource::DrawCircle(float x, float y, float radius, Windows::UI::Color color)
{
    ComPtr<ID2D1SolidColorBrush> brush;
    DX::ThrowIfFailed(
        m_d2dContext->CreateSolidColorBrush(
        DX::ConvertToColorF(color),
        &brush
        ));
    m_d2dContext->DrawEllipse(DX::ConvertToEllipse(x, y, radius), brush.Get());
}

void D2DImageSource::DrawLine(float x1, float y1, float x2, float y2, Windows::UI::Color color, float thickness)
{
    ComPtr<ID2D1SolidColorBrush> brush;
    DX::ThrowIfFailed(
        m_d2dContext->CreateSolidColorBrush(
        DX::ConvertToColorF(color),
        &brush
        ));

    ComPtr<ID2D1StrokeStyle> style;
    
    m_d2dContext->DrawLine(D2D1::Point2F(x1, y1), D2D1::Point2F(x2, y2), brush.Get(), thickness);
}

void D2DImageSource::DrawFilledCircle(float x, float y, float radius, Windows::UI::Color color)
{
    ComPtr<ID2D1SolidColorBrush> brush;
    DX::ThrowIfFailed(
        m_d2dContext->CreateSolidColorBrush(
        DX::ConvertToColorF(color),
        &brush
        ));

    m_d2dContext->FillEllipse(DX::ConvertToEllipse(x, y, radius), brush.Get() );
}


void D2DImageSource::DrawFilledRect(float x1, float y1, float x2, float y2, Windows::UI::Color color)
{
    // Create a solid color D2D brush.
    ComPtr<ID2D1SolidColorBrush> brush;
    DX::ThrowIfFailed(
        m_d2dContext->CreateSolidColorBrush(
        DX::ConvertToColorF(color),
        &brush
        )
        );

    // Draw a filled rectangle.
    m_d2dContext->FillRectangle(D2D1::RectF(x1, y1, x2, y2), brush.Get());
}