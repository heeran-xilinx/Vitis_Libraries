.. _design-example:

Design Examples Using Vitis Vision Library
###########################################

All the hardware functions in the library have their own respective
examples that are available in the github. This section provides details
of image processing functions and pipelines implemented using a
combination of various functions in Vitis vision. They illustrate how to
best implement various functionalities using the capabilities of both
the processor and the programmable logic. These examples also illustrate
different ways to implement complex dataflow paths. The following
examples are described in this section:

-  `Iterative Pyramidal Dense Optical Flow <#interactive-pyramidal>`_
-  `Corner Tracking Using Optical Flow <#corner-tracking>`_
-  `Color Detection <#color-detection>`_
-  `Difference of Gaussian Filter <#difference-gaussian-filter>`_
-  `Stereo Vision Pipeline <#stereo-vision>`_
-  `X + ML Pipeline <#x-ml-pipeline>`_

.. _interative-pyramidal:

Iterative Pyramidal Dense Optical Flow
======================================

The Dense Pyramidal Optical Flow example uses the ``xf::cv::pyrDown`` and
``xf::cv::densePyrOpticalFlow`` hardware functions from the Vitis vision
library, to create an image pyramid, iterate over it and compute the
Optical Flow between two input images. The example uses ``xf::cv::pyrDown`` function to compute the image pyramids
of the two input images. The two image pyramids are
processed by ``xf::cv::densePyrOpticalFlow``
function, starting from the smallest image size going up to the largest
image size. The output flow vectors of each iteration are fed back to
the hardware kernel as input to the hardware function. The output of the
last iteration on the largest image size is treated as the output of the
dense pyramidal optical flow example.

.. figure:: ./images/bui1554997287170.png
   :alt: 
   :figclass: image
   :name: jcr1510602888334__image_jh4_sq2_bcb


The Iterative Pyramidal Dense Optical Flow is computed in a nested for
loop which runs for iterations*pyramid levels number of iterations. The
main loop starts from the smallest image size and iterates up to the
largest image size. Before the loop iterates in one pyramid level, it
sets the current pyramid level’s height and width, in curr_height and
current_width variables. In the nested loop, the next_height variable is
set to the previous image height if scaling up is necessary, that is, in
the first iterations. As divisions are costly and one time divisions can
be avoided in hardware, the scale factor is computed in the host and
passed as an argument to the hardware kernel. After each pyramid level,
in the first iteration, the scale-up flag is set to let the hardware
function know that the input flow vectors need to be scaled up to the
next higher image size. Scaling up is done using bilinear interpolation
in the hardware kernel.

After all the input data is prepared, and the flags are set, the host
processor calls the hardware function. Please note that the host
function swaps the flow vector inputs and outputs to the hardware
function to iteratively solve the optimization problem. 

.. _corner-tracking:

Corner Tracking Using Optical Flow
===================================

This example illustrates how to detect and track the characteristic
feature points in a set of successive frames of video. A Harris corner
detector is used as the feature detector, and a modified version of
Lucas Kanade optical flow is used for tracking. The core part of the
algorithm takes in current and next frame as the inputs and outputs the
list of tracked corners. The current image is the first frame in the
set, then corner detection is performed to detect the features to track.
The number of frames in which the points need to be tracked is also
provided as the input.

Corner tracking example uses five hardware functions from the Vitis vision
library ``xf::cv::cornerHarris``, ``xf::cv:: cornersImgToList``,
``xf::cv::cornerUpdate``, ``xf::cv::pyrDown``, and ``xf::cv::densePyrOpticalFlow``.

.. figure:: ./images/tpr1554997250097.png
   :alt: 
   :figclass: image
   :name: ypx1510602888667__image_dmv_5cv_hdb

The function, ``xf::cv::cornerUpdate``, has been added to ensure
that the dense flow vectors from the output of
the\ ``xf::cv::densePyrOpticalFlow`` function are sparsely picked and stored
in a new memory location as a sparse array. This was done to ensure that
the next function in the pipeline would not have to surf through the
memory by random accesses. The function takes corners from Harris corner
detector and dense optical flow vectors from the dense pyramidal optical
flow function and outputs the updated corner locations, tracking the
input corners using the dense flow vectors, thereby imitating the sparse
optical flow behavior. This hardware function runs at 300 MHz for 10,000
corners on a 720p image, adding very minimal latency to the pipeline.



cornerUpdate()
---------------

