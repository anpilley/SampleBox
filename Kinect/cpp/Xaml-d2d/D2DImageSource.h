#pragma once


namespace XamlKinect
{

    ref class D2DImageSource sealed :
        public Windows::UI::Xaml::Media::Imaging::SurfaceImageSource
    {
    public:
        D2DImageSource(
            int pixelWidth,
            int pixelHeight,
            bool isOpaque);

        void BeginDraw(Windows::Foundation::Rect updateRect);
        void BeginDraw()    { BeginDraw(Windows::Foundation::Rect(0, 0, (float)m_width, (float)m_height)); }
        void EndDraw();


        // drawing functions
        void Clear(Windows::UI::Color color);
        void DrawCircle(float x, float y, float radius, Windows::UI::Color color);
        void DrawLine(float x1, float y1, float x2, float y2, Windows::UI::Color color, float thickness);
        void DrawLine(float x1, float y1, float x2, float y2, Windows::UI::Color color)
        {
            DrawLine(x1, y1, x2, y2, color, 1.0f);
        };
        void DrawFilledCircle(float x, float y, float radius, Windows::UI::Color color);
        void DrawFilledRect(float x1, float y1, float x2, float y2, Windows::UI::Color color);

    private:
        void CreateDeviceResources();
        void CreateDeviceIndependentResources();


        int m_width;
        int m_height;

        Microsoft::WRL::ComPtr<ISurfaceImageSourceNative> m_sisNative;
        Microsoft::WRL::ComPtr<ID3D11Device> m_d3dDevice;
        Microsoft::WRL::ComPtr<ID2D1Device> m_d2dDevice;
        Microsoft::WRL::ComPtr<ID2D1DeviceContext> m_d2dContext;
        Microsoft::WRL::ComPtr<ID2D1Factory> m_d2dFactory;
    };

}

