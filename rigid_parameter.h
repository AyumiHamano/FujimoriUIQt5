#ifndef RIGID_PARAMETER_H
#define RIGID_PARAMETER_H

#include <iostream>


/*******************
 * Rigid parameters
 * ****************/

const std::string RIGID_PARAMETERS[57] = {
    "//ImageTypes",
    "(FixedInternalImagePixelType \"float\")",
    "(MovingInternalImagePixelType \"float\")",
    "(UseDirectionCosines \"true\")",
    "(FixedImageDimension 2)",
    "(MovingImageDimension 2)",

    "\n//Components",
    "(Registration \"MultiResolutionRegistration\")",
    "(Interpolator \"BSplineInterpolator\")",
    "(Metric \"AdvancedMattesMutualInformation\")",
    "(Optimizer \"AdaptiveStochasticGradientDescent\")",
    "(ResampleInterpolator \"FinalBSplineInterpolator\")",
    "(Resampler \"DefaultResampler\")",
    "(Transform \"EulerTransform\")",

    "\n//**************** Pyramid",
    "//Total number of resolutions",
    //"(NumberOfResolutions 6)",
    "(NumberOfResolutions 3)",
    "(FixedImagePyramid \"FixedRecursiveImagePyramid\")",
    "(MovingImagePyramid \"MovingRecursiveImagePyramid\")",
    //"(ImagePyramidSchedule 16 16 8 8 4 4 2 2 1 1 1 1)",
    "(ImagePyramidSchedule 4 4 2 2 1 1)",

    "\n//**************** Transform",
    "(AutomaticTransformInitialization \"true\")",
    "(AutomaticScaleEstimation \"true\")",
    "(HowToCombineTransform \"Compose\")",

    "\n//**************** Optimizer",
    "//Maximum number of iterations in each resolution level :",
    //"(MaximumNumberOfIterations 500 500 500 500 500 500)",
    "(MaximumNumberOfIterations 500 500 500)",
    "(MaximumNumberOfSamplingAttempts 5 5 5)",
    "(AutomaticParameterEstimation \"true\")",
    "(UseAdaptiveStepSize \"true\")",

    "\n//**************** Metric",
    "//Number of gray level bins in each resolution level : ",
    "(NumberOfHistogramBins 32)",
    "(FixedKernelBSplineOrder 3)",
    "(MovingKernelBSplineOrder 3)",

    "\n//**************** Several",
    "(WriteTransformParametersEachIteration \"false\")",
    "(WriteTransformParametersEachResolution \"false\")",
    "(ShowExactMetricValue \"false\")",
    "(ErodeMask \"false\")",

    "\n//**************** ImageSampler",
    "//Number of spatial samples used to compute the mutual information in each resolution level : ",
    "(ImageSampler \"RandomCoordinate\")",
    "(NumberOfSpatialSamples 3000)",
    "(NewSamplesEveryIteration \"true\")",

    "\n//**************** Interpolator and Resampler",
    "//Order of B-Spline interpolation used in each resolution level : ",
    "(BSplineInterpolationOrder 1)",
    "//Order of B-Spline interpolation used for applying the final deformation : ",
    "(FinalBSplineInterpolationOrder 3)",
    "//Default pixel value for pixels that come from outside the picture : ",
    "(DefaultPixelValue 0)",

    "\n//**************** Outputs",
    "(WriteResultImage \"true\")",
    "(ResultImagePixelType \"unsigned char\")",
    "(ResultImageFormat \"tiff\")",
    "END"
};



#endif // RIGID_PARAMETER_H
