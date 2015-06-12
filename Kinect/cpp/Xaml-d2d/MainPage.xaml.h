//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"

#include "BodyView.xaml.h"

namespace XamlKinect
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public ref class MainPage sealed
    {
    public:
        MainPage();

    protected:
        void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;

    private:
        WindowsPreview::Kinect::KinectSensor^ sensor;

        // shared consts
        static const int cBytesPerPixel = 4;

        // color bits
        void InitColorImage();
        WindowsPreview::Kinect::ColorFrameReader^ colorReader;
        void ColorFrameArrived(WindowsPreview::Kinect::ColorFrameReader ^sender, WindowsPreview::Kinect::ColorFrameArrivedEventArgs ^args);
        unsigned int bytesPerPixel;
        uint8* colorPixels;
        Windows::UI::Xaml::Media::Imaging::WriteableBitmap^ colorBitmap;

        // depth stream.
        void InitDepthImage();
        void ConvertDepthData(uint16 minDepth, uint16 maxDepth);
        void RenderDepthPixels();
        void DepthFrameArrived(WindowsPreview::Kinect::DepthFrameReader ^sender, WindowsPreview::Kinect::DepthFrameArrivedEventArgs ^args);
        static const int MaxDepthToBytes = 8000 / 256;
        WindowsPreview::Kinect::DepthFrameReader^ depthReader;
        Windows::UI::Xaml::Media::Imaging::WriteableBitmap^ depthBitmap;
        Platform::Array<uint16>^ depthFrameData;
        uint8* depthPixels;
        uint32 depthPixelsSize;
        uint8* pixelData;

        // IR
        void InitIRImage();
        static const float InfraredSourceValueMaximum;
        static const float InfraredOutputValueMinimum;
        static const float InfraredOutputValueMaximum;
        static const float InfraredSceneValueAverage;
        static const float InfraredSceneStandardDeviations;

        WindowsPreview::Kinect::InfraredFrameReader^ infraredFrameReader;
        Windows::UI::Xaml::Media::Imaging::WriteableBitmap^ irBitmap;
        Platform::Array<uint16>^ infraredFrameData;
        unsigned int infraredFrameDataSize;
        uint8* infraredPixels;
        unsigned int infraredPixelsSize;
        void ConvertInfraredData();
        void RenderInfraredPixels();



        void KinectAvailableChanged(
            WindowsPreview::Kinect::KinectSensor ^sender,
            WindowsPreview::Kinect::IsAvailableChangedEventArgs ^args);
        void Page_Unloaded(
            Platform::Object^ sender,
            Windows::UI::Xaml::RoutedEventArgs^ e);




        void IRFrameArrived(WindowsPreview::Kinect::InfraredFrameReader ^sender, WindowsPreview::Kinect::InfraredFrameArrivedEventArgs ^args);
    };
}
