# SampleBox
Various samples, snippets, etc.


Kinect/cpp/xaml-d2d
-------------------

Port of the C#/Xaml Kinect v2.0 (windows 8.1 store app) samples to c++/cx, Xaml and D2D.

Combines the Color, IR, Depth and Body frame reader samples into one sample using a similar pattern for all 4. The Body sample uses a D2D surface to draw the Bodies instead of Xaml to improve performance significantly.

* Known Issues
	* Depth perf is still bad, as it's performing multiple floating point operations per pixel per frame. This could probably be optimized significantly.