.. rubric:: API Syntax


.. code:: c

   template <unsigned int MAXCORNERSNO, unsigned int TYPE, unsigned int ROWS, unsigned int COLS, unsigned int NPC>
   void cornerUpdate(ap_uint<64> *list_fix, unsigned int *list, uint32_t nCorners, xf::cv::Mat<TYPE,ROWS,COLS,NPC> &flow_vectors, ap_uint<1> harris_flag)

.. rubric:: Parameter Descriptions


The following table describes the template and the function parameters.

.. table:: Table: CornerUpdate Function Parameter Descriptions

   +----------+-----------------------------------------------------------+
   | Paramete | Description                                               |
   | r        |                                                           |
   +==========+===========================================================+
   | MAXCORNE | Maximum number of corners that the function needs to work |
   | RSNO     | on                                                        |
   +----------+-----------------------------------------------------------+
   | TYPE     | Input Pixel Type. Only 8-bit, unsigned, 1 channel is      |
   |          | supported (XF_8UC1)                                       |
   +----------+-----------------------------------------------------------+
   | ROWS     | Maximum height of input and output image (Must be         |
   |          | multiple of 8)                                            |
   +----------+-----------------------------------------------------------+
   | COLS     | Maximum width of input and output image (Must be multiple |
   |          | of 8)                                                     |
   +----------+-----------------------------------------------------------+
   | NPC      | Number of pixels to be processed per cycle. This function |
   |          | supports only XF_NPPC1 or 1-pixel per cycle operations.   |
   +----------+-----------------------------------------------------------+
   | list_fix | A list of packed fixed point coordinates of the corner    |
   |          | locations in 16, 5 (16 integer bits and 5 fractional      |
   |          | bits) format. Bits from 20 to 0 represent the column      |
   |          | number, while the bits 41 to 21 represent the row number. |
   |          | The rest of the bits are used for flag, this flag is set  |
   |          | when the tracked corner is valid.                         |
   +----------+-----------------------------------------------------------+
   | list     | A list of packed positive short integer coordinates of    |
   |          | the corner locations in unsigned short format. Bits from  |
   |          | 15 to 0 represent the column number, while the bits 31 to |
   |          | 16 represent the row number. This list is same as the     |
   |          | list output by Harris Corner Detector.                    |
   +----------+-----------------------------------------------------------+
   | nCorners | Number of corners to track                                |
   +----------+-----------------------------------------------------------+
   | flow_vec | Packed flow vectors as in xf::cv::DensePyrOpticalFlow     |
   | tors     | function                                                  |
   +----------+-----------------------------------------------------------+
   | harris_f | If set to 1, the function takes input corners from list.  |
   | lag      |                                                           |
   |          | if set to 0, the function takes input corners from        |
   |          | list_fix.                                                 |
   +----------+-----------------------------------------------------------+

The example codeworks on an input video which is read and processed
using the Vitis vision library. 


cornersImgToList()
--------------------

.. rubric:: API Syntax


.. code:: c

   template <unsigned int MAXCORNERSNO, unsigned int TYPE, unsigned int ROWS, unsigned int COLS, unsigned int NPC>
   void cornersImgToList(xf::cv::Mat<TYPE,ROWS,COLS,NPC> &_src, unsigned int list[MAXCORNERSNO], unsigned int *ncorners)

.. rubric:: Parameter Descriptions


The following table describes the function parameters.

.. table:: Table: CornerImgToList Function Parameter Descriptions

   +----------+-----------------------------------------------------------+
   | Paramete | Description                                               |
   | r        |                                                           |
   +==========+===========================================================+
   | \_src    | The output image of harris corner detector. The size of   |
   |          | this xf::cv::Mat object is the size of the input image to |
   |          | Harris corner detector. The value of each pixel is 255 if |
   |          | a corner is present in the location, 0 otherwise.         |
   +----------+-----------------------------------------------------------+
   | list     | A 32 bit memory allocated, the size of MAXCORNERS, to     |
   |          | store the corners detected by Harris Detector             |
   +----------+-----------------------------------------------------------+
   | ncorners | Total number of corners detected by Harris, that is, the  |
   |          | number of corners in the list                             |
   +----------+-----------------------------------------------------------+




Image Processing
~~~~~~~~~~~~~~~~~

The following steps demonstrate the Image Processing procedure in the
hardware pipeline

