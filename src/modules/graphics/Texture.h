/**
 * Copyright (c) 2006-2020 LOVE Development Team
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/

#ifndef LOVE_GRAPHICS_TEXTURE_H
#define LOVE_GRAPHICS_TEXTURE_H

// LOVE
#include "common/StringMap.h"
#include "common/math.h"
#include "common/pixelformat.h"
#include "common/Exception.h"
#include "common/Optional.h"
#include "common/int.h"
#include "Drawable.h"
#include "Quad.h"
#include "vertex.h"
#include "renderstate.h"
#include "Resource.h"
#include "image/ImageData.h"
#include "image/Image.h"
#include "image/CompressedImageData.h"

// C
#include <stddef.h>

namespace love
{
namespace graphics
{

class Graphics;

enum TextureType
{
	TEXTURE_2D,
	TEXTURE_VOLUME,
	TEXTURE_2D_ARRAY,
	TEXTURE_CUBE,
	TEXTURE_MAX_ENUM
};

struct SamplerState
{
	enum WrapMode
	{
		WRAP_CLAMP,
		WRAP_CLAMP_ZERO,
		WRAP_CLAMP_ONE,
		WRAP_REPEAT,
		WRAP_MIRRORED_REPEAT,
		WRAP_MAX_ENUM
	};

	enum FilterMode
	{
		FILTER_LINEAR,
		FILTER_NEAREST,
		FILTER_MAX_ENUM
	};

	enum MipmapFilterMode
	{
		MIPMAP_FILTER_NONE,
		MIPMAP_FILTER_LINEAR,
		MIPMAP_FILTER_NEAREST,
		MIPMAP_FILTER_MAX_ENUM
	};

	FilterMode minFilter = FILTER_LINEAR;
	FilterMode magFilter = FILTER_LINEAR;
	MipmapFilterMode mipmapFilter = MIPMAP_FILTER_NONE;

	WrapMode wrapU = WRAP_CLAMP;
	WrapMode wrapV = WRAP_CLAMP;
	WrapMode wrapW = WRAP_CLAMP;

	float lodBias = 0.0f;

	uint8 maxAnisotropy = 1;

	uint8 minLod = 0;
	uint8 maxLod = LOVE_UINT8_MAX;

	Optional<CompareMode> depthSampleMode;

	uint64 toKey() const;
	static SamplerState fromKey(uint64 key);

	static bool isClampZeroOrOne(WrapMode w);

	static bool getConstant(const char *in, FilterMode &out);
	static bool getConstant(FilterMode in, const char *&out);
	static std::vector<std::string> getConstants(FilterMode);

	static bool getConstant(const char *in, MipmapFilterMode &out);
	static bool getConstant(MipmapFilterMode in, const char *&out);
	static std::vector<std::string> getConstants(MipmapFilterMode);

	static bool getConstant(const char *in, WrapMode &out);
	static bool getConstant(WrapMode in, const char *&out);
	static std::vector<std::string> getConstants(WrapMode);
};

/**
 * Base class for 2D textures. All textures can be drawn with Quads, have a
 * width and height, and have filter and wrap modes.
 **/
class Texture : public Drawable, public Resource
{
public:

	static love::Type type;
	static int textureCount;

	enum MipmapsMode
	{
		MIPMAPS_NONE,
		MIPMAPS_MANUAL,
		MIPMAPS_AUTO,
		MIPMAPS_MAX_ENUM
	};

	enum SettingType
	{
		SETTING_WIDTH,
		SETTING_HEIGHT,
		SETTING_LAYERS,
		SETTING_MIPMAPS,
		SETTING_FORMAT,
		SETTING_LINEAR,
		SETTING_TYPE,
		SETTING_DPI_SCALE,
		SETTING_MSAA,
		SETTING_RENDER_TARGET,
		SETTING_READABLE,
		SETTING_MAX_ENUM
	};

	// Size and format will be overridden by ImageData when supplied.
	struct Settings
	{
		int width  = 1;
		int height = 1;
		int layers = 1; // depth for 3D textures
		TextureType type = TEXTURE_2D;
		MipmapsMode mipmaps = MIPMAPS_NONE;
		PixelFormat format = PIXELFORMAT_NORMAL;
		bool linear = false;
		float dpiScale = 1.0f;
		int msaa = 0;
		bool renderTarget = false;
		OptionalBool readable;
	};

	struct Slices
	{
	public:

