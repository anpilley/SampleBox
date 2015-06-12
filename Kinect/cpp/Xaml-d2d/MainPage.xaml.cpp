//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

using namespace XamlKinect;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Media::Imaging;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::Storage::Streams;

using namespace WindowsPreview::Kinect;

using namespace Microsoft::WRL;

const float MainPage::InfraredSourceValueMaximum = (float)UINT16_MAX;
const float MainPage::InfraredOutputValueMinimum = 0.01f;
const float MainPage::InfraredOutputValueMaximum = 1.0f;
const float MainPage::InfraredSceneValueAverage = 0.08f;
const float MainPage::InfraredSceneStandardDeviations = 3.0f;

MainPage::MainPage()
{
    InitializeComponent();
}

void MainPage::OnNavigatedTo(NavigationEventArgs^ e)
{
    sensor = KinectSensor::GetDefault();

    InitColorImage();
    InitDepthImage();
    InitIRImage();
    this->bodyView->InitBodyView(sensor);

    this->sensor->IsAvailableChanged +=
        ref new TypedEventHandler<KinectSensor ^, IsAvailableChangedEventArgs ^>(
        this,
        &XamlKinect::MainPage::KinectAvailableChanged);

    sensor->Open();

    this->DataContext = this;
    this->colorImage->Source = this->colorBitmap;

}

void MainPage::InitIRImage()
{
    FrameDescription^ frameDescription = this->sensor->InfraredFrameSource->FrameDescription;
    this->infraredFrameReader = this->sensor->InfraredFrameSource->OpenReader();
    this->infraredFrameReader->FrameArrived +=
        ref new TypedEventHandler<InfraredFrameReader ^, InfraredFrameArrivedEventArgs ^>(
        this,
        &XamlKinect::MainPage::IRFrameArrived);

    this->infraredFrameDataSize = frameDescription->Width * frameDescription->Height;
    this->infraredFrameData = ref new Array<uint16>(infraredFrameDataSize);
    this->infraredPixelsSize = frameDescription->Width * frameDescription->Height * this->cBytesPerPixel;

    this->irBitmap = ref new WriteableBitmap(frameDescription->Width, frameDescription->Height);

    ComPtr<Windows::Storage::Streams::IBufferByteAccess> pBufferByteAccess;
    ComPtr<IUnknown> pBuffer((IUnknown*)this->irBitmap->PixelBuffer);
    pBuffer.As(&pBufferByteAccess);
    pBufferByteAccess->Buffer(&this->infraredPixels);

}

void MainPage::InitDepthImage()
{
    FrameDescription^ frameDescription = sensor->DepthFrameSource->FrameDescription;
    this->depthReader = sensor->DepthFrameSource->OpenReader();
    this->depthReader->FrameArrived +=
        ref new TypedEventHandler<DepthFrameReader ^, DepthFrameArrivedEventArgs ^>(
        this,
        &XamlKinect::MainPage::DepthFrameArrived);

    this->depthFrameData = ref new Array<uint16>(frameDescription->Width * frameDescription->Height);
    this->depthBitmap = ref new WriteableBitmap(frameDescription->Width, frameDescription->Height);

    ComPtr<Windows::Storage::Streams::IBufferByteAccess> pBufferByteAccess;
    ComPtr<IUnknown> pBuffer((IUnknown*)this->depthBitmap->PixelBuffer);
    pBuffer.As(&pBufferByteAccess);
    pBufferByteAccess->Buffer(&this->pixelData);
}

void MainPage::InitColorImage()
{
    this->colorReader = sensor->ColorFrameSource->OpenReader();
    this->colorReader->FrameArrived +=
        ref new TypedEventHandler<ColorFrameReader ^, ColorFrameArrivedEventArgs ^>(
        this,
        &XamlKinect::MainPage::ColorFrameArrived);

    FrameDescription^ colorFrameDescription = this->sensor->ColorFrameSource->CreateFrameDescription(ColorImageFormat::Rgba);

    this->bytesPerPixel = colorFrameDescription->BytesPerPixel;
    this->colorPixels = new uint8[colorFrameDescription->Width * colorFrameDescription->Height * this->bytesPerPixel];
    this->colorBitmap = ref new WriteableBitmap(colorFrameDescription->Width, colorFrameDescription->Height);
}


void XamlKinect::MainPage::ColorFrameArrived(WindowsPreview::Kinect::ColorFrameReader ^sender, WindowsPreview::Kinect::ColorFrameArrivedEventArgs ^args)
{
    bool frameProcessed = false;

    ColorFrame^ frame = args->FrameReference->AcquireFrame();

    if (frame != nullptr)
    {
        FrameDescription^ frameDescription = frame->FrameDescription;
        if ((frameDescription->Width == this->colorBitmap->PixelWidth) && (frameDescription->Height == this->colorBitmap->PixelHeight))
        {
            if (frame->RawColorImageFormat == ColorImageFormat::Bgra)
            {
                frame->CopyRawFrameDataToBuffer(this->colorBitmap->PixelBuffer);
            }
            else
            {
                frame->CopyConvertedFrameDataToBuffer(this->colorBitmap->PixelBuffer, ColorImageFormat::Bgra);
            }
            frameProcessed = true;
        }
        delete frame;
    }
    if (frameProcessed)
    {
        this->colorBitmap->Invalidate();
    }

}