#. ``xf::cv::cornerharris`` is called to start processing the first input
   image
#. The output of\ ``xf::cv::cornerHarris`` is fed to\ ``xf::cv::cornersImgToList``. This function takes in an
   image with corners (marked as 255 and 0 elsewhere), and converts them
   to a list of corners.
#. \ ``xf::cv::pyrDown`` creates the two image pyramids and
   Dense Optical Flow is computed using the two image pyramids as
   described in the Iterative Pyramidal Dense Optical Flow example.
#. ``xf::cv::densePyrOpticalFlow`` is called with the two image pyramids as
   inputs.
#. ``xf::cv::cornerUpdate`` function is called to track the corner locations
   in the second image. If harris_flag is enabled, the ``cornerUpdate``
   tracks corners from the output of the list, else it tracks the
   previously tracked corners.


The ``HarrisImg()`` function takes a flag called
harris_flag which is set during the first frame or when the corners need
to be redetected. The ``xf::cv::cornerUpdate`` function outputs the updated
corners to the same memory location as the output corners list of
``xf::cv::cornerImgToList``. This means that when harris_flag is unset, the
corners input to the ``xf::cv::cornerUpdate`` are the corners tracked in the
previous cycle, that is, the corners in the first frame of the current
input frames.

After the Dense Optical Flow is computed, if harris_flag is set, the
number of corners that ``xf::cv::cornerharris`` has detected and
``xf::cv::cornersImgToList`` has updated is copied to num_corners variable
. The other being the tracked corners list, listfixed. If
harris_flag is set, ``xf::cv::cornerUpdate`` tracks the corners in ‘list’
memory location, otherwise it tracks the corners in ‘listfixed’ memory
location.

.. _color-detection: 

Color Detection
================

The Color Detection algorithm is basically used for color object
tracking and object detection, based on the color of the object. The
color based methods are very useful for object detection and
segmentation, when the object and the background have a significant
difference in color.

The Color Detection example uses four hardware functions from the
Vitis vision library. They are:

-  xf::cv::BGR2HSV
-  xf::cv::colorthresholding
-  xf::cv::erode
-  xf::cv::dilate

In the Color Detection example, the color space of the original BGR
image is converted into an HSV color space. Because HSV color space is
the most suitable color space for color based image segmentation. Later,
based on the H (hue), S (saturation) and V (value) values, apply the
thresholding operation on the HSV image and return either 255 or 0.
After thresholding the image, apply erode (morphological opening) and
dilate (morphological opening) functions to reduce unnecessary white
patches (noise) in the image. Here, the example uses two hardware
instances of erode and dilate functions. The erode followed by dilate
and once again applying dilate followed by erode.

.. figure:: ./images/ntl1554997353703.png
   :alt: 
   :figclass: image
   :name: dyn1510602889272__image_dzq_ys2_bcb

The following example demonstrates the Color Detection algorithm.

