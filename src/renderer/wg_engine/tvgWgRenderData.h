/*
 * Copyright (c) 2023 - 2024 the ThorVG project. All rights reserved.

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _TVG_WG_RENDER_DATA_H_
#define _TVG_WG_RENDER_DATA_H_

#include "tvgWgPipelines.h"
#include "tvgWgGeometry.h"

struct WgMeshData {
    WGPUBuffer bufferPosition{};
    WGPUBuffer bufferTexCoord{};
    WGPUBuffer bufferIndex{};
    size_t vertexCount{};
    size_t indexCount{};

    void draw(WgContext& context, WGPURenderPassEncoder renderPassEncoder);
    void drawFan(WgContext& context, WGPURenderPassEncoder renderPassEncoder);
    void drawImage(WgContext& context, WGPURenderPassEncoder renderPassEncoder);

    void update(WgContext& context, const WgPolyline* polyline);
    void update(WgContext& context, const WgGeometryData* geometryData);
    void update(WgContext& context, const WgPoint pmin, const WgPoint pmax);
    void release(WgContext& context);
};

class WgMeshDataPool {
private:
    Array<WgMeshData*> mPool;
    Array<WgMeshData*> mList;
public:
    WgMeshData* allocate(WgContext& context);
    void free(WgContext& context, WgMeshData* meshData);
    void release(WgContext& context);
};

struct WgMeshDataGroup {
    static WgMeshDataPool* gMeshDataPool;

    Array<WgMeshData*> meshes{};
    
    void append(WgContext& context, const WgPolyline* polyline);
    void append(WgContext& context, const WgGeometryData* geometryData);
    void append(WgContext& context, const WgPoint pmin, const WgPoint pmax);
    void release(WgContext& context);
};

struct WgImageData {
    WGPUTexture texture{};
    WGPUTextureView textureView{};

    void update(WgContext& context, Surface* surface);
    void release(WgContext& context);
};

enum class WgRenderSettingsType { None = 0, Solid = 1, Linear = 2, Radial = 3 };
enum class WgRenderRasterType { Solid = 0, Gradient, Image };

struct WgRenderSettings
{
    WGPUBuffer bufferGroupSolid{};
    WGPUBindGroup bindGroupSolid{};
    WGPUTexture texGradient{};
    WGPUTextureView texViewGradient{};
    WGPUBuffer bufferGroupGradient{};
    WGPUBindGroup bindGroupGradient{};
    WgRenderSettingsType fillType{};
    WgRenderRasterType rasterType{};
    bool skip{};

    void update(WgContext& context, const Fill* fill, const uint8_t* color, const RenderUpdateFlag flags);
    void release(WgContext& context);
};

struct WgRenderDataPaint
{
    WGPUBuffer bufferModelMat{};
    WGPUBuffer bufferBlendSettings{};
    WGPUBindGroup bindGroupPaint{};
    RenderRegion viewport{};
    float opacity{};
    Array<WgRenderDataPaint*> clips;

    virtual ~WgRenderDataPaint() {};
    virtual void release(WgContext& context);
    virtual Type type() { return Type::Undefined; };

    void update(WgContext& context, const tvg::Matrix& transform, tvg::ColorSpace cs, uint8_t opacity);
    void updateClips(tvg::Array<tvg::RenderData> &clips);
};

struct WgRenderDataShape: public WgRenderDataPaint
{
    WgRenderSettings renderSettingsShape{};
    WgRenderSettings renderSettingsStroke{};
    WgMeshDataGroup meshGroupShapes{};
    WgMeshDataGroup meshGroupShapesBBox{};
    WgMeshData meshDataBBox{};
    WgMeshDataGroup meshGroupStrokes{};
    WgMeshDataGroup meshGroupStrokesBBox{};
    WgPoint pMin{};
    WgPoint pMax{};
    bool strokeFirst{};
    FillRule fillRule{};

    void updateBBox(WgPoint pmin, WgPoint pmax);
    void updateMeshes(WgContext& context, const RenderShape& rshape, const Matrix& rt);
    void updateShapes(WgContext& context, const WgPolyline* polyline);
    void updateStrokesList(WgContext& context, Array<WgPolyline*> polylines, const RenderStroke* rstroke, float totalLen, float trimBegin, float trimEnd);
    void updateStrokes(WgContext& context, const WgPolyline* polyline, const RenderStroke* rstroke, float trimBegin, float trimEnd);
    void releaseMeshes(WgContext& context);
    void release(WgContext& context) override;
    Type type() override { return Type::Shape; };
};

class WgRenderDataShapePool {
private:
    Array<WgRenderDataShape*> mPool;
    Array<WgRenderDataShape*> mList;
public:
    WgRenderDataShape* allocate(WgContext& context);
    void free(WgContext& context, WgRenderDataShape* dataShape);
    void release(WgContext& context);
};

struct WgRenderDataPicture: public WgRenderDataPaint
{
    WGPUBindGroup bindGroupPicture{};
    WgImageData imageData{};
    WgMeshData meshData{};

    void release(WgContext& context) override;
    Type type() override { return Type::Picture; };
};

#endif // _TVG_WG_RENDER_DATA_H_