void XamlKinect::MainPage::KinectAvailableChanged(WindowsPreview::Kinect::KinectSensor ^sender, WindowsPreview::Kinect::IsAvailableChangedEventArgs ^args)
{
    String^ msg = ref new Platform::String(L"Kinect status: ") + args->IsAvailable.ToString() + L"\n";
    OutputDebugStringW(msg->Data());
}


void XamlKinect::MainPage::Page_Unloaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{

    if (this->colorReader != nullptr)
    {
        delete this->colorReader;
        this->colorReader = nullptr;
    }

    if (this->depthReader != nullptr)
    {
        delete this->depthReader;
        this->depthReader = nullptr;
    }

    if (this->infraredFrameReader != nullptr)
    {
        delete this->infraredFrameReader;
        this->infraredFrameReader = nullptr;
    }

    this->bodyView->Cleanup();

    if (this->sensor != nullptr)
    {
        this->sensor->Close();
        this->sensor = nullptr;
    }

    if (this->colorPixels != nullptr)
    {
        delete this->colorPixels;
        this->colorPixels = nullptr;
    }

    if (this->infraredPixels != nullptr)
    {
        delete this->infraredPixels;
        this->infraredPixels = nullptr;
    }
}


void MainPage::DepthFrameArrived(WindowsPreview::Kinect::DepthFrameReader ^sender, WindowsPreview::Kinect::DepthFrameArrivedEventArgs ^args)
{
    uint16 minDepth = 0;
    uint16 maxDepth = 0;

    bool depthFrameProcessed = false;
    DepthFrame^ frame = args->FrameReference->AcquireFrame();
    if (frame != nullptr)
    {
        FrameDescription^ depthFrameDescription = frame->FrameDescription;

        if (((depthFrameDescription->Width* depthFrameDescription->Height) == this->depthFrameData->Length) &&
            (depthFrameDescription->Width == this->depthBitmap->PixelWidth) &&
            (depthFrameDescription->Height == this->depthBitmap->PixelHeight))
        {
            frame->CopyFrameDataToArray(this->depthFrameData);

            minDepth = frame->DepthMinReliableDistance;
            maxDepth = UINT16_MAX;

            depthFrameProcessed = true;
        }

        delete frame;
    }

    if (depthFrameProcessed)
    {
        ConvertDepthData(minDepth, maxDepth);

        RenderDepthPixels();
    }
}

void MainPage::ConvertDepthData(uint16 minDepth, uint16 maxDepth)
{
    int colorPixelIndex = 0;
    uint16* buf = this->depthFrameData->Data;
    unsigned int len = this->depthFrameData->Length;
    for (unsigned int i = 0; i < len; ++i)
    {
        uint16 depth = buf[i];

        uint8 intensity = (uint8)(depth >= minDepth && depth <= maxDepth ? (depth / MaxDepthToBytes) : 0);
        this->pixelData[colorPixelIndex++] = intensity;
        this->pixelData[colorPixelIndex++] = intensity;
        this->pixelData[colorPixelIndex++] = intensity;
        this->pixelData[colorPixelIndex++] = 255;
    }
}

void MainPage::RenderDepthPixels()
{
    this->depthBitmap->Invalidate();

    depthImage->Source = this->depthBitmap;

}


void XamlKinect::MainPage::IRFrameArrived(WindowsPreview::Kinect::InfraredFrameReader ^sender, WindowsPreview::Kinect::InfraredFrameArrivedEventArgs ^args)
{
    bool irFrameProcessed = false;
    InfraredFrame^ frame = args->FrameReference->AcquireFrame();
    if (frame != nullptr)
    {
        FrameDescription^ frameDescription = frame->FrameDescription;

        if (((frameDescription->Width * frameDescription->Height) == this->infraredFrameDataSize) &&
            (frameDescription->Width == this->irBitmap->PixelWidth) && (frameDescription->Height == this->irBitmap->PixelHeight))
        {
            frame->CopyFrameDataToArray(this->infraredFrameData);
            irFrameProcessed = true;
        }
        delete frame;
    }

    if (irFrameProcessed)
    {
        ConvertInfraredData();
        RenderInfraredPixels();
    }
}

void MainPage::ConvertInfraredData()
{
    int irPixelIndex = 0;
    uint16* buf = this->infraredFrameData->Data;
    unsigned int len = this->infraredFrameData->Length;
    for (unsigned int i = 0; i < len; ++i)
    {
        uint16 ir = buf[i];

        float intensityRatio = (float)ir / InfraredSourceValueMaximum;
        intensityRatio /= InfraredSceneValueAverage * InfraredSceneStandardDeviations;
        intensityRatio = fminf(InfraredOutputValueMaximum, intensityRatio);
        intensityRatio = fmaxf(InfraredOutputValueMinimum, intensityRatio);

        uint8 intensity = (uint8)(intensityRatio * 255.0f);
        this->infraredPixels[irPixelIndex++] = intensity;
        this->infraredPixels[irPixelIndex++] = intensity;
        this->infraredPixels[irPixelIndex++] = intensity;
        this->infraredPixels[irPixelIndex++] = 255;
    }
}

void MainPage::RenderInfraredPixels()
{
    this->irBitmap->Invalidate();

    irImage->Source = this->irBitmap;
}