.. code:: c

   void color_detect(ap_uint<PTR_IN_WIDTH>* img_in,
                  unsigned char* low_thresh,
                  unsigned char* high_thresh,
                  unsigned char* process_shape,
                  ap_uint<PTR_OUT_WIDTH>* img_out,
                  int rows,
                  int cols) {
   
    #pragma HLS INTERFACE m_axi      port=img_in        offset=slave  bundle=gmem0
    #pragma HLS INTERFACE m_axi      port=low_thresh    offset=slave  bundle=gmem1
    #pragma HLS INTERFACE m_axi      port=high_thresh   offset=slave  bundle=gmem2
    #pragma HLS INTERFACE s_axilite  port=rows 			      bundle=control
    #pragma HLS INTERFACE s_axilite  port=cols 			      bundle=control
    #pragma HLS INTERFACE m_axi      port=process_shape offset=slave  bundle=gmem3 
    #pragma HLS INTERFACE m_axi      port=img_out       offset=slave  bundle=gmem4
    #pragma HLS INTERFACE s_axilite  port=return 		bundle=control
   

    xf::cv::Mat<IN_TYPE, HEIGHT, WIDTH, NPC1> imgInput(rows, cols);   
    #pragma HLS stream variable=imgInput.data depth=2  
    xf::cv::Mat<IN_TYPE, HEIGHT, WIDTH, NPC1> rgb2hsv(rows, cols);   
    #pragma HLS stream variable=rgb2hsv.data depth=2
    xf::cv::Mat<OUT_TYPE, HEIGHT, WIDTH, NPC1> imgHelper1(rows, cols);
    #pragma HLS stream variable=imgHelper1.data depth=2
    xf::cv::Mat<OUT_TYPE, HEIGHT, WIDTH, NPC1> imgHelper2(rows, cols);  
    #pragma HLS stream variable=imgHelper2.data depth=2
    xf::cv::Mat<OUT_TYPE, HEIGHT, WIDTH, NPC1> imgHelper3(rows, cols);
    #pragma HLS stream variable=imgHelper3.data depth=2 
    xf::cv::Mat<OUT_TYPE, HEIGHT, WIDTH, NPC1> imgHelper4(rows, cols);
    #pragma HLS stream variable=imgHelper4.data depth=2
    xf::cv::Mat<OUT_TYPE, HEIGHT, WIDTH, NPC1> imgOutput(rows, cols);
    #pragma HLS stream variable=imgOutput.data depth=2
   
    // Copy the shape data:
    unsigned char _kernel[FILTER_SIZE * FILTER_SIZE];
    for (unsigned int i = 0; i < FILTER_SIZE * FILTER_SIZE; ++i) {
        #pragma HLS PIPELINE
        _kernel[i] = process_shape[i];
    }
	
    #pragma HLS DATAFLOW
   
    // Retrieve xf::cv::Mat objects from img_in data:
    xf::cv::Array2xfMat<PTR_IN_WIDTH, IN_TYPE, HEIGHT, WIDTH, NPC1>(img_in, imgInput);

    // Convert RGBA to HSV:
    xf::cv::bgr2hsv<IN_TYPE, HEIGHT, WIDTH, NPC1>(imgInput, rgb2hsv);

    // Do the color thresholding:
    xf::cv::colorthresholding<IN_TYPE, OUT_TYPE, MAXCOLORS, HEIGHT, WIDTH, NPC1>(rgb2hsv, imgHelper1, low_thresh,
                                                                                 high_thresh);
    // Use erode and dilate to fully mark color areas:
    xf::cv::erode<XF_BORDER_CONSTANT, OUT_TYPE, HEIGHT, WIDTH, XF_KERNEL_SHAPE, FILTER_SIZE, FILTER_SIZE, ITERATIONS,
                  NPC1>(imgHelper1, imgHelper2, _kernel);
    xf::cv::dilate<XF_BORDER_CONSTANT, OUT_TYPE, HEIGHT, WIDTH, XF_KERNEL_SHAPE, FILTER_SIZE, FILTER_SIZE, ITERATIONS,
                   NPC1>(imgHelper2, imgHelper3, _kernel);
    xf::cv::dilate<XF_BORDER_CONSTANT, OUT_TYPE, HEIGHT, WIDTH, XF_KERNEL_SHAPE, FILTER_SIZE, FILTER_SIZE, ITERATIONS,
                   NPC1>(imgHelper3, imgHelper4, _kernel);
    xf::cv::erode<XF_BORDER_CONSTANT, OUT_TYPE, HEIGHT, WIDTH, XF_KERNEL_SHAPE, FILTER_SIZE, FILTER_SIZE, ITERATIONS,
                  NPC1>(imgHelper4, imgOutput, _kernel);

    // Convert _dst xf::cv::Mat object to output array:
    xf::cv::xfMat2Array<PTR_OUT_WIDTH, OUT_TYPE, HEIGHT, WIDTH, NPC1>(imgOutput, img_out);

    return;

} // End of kernel

In the given example, the source image is passed to the ``xf::cv::BGR2HSV``
function, the output of that function is passed to the
``xf::cv::colorthresholding`` module, the thresholded image is passed to the
``xf::cv::erode`` function and, the ``xf::cv::dilate`` functions and the final
output image are returned.


.. _difference-gaussian-filter: 

Difference of Gaussian Filter
==============================

The Difference of Gaussian Filter example uses four hardware functions
from the Vitis vision library. They are:

-  xf::cv::GaussianBlur
-  xf::cv::duplicateMat
-  xf::cv::delayMat
-  xf::cv::subtract

The Difference of Gaussian Filter function can be implemented by
applying Gaussian Filter on the original source image, and that Gaussian
blurred image is duplicated as two images. The Gaussian blur function is
applied to one of the duplicated images, whereas the other one is stored
as it is. Later, perform the Subtraction function on, two times Gaussian
applied image and one of the duplicated image. Here, the duplicated
image has to wait until the Gaussian applied for other one generates at
least for one pixel output. Therefore, here xf::cv::delayMat function is
used to add delay.

