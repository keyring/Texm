/**
    @author Jukka Jylunki, keyring

    @brief Implements different bin packer algorithms that use the MAXRECTS data structure.

    This work is released to Public Domain, do whatever you want with it.
*/

#ifndef MAX_RECTS_PACK_H
#define MAX_RECTS_PACK_H



#include <vector>

#include "rect.h"



/** MaxRectsBinPack implements the MAXRECTS data structure and different bin packing algorithms that
    use this structure. */
class MaxRectsBinPack
{
public:
    /// Instantiates a bin of size (0,0). Call Init to create a new bin.
    MaxRectsBinPack();

    /// Instantiates a bin of the given size.
    MaxRectsBinPack(int width, int height, bool rotate);

    /// (Re)initializes the packer to an empty bin of width x height units. Call whenever
    /// you need to restart with a new bin.
    void Init(int width, int height, bool rotate);

    /// Inserts the given list of rectangles in an offline/batch mode, possibly rotated.
    /// @param rects The list of rectangles to insert. This vector will be destroyed in the process.
    /// @param dst [out] This list will contain the packed rectangles. The indices will not correspond to that of rects.
    /// @param method The rectangle placement rule to use when packing.
    void Insert(std::vector<RectSize> &rects, std::vector<Rect> &dst, FreeRectChoiceHeuristic method);

    /// Inserts a single rectangle into the bin, possibly rotated.
    Rect Insert(Rect rect, FreeRectChoiceHeuristic method);

    /// Computes the ratio of used surface area to the total bin area.
    float Occupancy() const;

    Page *Result() const;

    Rect UsedRect() const;

private:
    int binWidth;
    int binHeight;
    bool isRotate;

    std::vector<Rect> usedRectangles;
    std::vector<Rect> freeRectangles;

    /// Computes the placement score for placing the given rectangle with the given method.
    /// @param score1 [out] The primary placement score will be outputted here.
    /// @param score2 [out] The secondary placement score will be outputted here. This isu sed to break ties.
    /// @return This struct identifies where the rectangle would be placed if it were placed.
    Rect ScoreRect(int width, int height, FreeRectChoiceHeuristic method, int &score1, int &score2) const;

    /// Places the given rectangle into the bin.
    void PlaceRect(const Rect &node);

    /// Computes the placement score for the -CP variant.
    int ContactPointScoreNode(int x, int y, int width, int height) const;

    Rect FindPositionForNewNodeBottomLeft(int width, int height, int &bestY, int &bestX) const;
    Rect FindPositionForNewNodeBestShortSideFit(int width, int height, int &bestShortSideFit, int &bestLongSideFit) const;
    Rect FindPositionForNewNodeBestLongSideFit(int width, int height, int &bestShortSideFit, int &bestLongSideFit) const;
    Rect FindPositionForNewNodeBestAreaFit(int width, int height, int &bestAreaFit, int &bestShortSideFit) const;
    Rect FindPositionForNewNodeContactPoint(int width, int height, int &contactScore) const;

    /// @return True if the free node was split.
    bool SplitFreeNode(Rect freeNode, const Rect &usedNode);

    /// Goes through the free rectangle list and removes any redundant entries.
    void PruneFreeList();
};




#endif // MAX_RECTS_PACK_H
