#pragma once

namespace XamlKinect
{
    ref class Tuple sealed : 
        Windows::Foundation::Collections::IKeyValuePair<
            WindowsPreview::Kinect::JointType, 
            WindowsPreview::Kinect::JointType>
    {
    public:
        Tuple(WindowsPreview::Kinect::JointType key, WindowsPreview::Kinect::JointType value);

        virtual property WindowsPreview::Kinect::JointType Key;

        virtual property WindowsPreview::Kinect::JointType Value;

    private:

        //bool operator<(Tuple^ rhs);
        //bool operator==(Tuple^ rhs);
    };

}