.. figure:: ./images/crx1554997276344.png
   :alt: 
   :figclass: image
   :name: fmq1510602889620__image_lgr_1xf_bcb

The following example demonstrates the Difference of Gaussian Filter
example.

.. code:: c

   void gaussiandiference(ap_uint<PTR_WIDTH>* img_in, float sigma, ap_uint<PTR_WIDTH>* img_out, int rows, int cols) {
   
    #pragma HLS INTERFACE m_axi      port=img_in        offset=slave  bundle=gmem0
    #pragma HLS INTERFACE m_axi      port=img_out       offset=slave  bundle=gmem1  
    #pragma HLS INTERFACE s_axilite  port=sigma 		bundle=control
    #pragma HLS INTERFACE s_axilite  port=rows 		bundle=control
    #pragma HLS INTERFACE s_axilite  port=cols 		bundle=control
    #pragma HLS INTERFACE s_axilite  port=return 		bundle=control
    
    xf::cv::Mat<TYPE, HEIGHT, WIDTH, NPC1> imgInput(rows, cols);
    xf::cv::Mat<TYPE, HEIGHT, WIDTH, NPC1> imgin1(rows, cols);
    xf::cv::Mat<TYPE, HEIGHT, WIDTH, NPC1> imgin2(rows, cols);
    xf::cv::Mat<TYPE, HEIGHT, WIDTH, NPC1> imgin3(rows, cols);
    xf::cv::Mat<TYPE, HEIGHT, WIDTH, NPC1> imgin4(rows, cols);
    xf::cv::Mat<TYPE, HEIGHT, WIDTH, NPC1> imgin5(rows, cols);
    xf::cv::Mat<TYPE, HEIGHT, WIDTH, NPC1> imgOutput(rows, cols);

 
    #pragma HLS STREAM variable=imgInput.data depth=2
    #pragma HLS STREAM variable=imgin1.data depth=2
    #pragma HLS STREAM variable=imgin2.data depth=2
    #pragma HLS STREAM variable=imgin3.data depth=2
    #pragma HLS STREAM variable=imgin4.data depth=2
    #pragma HLS STREAM variable=imgin5.data depth=2
    #pragma HLS STREAM variable=imgOutput.data depth=2
   
    #pragma HLS DATAFLOW
    

    // Retrieve xf::cv::Mat objects from img_in data:
    xf::cv::Array2xfMat<PTR_WIDTH, TYPE, HEIGHT, WIDTH, NPC1>(img_in, imgInput);

    // Run xfOpenCV kernel:
    xf::cv::GaussianBlur<FILTER_WIDTH, XF_BORDER_CONSTANT, TYPE, HEIGHT, WIDTH, NPC1>(imgInput, imgin1, sigma);
    xf::cv::duplicateMat<TYPE, HEIGHT, WIDTH, NPC1>(imgin1, imgin2, imgin3);
    xf::cv::delayMat<MAXDELAY, TYPE, HEIGHT, WIDTH, NPC1>(imgin3, imgin5);
    xf::cv::GaussianBlur<FILTER_WIDTH, XF_BORDER_CONSTANT, TYPE, HEIGHT, WIDTH, NPC1>(imgin2, imgin4, sigma);
    xf::cv::subtract<XF_CONVERT_POLICY_SATURATE, TYPE, HEIGHT, WIDTH, NPC1>(imgin5, imgin4, imgOutput);

    // Convert output xf::cv::Mat object to output array:
    xf::cv::xfMat2Array<PTR_WIDTH, TYPE, HEIGHT, WIDTH, NPC1>(imgOutput, img_out);

    return;
	} // End of kernel

In the given example, the Gaussain Blur function is applied for source
image imginput, and resultant image imgin1 is passed to
xf::cv::duplicateMat. The imgin2 and imgin3 are the duplicate images of
Gaussian applied image. Again gaussian blur is applied to imgin2 and the
result is stored in imgin4. Now, perform the subtraction between imgin4
and imgin3, but here imgin3 has to wait up to at least one pixel of
imgin4 generation. So, delay has applied for imgin3 and stored in
imgin5. Finally the subtraction performed on imgin4 and imgin5.

.. _stereo-vision: 

Stereo Vision Pipeline
========================

