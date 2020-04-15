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
        _imageData.reserve(width*height*numChannels);
    }

    Image(const std::string& path, int numComponents = 0)
    {
        if (numComponents > 4) { std::cerr << "IMAGE ERROR: Too many channels! Max is 4\n"; }

        // 0 as standard image loading. [1,4] loads image with given number of channels
        unsigned char* imgData = stbi_load(path.c_str(), &_width, &_height, &_inChannels,
                numComponents == 0 ? 0 : STBI_COMPONENTS[numComponents-1]);
        if(!imgData) { std::cerr << "IMAGE ERROR: could not load image from path:" << path << "\n"; }

        _imageData.assign(imgData, imgData + _width*_height*_inChannels);

        delete imgData;
    }

    void pixelColor(const int x, const int y, int& r, int& g, int& b) const
    {
        int idx = y*_width*_inChannels + x *_inChannels;
        r = _imageData[idx];
        g = _imageData[++idx];
        b = _imageData[++idx];
    }

    void write(const std::string& path, int outChannels= 0)
    {
        stbi_write_jpg(path.c_str(), _width, _height, outChannels == 0 ? _inChannels : outChannels, &_imageData[0], 100);
    }

    int width() const { return _width; }
    int height() const { return _height; }
    int channels() const { return _inChannels; }

private:
    int _width, _height, _inChannels;
    //unsigned char* _imageData;
    std::vector<unsigned char> _imageData;
};