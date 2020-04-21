#include <iostream>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtx/matrix_operation.hpp>

#include "Image.h"

void calcLAlphaBetaImg(const Image& img, std::vector<float>& lAlphaBetaImg)
{
    glm::mat3 M_rgb2lms = glm::transpose(glm::mat3(0.3811, 0.5783, 0.0402,
                                                   0.1967, 0.7244, 0.0782,
                                                   0.0241 ,0.1288, 0.8444));

    glm::vec3 diag(1.0f/glm::sqrt(3),1.0f/glm::sqrt(6), 1.0f/glm::sqrt(2));
    glm::mat3 M_lms2lab = glm::transpose(glm::diagonal3x3(diag)) * glm::transpose(glm::mat3(1, 1, 1,
                                                             1, 1, -2,
                                                             1, -1, 0));

    for (int idx = 0; idx < img.size(); idx+=3)
    {
        unsigned char r, g, b;
        img.pixelColor(idx, r, g, b);
        glm::vec3 rgbColor(static_cast<float>(r)/255.0f, static_cast<float>(g)/255.0f, static_cast<float>(b)/255.0f);

        glm::vec3 lmsColor = M_rgb2lms * rgbColor;
        lmsColor = glm::vec3(glm::max(FLT_MIN, lmsColor.r),
                             glm::max(FLT_MIN, lmsColor.g),
                             glm::max(FLT_MIN, lmsColor.b));

        // log10(x) = ln(x) / ln(10)
        glm::vec3 lmsLogColor = glm::log(lmsColor) / glm::log(10.0f);

        glm::vec3 lALphaBetaColor = M_lms2lab * lmsLogColor;

        lAlphaBetaImg.push_back(lALphaBetaColor.r);
        lAlphaBetaImg.push_back(lALphaBetaColor.g);
        lAlphaBetaImg.push_back(lALphaBetaColor.b);
    }
}

void calcAxisMeans(const std::vector<float>& imgData, glm::vec3& means)
{
    const int numValues = imgData.size();
    glm::vec3 sum(0);
    for(int idx = 0; idx < numValues; idx++)
    {
        sum.x += imgData[idx];
        sum.y += imgData[++idx];
        sum.z += imgData[++idx];
    }

    means = sum / static_cast<float>(numValues);
}

void calcAxisStds(const std::vector<float>& imgData, const glm::vec3& means, glm::vec3& stds)
{
    const int numValues = imgData.size();
    glm::vec3 sum(0);
    for (int idx = 0; idx < numValues; idx++)
    {
        sum.x += glm::pow(imgData[idx] - means.x,2);
        sum.y += glm::pow(imgData[++idx] - means.y,2);
        sum.z += glm::pow(imgData[++idx] - means.z,2);
    }

    stds = glm::sqrt(1.0f/ (static_cast<float>(numValues)-1.0f) * sum);
}

void transfer(std::vector<float>& lAlphaBetaData, const glm::vec3& srcMeans, const glm::vec3& srcStds,
                const glm::vec3& transMeans, const glm::vec3& transStds)
{
    glm::vec3 lALphaBetaColor;
    for (int idx = 0; idx < lAlphaBetaData.size(); idx+=3)
    {
        lAlphaBetaData[idx] = (transStds.x/srcStds.x) * (lAlphaBetaData[idx] - srcMeans.x) + transMeans.x;
        lAlphaBetaData[idx+1] = (transStds.y/srcStds.y) * (lAlphaBetaData[idx+1] - srcMeans.y) + transMeans.y;
        lAlphaBetaData[idx+2] = (transStds.z/srcStds.z) * (lAlphaBetaData[idx+2] - srcMeans.z) + transMeans.z;
    }
}

void calcRGBImg(const std::vector<float>& lAlphaBetaData, Image& destImg)
{
    glm::mat3 M_lab2lms = glm::transpose(glm::mat3(1,1,1,
                                    1,1, -1,
                                    1, -2,0)) * glm::transpose(glm::diagonal3x3(glm::vec3(1/glm::sqrt(3),
                                                                                        1/glm::sqrt(6),
                                                                                        1/glm::sqrt(2))));
    glm::mat3 M_lms2rgb = glm::transpose(glm::mat3(4.4679, -3.5873, 0.1193,
                                                       -1.2186, 2.3809, -0.1624,
                                                       0.0497, -0.2439, 1.2045));

    for (int idx = 0; idx < lAlphaBetaData.size(); idx+=3)
    {
        glm::vec3 lAlphaBetaColor(lAlphaBetaData[idx], lAlphaBetaData[idx+1], lAlphaBetaData[idx+2]);
        glm::vec3 lmsLogColor = M_lab2lms * lAlphaBetaColor;
        glm::vec3 lmsColor = glm::pow(glm::vec3(10), lmsLogColor);
        glm::vec3 rgbColor = M_lms2rgb * lmsColor;
        rgbColor = glm::clamp(rgbColor, 0.0f, 1.0f);
        destImg.setPixel(idx, static_cast<unsigned char>(255.0f * rgbColor.r),
                              static_cast<unsigned char>(255.0f * rgbColor.g),
                              static_cast<unsigned char>(255.0f * rgbColor.b));
    }
}

void transferColor(const Image& sourceImg, const Image& transferImg, Image& destinationImg)
{
    std::vector<float> srcLAlphaBeta, transLAlphaBeta;
    srcLAlphaBeta.reserve(sourceImg.size());
    transLAlphaBeta.reserve(transferImg.size());
    calcLAlphaBetaImg(sourceImg, srcLAlphaBeta);
    calcLAlphaBetaImg(transferImg, transLAlphaBeta);

    const auto [minS, maxS] = std::minmax_element(srcLAlphaBeta.begin(), srcLAlphaBeta.end());
    const auto [minT, maxT] = std::minmax_element(transLAlphaBeta.begin(), transLAlphaBeta.end());

    glm::vec3 srcMeans, srcStds, transMeans, transStds;
    calcAxisMeans(srcLAlphaBeta, srcMeans);
    calcAxisMeans(transLAlphaBeta, transMeans);

    calcAxisStds(srcLAlphaBeta, srcMeans, srcStds);
    calcAxisStds(transLAlphaBeta, transMeans, transStds);

    transfer(srcLAlphaBeta, srcMeans, srcStds, transMeans, transStds);

    calcRGBImg(srcLAlphaBeta, destinationImg);
}

int main(int argc, char* argv[])
{
    if (argc != 4) {
        std::cerr << "ERROR: Wrong number of given parameters! Call ./ColorTransfer <srcImg> <transferImg> <destination>\n";
        return -1;
    }
    Image srcImg(argv[1]);
    Image transferImg(argv[2]);
    Image dstImg(srcImg.width(), srcImg.height(), 3);

    transferColor(srcImg, transferImg, dstImg);

    dstImg.write(argv[3]);

    return 0;
}