Disparity map generation is one of the first steps in creating a three
dimensional map of the environment. The Vitis vision library has components
to build an image processing pipeline to compute a disparity map given
the camera parameters and inputs from a stereo camera setup.

The two main components involved in the pipeline are stereo
rectification and disparity estimation using local block matching
method. While disparity estimation using local block matching is a
discrete component in Vitis vision, rectification block can be constructed
using ``xf::cv::InitUndistortRectifyMapInverse()`` and ``xf::cv::Remap()``. The
dataflow pipeline is shown below. The camera parameters are an
additional input to the pipeline.

.. figure:: ./images/qlb1554997048260.png
   :alt: 
   :figclass: image
   :width: 560px
   :height: 240px

The following code is for the pipeline.

.. code:: c

  void stereopipeline_accel(ap_uint<INPUT_PTR_WIDTH>* img_L,
                          ap_uint<INPUT_PTR_WIDTH>* img_R,
                          ap_uint<OUTPUT_PTR_WIDTH>* img_disp,
                          float* cameraMA_l,
                          float* cameraMA_r,
                          float* distC_l,
                          float* distC_r,
                          float* irA_l,
                          float* irA_r,
                          int* bm_state_arr,
                          int rows,
                          int cols) {
   
    #pragma HLS INTERFACE m_axi     port=img_L  offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi     port=img_R  offset=slave bundle=gmem5
    #pragma HLS INTERFACE m_axi     port=img_disp  offset=slave bundle=gmem6
    #pragma HLS INTERFACE m_axi     port=cameraMA_l  offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi     port=cameraMA_r  offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi     port=distC_l  offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi     port=distC_r  offset=slave bundle=gmem3
    #pragma HLS INTERFACE m_axi     port=irA_l  offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi     port=irA_r  offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi     port=bm_state_arr  offset=slave bundle=gmem4
    #pragma HLS INTERFACE s_axilite port=rows               bundle=control
    #pragma HLS INTERFACE s_axilite port=cols               bundle=control
    #pragma HLS INTERFACE s_axilite port=return                bundle=control
    

    ap_fixed<32, 12> cameraMA_l_fix[XF_CAMERA_MATRIX_SIZE], cameraMA_r_fix[XF_CAMERA_MATRIX_SIZE],
        distC_l_fix[XF_DIST_COEFF_SIZE], distC_r_fix[XF_DIST_COEFF_SIZE], irA_l_fix[XF_CAMERA_MATRIX_SIZE],
        irA_r_fix[XF_CAMERA_MATRIX_SIZE];

    for (int i = 0; i < XF_CAMERA_MATRIX_SIZE; i++) {
       
        #pragma HLS PIPELINE II=1
       
        cameraMA_l_fix[i] = (ap_fixed<32, 12>)cameraMA_l[i];
        cameraMA_r_fix[i] = (ap_fixed<32, 12>)cameraMA_r[i];
        irA_l_fix[i] = (ap_fixed<32, 12>)irA_l[i];
        irA_r_fix[i] = (ap_fixed<32, 12>)irA_r[i];
    }
    for (int i = 0; i < XF_DIST_COEFF_SIZE; i++) {
       
        #pragma HLS PIPELINE II=1
       
        distC_l_fix[i] = (ap_fixed<32, 12>)distC_l[i];
        distC_r_fix[i] = (ap_fixed<32, 12>)distC_r[i];
    }

    xf::cv::xFSBMState<SAD_WINDOW_SIZE, NO_OF_DISPARITIES, PARALLEL_UNITS> bm_state;
    bm_state.preFilterType = bm_state_arr[0];
    bm_state.preFilterSize = bm_state_arr[1];
    bm_state.preFilterCap = bm_state_arr[2];
    bm_state.SADWindowSize = bm_state_arr[3];
    bm_state.minDisparity = bm_state_arr[4];
    bm_state.numberOfDisparities = bm_state_arr[5];
    bm_state.textureThreshold = bm_state_arr[6];
    bm_state.uniquenessRatio = bm_state_arr[7];
    bm_state.ndisp_unit = bm_state_arr[8];
    bm_state.sweepFactor = bm_state_arr[9];
    bm_state.remainder = bm_state_arr[10];

    int _cm_size = 9, _dc_size = 5;

    xf::cv::Mat<XF_8UC1, XF_HEIGHT, XF_WIDTH, XF_NPPC1> mat_L(rows, cols);  
    #pragma HLS stream variable=mat_L.data depth=2 
    xf::cv::Mat<XF_8UC1, XF_HEIGHT, XF_WIDTH, XF_NPPC1> mat_R(rows, cols);  
    #pragma HLS stream variable=mat_R.data depth=2
    xf::cv::Mat<XF_16UC1, XF_HEIGHT, XF_WIDTH, XF_NPPC1> mat_disp(rows, cols);
    #pragma HLS stream variable=mat_disp.data depth=2
    xf::cv::Mat<XF_32FC1, XF_HEIGHT, XF_WIDTH, XF_NPPC1> mapxLMat(rows, cols);
    #pragma HLS stream variable=mapxLMat.data depth=2
    xf::cv::Mat<XF_32FC1, XF_HEIGHT, XF_WIDTH, XF_NPPC1> mapyLMat(rows, cols);
    #pragma HLS stream variable=mapyLMat.data depth=2
    xf::cv::Mat<XF_32FC1, XF_HEIGHT, XF_WIDTH, XF_NPPC1> mapxRMat(rows, cols);
    #pragma HLS stream variable=mapxRMat.data depth=2
    xf::cv::Mat<XF_32FC1, XF_HEIGHT, XF_WIDTH, XF_NPPC1> mapyRMat(rows, cols);
    #pragma HLS stream variable=mapyRMat.data depth=2
    xf::cv::Mat<XF_8UC1, XF_HEIGHT, XF_WIDTH, XF_NPPC1> leftRemappedMat(rows, cols);
    #pragma HLS stream variable=leftRemappedMat.data depth=2
    xf::cv::Mat<XF_8UC1, XF_HEIGHT, XF_WIDTH, XF_NPPC1> rightRemappedMat(rows, cols);
    #pragma HLS stream variable=rightRemappedMat.data depth=2
   
    #pragma HLS DATAFLOW
   
    xf::cv::Array2xfMat<INPUT_PTR_WIDTH, XF_8UC1, XF_HEIGHT, XF_WIDTH, XF_NPPC1>(img_L, mat_L);
    xf::cv::Array2xfMat<INPUT_PTR_WIDTH, XF_8UC1, XF_HEIGHT, XF_WIDTH, XF_NPPC1>(img_R, mat_R);

    xf::cv::InitUndistortRectifyMapInverse<XF_CAMERA_MATRIX_SIZE, XF_DIST_COEFF_SIZE, XF_32FC1, XF_HEIGHT, XF_WIDTH,
                                           XF_NPPC1>(cameraMA_l_fix, distC_l_fix, irA_l_fix, mapxLMat, mapyLMat,
                                                     _cm_size, _dc_size);
    xf::cv::remap<XF_REMAP_BUFSIZE, XF_INTERPOLATION_BILINEAR, XF_8UC1, XF_32FC1, XF_8UC1, XF_HEIGHT, XF_WIDTH,
                  XF_NPPC1, XF_USE_URAM>(mat_L, leftRemappedMat, mapxLMat, mapyLMat);

    xf::cv::InitUndistortRectifyMapInverse<XF_CAMERA_MATRIX_SIZE, XF_DIST_COEFF_SIZE, XF_32FC1, XF_HEIGHT, XF_WIDTH,
                                           XF_NPPC1>(cameraMA_r_fix, distC_r_fix, irA_r_fix, mapxRMat, mapyRMat,
                                                     _cm_size, _dc_size);
    xf::cv::remap<XF_REMAP_BUFSIZE, XF_INTERPOLATION_BILINEAR, XF_8UC1, XF_32FC1, XF_8UC1, XF_HEIGHT, XF_WIDTH,
                  XF_NPPC1, XF_USE_URAM>(mat_R, rightRemappedMat, mapxRMat, mapyRMat);

    xf::cv::StereoBM<SAD_WINDOW_SIZE, NO_OF_DISPARITIES, PARALLEL_UNITS, XF_8UC1, XF_16UC1, XF_HEIGHT, XF_WIDTH,
                     XF_NPPC1, XF_USE_URAM>(leftRemappedMat, rightRemappedMat, mat_disp, bm_state);

    xf::cv::xfMat2Array<OUTPUT_PTR_WIDTH, XF_16UC1, XF_HEIGHT, XF_WIDTH, XF_NPPC1>(mat_disp, img_disp);
 }

