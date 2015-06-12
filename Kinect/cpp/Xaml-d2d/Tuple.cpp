#include "pch.h"
#include "Tuple.h"

using namespace XamlKinect;

using namespace Windows::Foundation::Collections;

Tuple::Tuple(WindowsPreview::Kinect::JointType key, WindowsPreview::Kinect::JointType value)
{
    Key = key;
    Value = value;
}

//
//bool Tuple::operator<(Tuple^ rhs)
//{
//    if (this->Key < rhs->Key)
//    {
//        return true;
//    }
//    if (this->Key > rhs->Key)
//    {
//        return false;
//    }
//    if (this->Value < rhs->Value)
//    {
//        return true;
//    }
//
//    return false;
//}
//
//bool Tuple::operator==(Tuple^ rhs)
//{
//    if (this->Key != rhs->Key || this->Value != rhs->Value)
//    {
//        return false;
//    }
//    return true;
//}