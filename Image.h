#pragma once
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <vector>

int STBI_COMPONENTS[4] = {STBI_grey, STBI_grey_alpha, STBI_rgb, STBI_rgb_alpha};

class Image
{
public:
    Image(int width, int height, int numChannels) : _width(width), _height(height), _inChannels(numChannels)
    {
        _imgSize = width * height * numChannels;
        _imageData.resize(_imgSize);
    }

    Image(const std::vector<unsigned char>& imgData, int width, int height, int numChannels) :
            _width(width), _height(height), _inChannels(numChannels)
    {
        _imgSize = width * height * numChannels;
        _imageData = imgData;
    }

    Image(const std::string& path, int numComponents = 0)
    {
        if (numComponents > 4) { std::cerr << "IMAGE ERROR: Too many channels! Max is 4\n"; }

        // 0 as standard image loading. [1,4] loads image with given number of channels
        unsigned char* imgData = stbi_load(path.c_str(), &_width, &_height, &_inChannels,
                numComponents == 0 ? 0 : STBI_COMPONENTS[numComponents-1]);
        if(!imgData) { std::cerr << "IMAGE ERROR: could not load image from path:" << path << "\n"; }

        _imgSize = _width * _height * _inChannels;
        _imageData.assign(imgData, imgData + _imgSize);
        delete imgData;
    }

    void calcAxisMeans(double& xMean, double& yMean, double& zMean) const
    {
        double a =0.0, b = 0.0, c=0.0;
        for (int i = 0; i < _imgSize; i++)
        {
            if(_imageData[i] != _imageData[i] || _imageData[i+1] != _imageData[i+1] || _imageData[i+2] != _imageData[i+2]){
                int a = _imageData[i];
                int b = _imageData[i+1];
                int c = _imageData[i+2];
            }
           a += (_imageData[i]);
           b += (_imageData[++i]);
           c += (_imageData[++i]);

        }

        double pixelsPerChannel = static_cast<double>(_imgSize)/3.0;
        xMean /= pixelsPerChannel;
        yMean /= pixelsPerChannel;
        zMean /= pixelsPerChannel;
    }

    void calcStandardDeviation(double xMean, double yMean, double zMean, double& xStd, double& yStd, double& zStd) const
    {
        //sqrt(1/(numPixels-1) * sum(x_i - x)^2)
        double xSum = 0, ySum = 0, zSum = 0;
        for (int i = 0; i < _imgSize; i++)
        {
            double x = static_cast<double>(_imageData[i]);
            double y = static_cast<double>(_imageData[++i]);
            double z = static_cast<double>(_imageData[++i]);
            xSum += pow(x - xMean,2);
            ySum += pow(y - yMean, 2);
            zSum += pow(z - zMean, 2);
        }

        int numPixels = _width*height();
        xStd = sqrt(1.0/(numPixels-1) * xSum);
        yStd = sqrt(1.0/(numPixels-1) * ySum);
        zStd = sqrt(1.0/(numPixels-1) * zSum);
    }

    void pixelData(std::vector<unsigned char>& pData) const
    {
        pData = _imageData;
    }

    void pixelColor(const int x, const int y, unsigned char& r, unsigned char& g, unsigned char& b) const
    {
        int idx = y*_width*_inChannels + x *_inChannels;
        r = _imageData[idx];
        g = _imageData[++idx];
        b = _imageData[++idx];
    }

    void pixelColor(int idx, unsigned char& r, unsigned char& g, unsigned char& b) const
    {
        r = _imageData[idx];
        g = _imageData[++idx];
        b = _imageData[++idx];
    }

    void setPixel(const int x, const int y, unsigned char r, unsigned char g , unsigned char b) {
        if (x > _width || y >= _height || x < 0 || y < 0) {
            std::cerr << "IMAGE ERROR: wrong dimension for x or y when setting pixel color!\n";
            return;
        }
        int idx = y*_width*_inChannels + x *_inChannels;
        setPixel(idx, r, g, b);
    }

    void setPixel(int idx, const unsigned char r, const unsigned char g, const unsigned char b)
    {
        if (idx > _imgSize) {
            std::cerr << "IMAGE ERROR: wrong index when setting pixel color! Max index is " << _imgSize << "\n";
            return;
        }

        _imageData[idx] = r;
        _imageData[++idx] = g;
        _imageData[++idx] = b;
    }

    void write(const std::string& path, int outChannels= 0)
    {
        // TODO: correct rounding on converting to unsigned char vector
        stbi_write_jpg(path.c_str(), _width, _height, outChannels == 0 ? _inChannels : outChannels,
                    &_imageData[0], 100);
    }

    int width() const { return _width; }
    int height() const { return _height; }
    int channels() const { return _inChannels; }
    long size() const { return _imgSize; }
    std::vector<unsigned char> getData() { return _imageData; }

private:
    int _width, _height, _inChannels, _imgSize;
    std::vector<unsigned char> _imageData;
};