.. _x-ml-pipeline: 

X + ML Pipeline
================

This example shows how various xfOpenCV funtions can be used to accelerate preprocessing of input images before feeding them to a Deep Neural Network (DNN) accelerator.

This specific application shows how pre-processing for Googlenet_v1 can be accelerated which involves resizing the input image to 224 x 224 size followed by mean subtraction. The two main
functions from Vitis vision library which are used to build this pipeline are ``xf::cv::resize()`` and ``xf::cv::preProcess()`` which operate in dataflow.

|pp_image|

The following code shows the top level wrapper containing the ``xf::cv::resize()`` and ``xf::cv::preProcess()`` calls.

.. code:: c

    void pp_pipeline_accel(ap_uint<INPUT_PTR_WIDTH> *img_inp, ap_uint<OUTPUT_PTR_WIDTH> *img_out, int rows_in, int cols_in, int rows_out, int cols_out, float params[3*T_CHANNELS], int th1, int th2)
    {
    //HLS Interface pragmas
    #pragma HLS INTERFACE m_axi     port=img_inp  offset=slave bundle=gmem1
    #pragma HLS INTERFACE m_axi     port=img_out  offset=slave bundle=gmem2
    #pragma HLS INTERFACE m_axi     port=params  offset=slave bundle=gmem3

    #pragma HLS INTERFACE s_axilite port=rows_in     bundle=control
    #pragma HLS INTERFACE s_axilite port=cols_in     bundle=control
    #pragma HLS INTERFACE s_axilite port=rows_out     bundle=control
    #pragma HLS INTERFACE s_axilite port=cols_out     bundle=control
    #pragma HLS INTERFACE s_axilite port=th1     bundle=control
    #pragma HLS INTERFACE s_axilite port=th2     bundle=control

    #pragma HLS INTERFACE s_axilite port=return   bundle=control

    xf::cv::Mat<XF_8UC3, HEIGHT, WIDTH, NPC1>   imgInput0(rows_in, cols_in);

        #pragma HLS stream variable=imgInput0.data depth=2

        
    xf::cv::Mat<TYPE, NEWHEIGHT, NEWWIDTH, NPC_T> out_mat(rows_out, cols_out);

    #pragma HLS stream variable=out_mat.data depth=2
        
        hls::stream<ap_uint<256> > resizeStrmout;
        int srcMat_cols_align_npc = ((out_mat.cols + (NPC_T - 1)) >> XF_BITSHIFT(NPC_T)) << XF_BITSHIFT(NPC_T);

        #pragma HLS DATAFLOW
        
        xf::cv::Array2xfMat<INPUT_PTR_WIDTH,XF_8UC3,HEIGHT, WIDTH, NPC1>  (img_inp, imgInput0);
        xf::cv::resize<INTERPOLATION,TYPE,HEIGHT,WIDTH,NEWHEIGHT,NEWWIDTH,NPC_T,MAXDOWNSCALE> (imgInput0, out_mat);
        xf::cv::accel_utils obj;
        obj.xfMat2hlsStrm<INPUT_PTR_WIDTH, TYPE, NEWHEIGHT, NEWWIDTH, NPC_T, (NEWWIDTH*NEWHEIGHT/8)>(out_mat, resizeStrmout, srcMat_cols_align_npc);
        xf::cv::preProcess <INPUT_PTR_WIDTH, OUTPUT_PTR_WIDTH, T_CHANNELS, CPW, HEIGHT, WIDTH, NPC_TEST, PACK_MODE, X_WIDTH, ALPHA_WIDTH, BETA_WIDTH, GAMMA_WIDTH, OUT_WIDTH, X_IBITS, ALPHA_IBITS, BETA_IBITS, GAMMA_IBITS, OUT_IBITS, SIGNED_IN, OPMODE> (resizeStrmout, img_out, params, rows_out, cols_out, th1, th2);

    }

This piepeline is integrated with `xDNN
<https://www.xilinx.com/support/documentation/white_papers/wp504-accel-dnns.pdf>`_ accelerator and `MLsuite <https://github.com/Xilinx/ml-suite>`_ to run Googlenet_v1 inference on Alveo-U200 accelerator card and achieved
11 % speed up compared to software pre-procesing. 

* Overall Performance (Images/sec):

* with software pre-processing : 125 images/sec

* with hardware accelerated pre-processing : 140 images/sec


.. |pp_image| image:: ./images/gnet_pp.png
   :class: image 
   :width: 500

