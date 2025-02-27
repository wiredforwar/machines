#pragma once

#include <unordered_map>
#include <memory>
#include "gui/painter.hpp"

//
// Ideally, clients should utilize the named bitmaps feature
//
class GuiRootSharedBitmaps final
{
public:
    GuiRootSharedBitmaps();
    explicit GuiRootSharedBitmaps(const IGuiPainter& painter);

    ~GuiRootSharedBitmaps();

    // Create or reassign a named bitmap, loading if necessary
    void createUpdateNamedBitmap(const std::string& imageName, const std::string& filePath, float scale = 1);

    // Load a bitmap unless its already been loaded
    //  This method will be invoked by createUpdateNamedBitmap
    void loadSharedBitmap(const std::string& filePath, float scale = 1);

    // Retrieve an OWNING REFERENCE to a named bitmap, e.g.: "backdrop"
    std::shared_ptr<GuiBitmap> getNamedBitmap(const std::string& imageName) const noexcept;

    // Retrieve a NON-OWNING REFERENCE to a shared bitmap, associated by filename/path
    std::weak_ptr<GuiBitmap> getSharedBitmap(const std::string& filePath) const noexcept;

    // Blit a section defined by `sourceArea` from `bitmap` to `destination`. Apply a transformation with a lambda.
    void blitNamedBitmapFromArea(
        const std::shared_ptr<GuiBitmap>& bitmap,
        const Gui::Box& sourceArea,
        const Gui::Coord& destination,
        std::function<Gui::Box(const Gui::Box&)> fnSourceAreaTransform) const;

    // Blit the `bitmap` to `destination`
    void blitNamedBitmap(const std::shared_ptr<GuiBitmap>& bitmap, const Gui::Coord& destination) const;

    // Getters for named bitmap width & height
    int getWidthOfNamedBitmap(const std::shared_ptr<GuiBitmap>& bitmap) const;
    int getHeightOfNamedBitmap(const std::shared_ptr<GuiBitmap>& bitmap) const;

private:
    const IGuiPainter& guiPainter_;

    std::unordered_map<std::string, std::shared_ptr<GuiBitmap>> sharedBitmaps_;
    std::unordered_map<std::string, std::string> namedBitmaps_;
};
