#pragma once
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <vector>

class Image
{
public:
    Image(int width, int height, int numChannels) : _width(width), _height(height), _inChannels(numChannels)
    {
        _imgSize = width * height * numChannels;
        _imageData.resize(_imgSize);
    }

    Image(const std::string& path)
    {
        unsigned char* imgData = stbi_load(path.c_str(), &_width, &_height, &_inChannels, STBI_rgb);
        if(!imgData) {
            std::cerr << "IMAGE ERROR: could not load image from path:" << path << "\n";
            exit(1);
        }

        _imgSize = _width * _height * _inChannels;
        _imageData.assign(imgData, imgData + _imgSize);

        delete imgData;
    }

    void pixelColor(int idx, unsigned char& r, unsigned char& g, unsigned char& b) const
    {
        r = _imageData[idx];
        g = _imageData[++idx];
        b = _imageData[++idx];
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
        stbi_write_jpg(path.c_str(), _width, _height, outChannels == 0 ? _inChannels : outChannels,
                    &_imageData[0], 100);
    }

    int width() const { return _width; }
    int height() const { return _height; }
    long size() const { return _imgSize; }

private:
    int _width, _height, _inChannels, _imgSize;
    std::vector<unsigned char> _imageData;
};