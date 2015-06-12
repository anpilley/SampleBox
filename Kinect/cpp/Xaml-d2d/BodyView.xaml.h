//
// BodyView.xaml.h
// Declaration of the BodyView class
//

#pragma once

#include "BodyView.g.h"

#include "Tuple.h"
#include "D2DImageSource.h"

namespace XamlKinect
{
    [Windows::UI::Xaml::Data::Bindable]
    [Windows::Foundation::Metadata::WebHostHidden]
    public ref class BodyView sealed : Windows::UI::Xaml::Data::INotifyPropertyChanged
    {
    public:
        BodyView();

        void InitBodyView(WindowsPreview::Kinect::KinectSensor^ sensor);
        void Cleanup();

        virtual event Windows::UI::Xaml::Data::PropertyChangedEventHandler ^ PropertyChanged;

    private:
        static const double HighConfidenceHandSize;
        static const double LowConfidenceHandSize;
        static const double JointThickness;
        static const double TrackedBoneThickness;
        static const double InferredBoneThickness;
        static const double ClipBoundsThickness;
        static const float InferredZPositionClamp;

        property int JointSpaceWidth;
        property int JointSpaceHeight;
        property unsigned int BodyCount
        {
            unsigned int get();
            void set(unsigned int count);
        };

        bool* trackedBodies;
        int bodyCount;

        D2DImageSource^ bodyImgSource;

        WindowsPreview::Kinect::CoordinateMapper^ coordinateMapper;
        WindowsPreview::Kinect::BodyFrameReader^ bodyFrameReader;
        Platform::Collections::Vector<WindowsPreview::Kinect::Body^>^ Bodies;
        std::vector<Windows::UI::Color> BodyColors;
        WindowsPreview::Kinect::KinectSensor^ sensor;

        std::vector<Tuple^> Bones;


        void BeginBodiesUpdate();
        void UpdateClippedEdges(
            WindowsPreview::Kinect::Body^ body);
        void UpdateBody(
            WindowsPreview::Kinect::Body^ body, 
            unsigned int bodyIndex);
        void ClearBody(unsigned int bodyIndex);
        void ClearClippedEdges();
        void UpdateJoint(
            WindowsPreview::Kinect::Joint joint, 
            Windows::Foundation::Point point);
        void UpdateHand(
            WindowsPreview::Kinect::HandState handState, 
            WindowsPreview::Kinect::TrackingConfidence trackingConfidence,
            Windows::Foundation::Point point);
        void UpdateBone(
            WindowsPreview::Kinect::Joint startJoint,
            WindowsPreview::Kinect::Joint endJoint,
            Windows::Foundation::Point startPoint,
            Windows::Foundation::Point endPoint,
            Windows::UI::Color color);
        Windows::UI::Color HandStateToColor(WindowsPreview::Kinect::HandState handState);
        void BodyFrameArrived(
            WindowsPreview::Kinect::BodyFrameReader ^sender, 
            WindowsPreview::Kinect::BodyFrameArrivedEventArgs ^args);
    };
}
