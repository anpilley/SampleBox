//
// BodyView.xaml.cpp
// Implementation of the BodyView class
//

#include "pch.h"
#include "BodyView.xaml.h"

// There's two Visibility types, one in Windows::UI::Xaml::UIElement and this one,
// which means you constantly have to give it a fully qualified type name. shorten it
// just within this file.
#define VIS_COLLAPSED Windows::UI::Xaml::Visibility::Collapsed
#define VIS_VISIBLE Windows::UI::Xaml::Visibility::Visible

using namespace XamlKinect;

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::UI::Xaml::Shapes;

using namespace WindowsPreview::Kinect;

const double BodyView::HighConfidenceHandSize = 40.0;
const double BodyView::LowConfidenceHandSize = 20.0;
const double BodyView::JointThickness = 8.0;
const double BodyView::TrackedBoneThickness = 4.0;
const double BodyView::InferredBoneThickness = 1.0;
const double BodyView::ClipBoundsThickness = 5;
const float BodyView::InferredZPositionClamp = 0.1f;

BodyView::BodyView()
{
    InitializeComponent();
    trackedBodies = nullptr;
    bodyCount = 0;

    bodyImgSource = ref new D2DImageSource((int)this->BodyImage->Width, (int)this->BodyImage->Height, true);

    this->BodyImage->Source = bodyImgSource;
}

void BodyView::InitBodyView(KinectSensor^ sensor)
{
    this->sensor = sensor;

    this->coordinateMapper = this->sensor->CoordinateMapper;

    FrameDescription^ frameDescription = this->sensor->DepthFrameSource->FrameDescription;

    this->JointSpaceWidth = frameDescription->Width;
    this->JointSpaceHeight = frameDescription->Height;

    this->Bodies = ref new Platform::Collections::Vector<Body^>(this->sensor->BodyFrameSource->BodyCount);
    this->bodyFrameReader = this->sensor->BodyFrameSource->OpenReader();

    this->bodyFrameReader->FrameArrived += ref new TypedEventHandler<BodyFrameReader ^, BodyFrameArrivedEventArgs ^>(this, &XamlKinect::BodyView::BodyFrameArrived);

    this->BodyColors.push_back(Colors::Red);
    this->BodyColors.push_back(Colors::Orange);
    this->BodyColors.push_back(Colors::Green);
    this->BodyColors.push_back(Colors::Blue);
    this->BodyColors.push_back(Colors::Indigo);
    this->BodyColors.push_back(Colors::Violet);

    this->BodyCount = this->sensor->BodyFrameSource->BodyCount;

    this->DataContext = this;


    this->Bones.push_back(ref new Tuple(JointType::Head, JointType::Neck));
    this->Bones.push_back(ref new Tuple(JointType::Neck, JointType::SpineShoulder));
    this->Bones.push_back(ref new Tuple(JointType::SpineShoulder, JointType::SpineMid));
    this->Bones.push_back(ref new Tuple(JointType::SpineMid, JointType::SpineBase));
    this->Bones.push_back(ref new Tuple(JointType::SpineShoulder, JointType::ShoulderRight));
    this->Bones.push_back(ref new Tuple(JointType::SpineShoulder, JointType::ShoulderLeft));
    this->Bones.push_back(ref new Tuple(JointType::SpineBase, JointType::HipRight));
    this->Bones.push_back(ref new Tuple(JointType::SpineBase, JointType::HipLeft));

    this->Bones.push_back(ref new Tuple(JointType::ShoulderRight, JointType::ElbowRight));
    this->Bones.push_back(ref new Tuple(JointType::ElbowRight, JointType::WristRight));
    this->Bones.push_back(ref new Tuple(JointType::WristRight, JointType::HandRight));
    this->Bones.push_back(ref new Tuple(JointType::HandRight, JointType::HandTipRight));
    this->Bones.push_back(ref new Tuple(JointType::WristRight, JointType::ThumbRight));

    this->Bones.push_back(ref new Tuple(JointType::ShoulderLeft, JointType::ElbowLeft));
    this->Bones.push_back(ref new Tuple(JointType::ElbowLeft, JointType::WristLeft));
    this->Bones.push_back(ref new Tuple(JointType::WristLeft, JointType::HandLeft));
    this->Bones.push_back(ref new Tuple(JointType::HandLeft, JointType::HandTipLeft));
    this->Bones.push_back(ref new Tuple(JointType::WristLeft, JointType::ThumbLeft));

    this->Bones.push_back(ref new Tuple(JointType::HipRight, JointType::KneeRight));
    this->Bones.push_back(ref new Tuple(JointType::KneeRight, JointType::AnkleRight));
    this->Bones.push_back(ref new Tuple(JointType::AnkleRight, JointType::FootRight));

    this->Bones.push_back(ref new Tuple(JointType::HipLeft, JointType::KneeLeft));
    this->Bones.push_back(ref new Tuple(JointType::KneeLeft, JointType::AnkleLeft));
    this->Bones.push_back(ref new Tuple(JointType::AnkleLeft, JointType::FootLeft));
}