		Slices(TextureType textype);

		void clear();
		void set(int slice, int mipmap, love::image::ImageDataBase *data);
		love::image::ImageDataBase *get(int slice, int mipmap) const;

		void add(love::image::CompressedImageData *cdata, int startslice, int startmip, bool addallslices, bool addallmips);

		int getSliceCount(int mip = 0) const;
		int getMipmapCount(int slice = 0) const;

		bool validate() const;

		TextureType getTextureType() const { return textureType; }

	private:

		TextureType textureType;

		// For 2D/Cube/2DArray texture types, each element in the data array has
		// an array of mipmap levels. For 3D texture types, each mipmap level
		// has an array of layers.
		std::vector<std::vector<StrongRef<love::image::ImageDataBase>>> data;

	}; // Slices

	static int64 totalGraphicsMemory;

	Texture(const Settings &settings, const Slices *slices);
	virtual ~Texture();

	// Drawable.
	void draw(Graphics *gfx, const Matrix4 &m) override;

	/**
	 * Draws the texture using the specified transformation with a Quad applied.
	 **/
	void draw(Graphics *gfx, Quad *quad, const Matrix4 &m);

	void drawLayer(Graphics *gfx, int layer, const Matrix4 &m);
	void drawLayer(Graphics *gfx, int layer, Quad *quad, const Matrix4 &m);

	void replacePixels(love::image::ImageDataBase *d, int slice, int mipmap, int x, int y, bool reloadmipmaps);
	void replacePixels(const void *data, size_t size, int slice, int mipmap, const Rect &rect, bool reloadmipmaps);

	virtual void generateMipmaps() = 0;

	virtual love::image::ImageData *newImageData(love::image::Image *module, int slice, int mipmap, const Rect &rect);

	virtual ptrdiff_t getRenderTargetHandle() const = 0;

	TextureType getTextureType() const;
	PixelFormat getPixelFormat() const;
	MipmapsMode getMipmapsMode() const;

	bool isRenderTarget() const;
	bool isReadable() const;

	bool isCompressed() const;
	bool isFormatLinear() const;

	bool isValidSlice(int slice) const;

	int getWidth(int mip = 0) const;
	int getHeight(int mip = 0) const;
	int getDepth(int mip = 0) const;
	int getLayerCount() const;
	int getMipmapCount() const;

	int getPixelWidth(int mip = 0) const;
	int getPixelHeight(int mip = 0) const;

	float getDPIScale() const;

	int getRequestedMSAA() const;
	virtual int getMSAA() const = 0;

	virtual void setSamplerState(const SamplerState &s);
	const SamplerState &getSamplerState() const;

	Quad *getQuad() const;

	static int getTotalMipmapCount(int w, int h);
	static int getTotalMipmapCount(int w, int h, int d);

	static bool getConstant(const char *in, TextureType &out);
	static bool getConstant(TextureType in, const char *&out);
	static std::vector<std::string> getConstants(TextureType);

	static bool getConstant(const char *in, MipmapsMode &out);
	static bool getConstant(MipmapsMode in, const char *&out);
	static std::vector<std::string> getConstants(MipmapsMode);

	static bool getConstant(const char *in, SettingType &out);
	static bool getConstant(SettingType in, const char *&out);
	static const char *getConstant(SettingType in);
	static std::vector<std::string> getConstants(SettingType);

protected:

	void setGraphicsMemorySize(int64 size);

	void uploadImageData(love::image::ImageDataBase *d, int level, int slice, int x, int y);
	virtual void uploadByteData(PixelFormat pixelformat, const void *data, size_t size, int level, int slice, const Rect &r, love::image::ImageDataBase *imgd = nullptr) = 0;

	bool validateDimensions(bool throwException) const;

	TextureType texType;

	PixelFormat format;
	bool renderTarget;
	bool readable;

	MipmapsMode mipmapsMode;

	bool sRGB;

	int width;
	int height;

	int depth;
	int layers;
	int mipmapCount;

	int pixelWidth;
	int pixelHeight;

	int requestedMSAA;

	SamplerState samplerState;

	StrongRef<Quad> quad;

	int64 graphicsMemorySize;

	// True if the image wasn't able to be properly created and it had to fall
	// back to a default texture.
	bool usingDefaultTexture;

}; // Texture

} // graphics
} // love

#endif // LOVE_GRAPHICS_TEXTURE_H
