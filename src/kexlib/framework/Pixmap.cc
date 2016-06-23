#include <iostream>
#include <framework/Pixmap>

using namespace kex::gfx;

namespace {

size_t alloc_length(size_t width, size_t height, PixelFormat format)
{
    PixelTypeinfo const &type = GetPixelTypeinfo(format);
    return width * height * type.bytes;
}

uint8_t *alloc_data(size_t width, size_t height, PixelFormat format)
{
    uint8_t *data;
    data = new uint8_t[alloc_length(width, height, format)];
    return data;
}

template <typename _Src, typename _Dst>
Pixmap _convert2(const Pixmap &pSrc)
{
    Pixmap &src = const_cast<Pixmap &>(pSrc);
    Pixmap dst(src.width(), src.height(), PixelTraits<_Dst>::format);

    auto srcMap = src.map<_Src, _Dst>();
    auto srcIt = srcMap.begin();
    auto srcEnd = srcMap.end();

    auto dstIt = dst.map<_Dst>().begin();

    for (; srcIt != srcEnd; ++srcIt, ++dstIt)
        (*dstIt).set((*srcIt).get());

    return dst;
}

template <typename _Dst>
Pixmap _convert(const Pixmap &src)
{
    switch (src.format())
    {
    case PixelFormat::rgb:
        return _convert2<Rgb, _Dst>(src);

    case PixelFormat::rgba:
        return _convert2<Rgba, _Dst>(src);

    default:
        throw std::exception();
    }
}

}

Pixmap::Pixmap() :
    mFormat(PixelFormat::unknown),
    mPixels(nullptr),
    mWidth(0),
    mHeight(0)
{
}

Pixmap::Pixmap(const Pixmap &other) :
    mFormat(other.mFormat),
    mWidth(other.mWidth),
    mHeight(other.mHeight)
{
    mPixels = alloc_data(mWidth, mHeight, mFormat);
}

Pixmap::Pixmap(Pixmap &&other) :
    mFormat(other.mFormat),
    mWidth(other.mWidth),
    mHeight(other.mHeight),
    mPixels(other.mPixels)
{
    other.mPixels = nullptr;
    other.reset();
}

Pixmap::Pixmap(size_t pWidth, size_t pHeight, PixelFormat pFormat):
    mFormat(pFormat),
    mWidth(pWidth),
    mHeight(pHeight)
{
    mPixels = alloc_data(mWidth, mHeight, mFormat);
}

Pixmap::~Pixmap()
{
    if (mPixels)
        delete[] mPixels;
}

void Pixmap::reset()
{
    if (mPixels)
        delete[] mPixels;

    mPixels = nullptr;
    mWidth = 0;
    mHeight = 0;
    mFormat = PixelFormat::unknown;
}

void Pixmap::reset(size_t pWidth, size_t pHeight, PixelFormat pFormat)
{
    reset();

    mWidth = pWidth;
    mHeight = pHeight;
    mFormat = pFormat;
    mPixels = alloc_data(mWidth, mHeight, mFormat);
}

Pixmap &Pixmap::operator=(const Pixmap &other)
{
    reset();

    mFormat = other.mFormat;
    mWidth = other.mWidth;
    mHeight = other.mHeight;
    mPixels = alloc_data(mWidth, mHeight, mFormat);

    return *this;
}

Pixmap &Pixmap::operator=(Pixmap &&other)
{
    reset();

    mFormat = other.mFormat;
    mWidth = other.mWidth;
    mHeight = other.mHeight;
    mPixels = other.mPixels;

    other.mPixels = nullptr;
    other.reset();

    return *this;
}

uint8_t *Pixmap::scanline(size_t yIndex)
{
    return mPixels + yIndex * mWidth * typeinfo().bytes;
}

uint8_t const *Pixmap::scanline(size_t yIndex) const
{
    return mPixels + yIndex * mWidth * typeinfo().bytes;
}

const PixelTypeinfo &Pixmap::typeinfo() const
{
    return GetPixelTypeinfo(mFormat);
}

Pixmap Pixmap::from_data(const void *pxData, size_t width, size_t height, PixelFormat format)
{
    Pixmap retval(width, height, format);

    std::copy_n((const uint8_t *) pxData, alloc_length(width, height, format), retval.data());

    return retval;
}

Pixmap Pixmap::convert(PixelFormat pConvFormat) const
{
    if (mFormat == pConvFormat)
        return *this;

    switch (pConvFormat)
    {
    case PixelFormat::rgb:
        return _convert<Rgb>(*this);

    case PixelFormat::rgba:
        return _convert<Rgba>(*this);

    default:
        throw std::exception();
    }

}