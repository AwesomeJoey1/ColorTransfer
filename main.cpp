#include <iostream>
#include <string>
#include <sstream>

#include <xtensor/xarray.hpp>
#include <xtensor/xio.hpp>
#include <xtensor/xmanipulation.hpp>
#include <xtensor/xbroadcast.hpp>
#include <xtensor-blas/xlinalg.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>
#include "Image.h"

template<typename T>
void printMatrix(const T& mat, const int n)
{
    std::ostringstream matStringStream;
    for (auto i = 0; i <  n; i++)
    {
        for (auto j = 0; j < n; j++)
        {
            matStringStream << mat[i][j] << " ";
        }
        matStringStream << "\n";
    }
    std::cout << matStringStream.str() << "\n";
}

void transferColor(const Image& sourceImg, const Image& transferImg, Image& destinationImg)
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