void BodyView::Cleanup()
{
    if (this->bodyFrameReader != nullptr)
    {
        delete this->bodyFrameReader;
    }

    if(this->Bodies!= nullptr)
    {
        this->Bodies->Clear();
    }
}

void BodyView::BodyFrameArrived(BodyFrameReader ^sender, BodyFrameArrivedEventArgs ^args)
{
    bool dataReceived = false;
    bool hasTrackedBody = false;

    BodyFrame^ bodyFrame = args->FrameReference->AcquireFrame();
    if (bodyFrame != nullptr)
    {
        bodyFrame->GetAndRefreshBodyData(this->Bodies);
        dataReceived = true;
        delete bodyFrame;
    }

    if (dataReceived)
    {
        // begindraw
        this->BeginBodiesUpdate();

        for (unsigned int bodyIndex = 0; bodyIndex < this->Bodies->Size; bodyIndex++)
        {
            Body^ body = this->Bodies->GetAt(bodyIndex);
            if (body->IsTracked)
            {
                
                this->UpdateClippedEdges(body);
                this->UpdateBody(body, bodyIndex);
                hasTrackedBody = true;
            }
            else
            {
                // mark body as no longer tracked
                this->ClearBody(bodyIndex);
            }
        }

        bodyImgSource->EndDraw();
        // enddraw.
    }
}

void BodyView::BeginBodiesUpdate()
{
    // start drawing phase.
    bodyImgSource->BeginDraw();

    // clear image to black.
    bodyImgSource->Clear(Colors::Black);

    // reset tracked bodies.
}

void BodyView::UpdateClippedEdges(Body^ body)
{
    // draw rect at edge where body clips for each edge.
    FrameEdges clippedEdges = body->ClippedEdges;

    if ((clippedEdges & FrameEdges::Left) == FrameEdges::Left)
    {
        bodyImgSource->DrawFilledRect(
            0.0f, 
            0.0f, 
            (float)ClipBoundsThickness, 
            (float)this->BodyImage->Height, 
            Colors::Red);
    }

    if ((clippedEdges & FrameEdges::Right) == FrameEdges::Right)
    {
        bodyImgSource->DrawFilledRect(
            (float)(this->BodyImage->Width - ClipBoundsThickness), 
            0.0f, 
            (float)this->BodyImage->Width, 
            (float)this->BodyImage->Height, 
            Colors::Red);
    }

    
    if ((clippedEdges & FrameEdges::Top) == FrameEdges::Top)
    {
        bodyImgSource->DrawFilledRect(
            0.0f,
            0.0f,
            (float)this->BodyImage->Width,
            (float)ClipBoundsThickness,
            Colors::Red);
    }

    
    if ((clippedEdges & FrameEdges::Bottom) == FrameEdges::Bottom)
    {
        bodyImgSource->DrawFilledRect(
            0.0f,
            (float)(this->BodyImage->Height - ClipBoundsThickness),
            (float)this->BodyImage->Width,
            (float)this->BodyImage->Height,
            Colors::Red);
    }
}

