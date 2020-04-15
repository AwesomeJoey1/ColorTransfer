#include <iostream>
#include <string>
#include <sstream>

#include <xtensor/xarray.hpp>
#include <xtensor/xmath.hpp>
#include <xtensor/xio.hpp>
#include <xtensor/xadapt.hpp>
#include <xtensor/xmanipulation.hpp>
#include <xtensor/xbroadcast.hpp>
#include <xtensor-blas/xlinalg.hpp>

#include "Image.h"

// Returns the transformation matrix from RGB to LMS space.
// Done in seperate steps for educational purposes.
xt::xarray<double> getRGB2LMS()
{
    // International Telecomunication Union standard matrix
    xt::xarray<double> M_itu{{0.4306, 0.3415, 0.1784},
                             {0.2220, 0.7067, 0.0713},
                             {0.0202, 0.1295, 0.9394}};

    // First map RGB->XYZ. Map RGB white to XYZ white. White in XYZ chromacity diagram is given by
    // x = X / (X*Y*Z) = 0.33 and
    // y = y / (X*Y*Z) = 0.33
    // --> Modify the XYZ_itu standard conversion matrix to have rows that add up to 1
    xt::xarray<double> V_x{1.0, 1.0, 1.0};

    // Solve M_itu * V_x = (1 1 1)^T for x
    xt::xarray<double> V_xItuTransform = xt::linalg::dot(xt::linalg::inv(M_itu),xt::transpose(V_x));


    std::cout <<"Standard conversion matrix:\n" << M_itu << "\n\n";

    // Multiply columns of M_itu to get final conversion matrix
    xt::xarray<double> M_rgbToXyz = M_itu * V_xItuTransform;

    std::cout << "RGB2XYZ:\n" << M_rgbToXyz << "\n\n";

    // [X Y Z]^T = M_ituTransform * [R G B]^T
    // XYZ values can be further converted to LMS space with another transformation matrix
    xt::xarray<double> M_xyzToLms = {{0.3897, 0.6890, -0.0787},
                                     {-0.2298, 1.1834, 0.0464},
                                     {0.0000, 0.0000, 1.0000}};

    // Combining the RGB to XYZ and XYZ to LMS matrices results in a transformation matrix
    // that maps from RGB to LMS space
    // The resulting matrix doesn't comply with the paper matrix but the rows actually sum up to exactly 1
    xt::xarray<double> M_rgbToLms = xt::linalg::dot(M_xyzToLms,M_rgbToXyz);

    std::cout <<"RGB2LMS:\n" << M_rgbToLms << "\n\n";

    return M_rgbToLms;
}

xt::xarray<double> getLMS2LAlphaBeta()
{
    xt::xarray<double> LMS2LAlphaBeta = xt::linalg::dot(xt::sqrt(xt::xarray<double>{{1.0/3.0, 0 ,0 },
                                                                     {0, 1.0/6.0, 0},
                                                                     {0, 0, 1.0/2.0}}),
                                         xt::xarray<double>{{1.0, 1.0, 1.0},
                                                            {1.0, 1.0, -2.0},
                                                            {1.0, -1.0, 0.0}});
    return LMS2LAlphaBeta;
}

xt::xarray<double> getLAlphaBeta2LMS()
{
    // Weird as well, because the diagonal matrix isn't the inverse of the diagonal matrix in the "forward pass"
    xt::xarray<double> LAlphaBeta2LMS = xt::linalg::dot(xt::xarray<double>{{1.0, 1.0, 1.0},
                                                                           {1.0, 1.0, -1.0},
                                                                           {1.0, -2.0, 0.0}},
                                                        xt::sqrt(xt::xarray<double>{{1.0/3.0, 0 ,0 },
                                                                                    {0, 1.0/6.0, 0},
                                                                                    {0, 0, 1.0/2.0}}));
    return LAlphaBeta2LMS;
}

void transferColor(const Image& sourceImg, const Image& transferImg, Image& destinationImg)
{
    // Initialization of the matrices. Further explanation in the corresponding methods.
    // The transformation back from L-alpha-beta space to RGB are the inverse transformations.
    xt::xarray<double> M_Rgb2Lms = getRGB2LMS();
    std::cout << "RBG2LMS\n" << M_Rgb2Lms<< "\n\n";
    xt::xarray<double> M_Lms2LAlphaBeta = getLMS2LAlphaBeta();
    std::cout << "LMS2LAlphaBeta:\n" << M_Lms2LAlphaBeta<< "\n\n";
    xt::xarray<double> M_LAlphaBeta2LMS = getLAlphaBeta2LMS();
    std::cout << "LAlphaBeta2Lms:\n" << M_LAlphaBeta2LMS<< "\n\n";
    xt::xarray<double> M_Lms2Rgb = xt::linalg::inv(M_Rgb2Lms);
    std::cout << "LMS2Rgb:\n" << M_Lms2Rgb<< "\n\n";

    //M_rgbToLms[0] = xt::log10<double>(M_rgbToLms[0]);

    for (int i = 0; i < sourceImg.width(); i++) {
        for (int j = 0; j < sourceImg.height(); j++) {
            int r,g,b;
            sourceImg.pixelColor(i,j,r,g,b);
            xt::xarray<double> rgbColor{r,g,b};
            std::cout << "Initial RGB:\n" << rgbColor << "\n\n";

            xt::xarray<double> lmsColor = xt::linalg::dot(M_Rgb2Lms, rgbColor);
            std::cout << "LMS:\n" << lmsColor << "\n\n";

            // "The data in [the LMS] color space shows a great deal of skew, which we can largely eliminate by
            // converting the data to logarithmic space"
            lmsColor = xt::log10(lmsColor);
            std::cout << "LMS-log:\n" << lmsColor << "\n\n";

            xt::xarray<double> lAlphaBetaColor = xt::linalg::dot(M_Lms2LAlphaBeta, lmsColor);
            std::cout << "LAlphaBeta:\n" << lAlphaBetaColor << "\n\n";

            // TODO: color correction with stds and means

            // Converting back to RGB
            lmsColor = xt::linalg::dot(M_LAlphaBeta2LMS, lAlphaBetaColor);
            std::cout << "LMS-log:\n" << lmsColor << "\n\n";

            lmsColor = xt::pow(10,lmsColor);
            std::cout << "LMS:\n" << lmsColor << "\n\n";

            rgbColor = xt::linalg::dot(M_Lms2Rgb, lmsColor);

            std::cout << "Final RGB:\n" << rgbColor << "\n\n";


        }
    }
}


int main()
{
    Image srcImg("../Images/ocean1.jpg");
    Image transferImg("../Images/sunset1.jpg");
    Image dstImg(800, 600, 3);

    transferColor(srcImg, transferImg, dstImg);

    transferImg.write("../Images/ocean_processed.jpg");

    return 0;
}