void BodyView::UpdateBody(Body^ body, unsigned int bodyIndex)
{
    IMapView<JointType, Joint>^ joints = body->Joints;
    
    IMap<JointType, Point>^ jointPointsInDepthSpace = ref new Map<JointType, Point>();

    CoordinateMapper^ coordinateMapper = this->sensor->CoordinateMapper;

    // draw joints for body.
    std::for_each(begin(body->Joints), end(body->Joints), 
        [this, body, joints, bodyIndex, coordinateMapper, jointPointsInDepthSpace](IKeyValuePair<JointType, Joint>^ jointType){

        // clamp z, we're rendering to a 2d surface.
        CameraSpacePoint position = body->Joints->Lookup(jointType->Key).Position;
        if (position.Z < 0)
        {
            position.Z = BodyView::InferredZPositionClamp;
        }

        // draw joint
        DepthSpacePoint depthSpacePoint = coordinateMapper->MapCameraPointToDepthSpace(position);
        jointPointsInDepthSpace->Insert(jointType->Key, Point(depthSpacePoint.X, depthSpacePoint.Y));
        this->UpdateJoint( 
            joints->Lookup(jointType->Key), 
            jointPointsInDepthSpace->Lookup(jointType->Key));

        // TODO consider drawing these first pass?

        // draw right hand state indicator
        if (jointType->Key == JointType::HandRight)
        {
            this->UpdateHand(
                body->HandRightState,
                body->HandRightConfidence, 
                jointPointsInDepthSpace->Lookup(jointType->Key));
        }
        
        // draw left hand state indicator
        if (jointType->Key == JointType::HandLeft)
        {
            this->UpdateHand(
                body->HandLeftState,
                body->HandLeftConfidence,
                jointPointsInDepthSpace->Lookup(jointType->Key));
        }
    });

    //Color color = this->BodyColors.at(bodyIndex);
    std::for_each(this->Bones.begin(), this->Bones.end(),
        [this, bodyIndex, jointPointsInDepthSpace, joints](Tuple^ bone){
        // draw lines between joints.
        Joint startJoint = joints->Lookup(bone->Key);
        Joint endJoint = joints->Lookup(bone->Value);
        Point startPoint = jointPointsInDepthSpace->Lookup(bone->Key);
        Point endPoint = jointPointsInDepthSpace->Lookup(bone->Value);
        this->UpdateBone(
            startJoint,
            endJoint,
            startPoint,
            endPoint,
            Colors::Blue);
    });
}

void BodyView::ClearBody(unsigned int bodyIndex)
{
    // mark body as no longer tracked, so it stops rendering.
    this->trackedBodies[bodyIndex] = false;
}

void BodyView::UpdateJoint(Joint joint, Point point)
{
    TrackingState trackingState = joint.TrackingState;

    if (trackingState != TrackingState::NotTracked)
    {
        if (trackingState == TrackingState::Tracked)
        {
            bodyImgSource->DrawFilledCircle(point.X, point.Y, (float)JointThickness, Colors::Green);
        }
        else
        {
            bodyImgSource->DrawFilledCircle(point.X, point.Y, (float)JointThickness, Colors::Yellow);
        }
    }
}

void BodyView::UpdateBone(Joint startJoint, Joint endJoint, Point startPoint, Point endPoint, Color color)
{
    if (startJoint.TrackingState == TrackingState::NotTracked || endJoint.TrackingState == TrackingState::NotTracked)
    {
        return;
    }

    double thickness = InferredBoneThickness;
    
    if (startJoint.TrackingState == TrackingState::Tracked &&
        endJoint.TrackingState == TrackingState::Tracked)
    {
        thickness = TrackedBoneThickness;
    }

    bodyImgSource->DrawLine(startPoint.X, startPoint.Y, endPoint.X, endPoint.Y, color, (float)thickness);
}

void BodyView::UpdateHand(HandState handState, TrackingConfidence trackingConfidence, Point point)
{
    // draw ellipse
    if (!_isnan(point.X) && !_isnan(point.Y))
    {
        float radius = (float)((trackingConfidence == TrackingConfidence::Low) ? LowConfidenceHandSize : HighConfidenceHandSize);
        bodyImgSource->DrawFilledCircle(point.X, point.Y, radius, this->HandStateToColor(handState));
    }
}

Color BodyView::HandStateToColor(HandState handState)
{
    switch (handState)
    {
    case HandState::Open:
        return Colors::Green;

    case HandState::Closed:
        return Colors::Red;

    case HandState::Lasso:
        return Colors::Blue;
    }

    return Colors::Transparent;
}


unsigned int BodyView::BodyCount::get()
{
    if (this->trackedBodies != nullptr)
    {
        return this->bodyCount;
    }
    return 0;
}

void BodyView::BodyCount::set(unsigned int count)
{
    if (count == 0)
    {
        delete this->trackedBodies;
        this->trackedBodies = nullptr;
        this->bodyCount = 0;
        return;
    }

    if (this->trackedBodies == nullptr || this->bodyCount != count)
    {
        this->bodyCount = count;
        if (trackedBodies != nullptr)
        {
            delete trackedBodies;
        }

        this->trackedBodies = new bool[count];

        for (unsigned int bodyIndex = 0; bodyIndex < count; bodyIndex++)
        {
            this->trackedBodies[bodyIndex] = false;
        }
    }
}