/**
    @author Jukka Jylunki, keyring

    This work is released to Public Domain, do whatever you want with it.
*/

#include <utility>

#include "rect.h"

/*
#include "clb/Algorithm/Sort.h"

int CompareRectShortSide(const Rect &a, const Rect &b)
{
    using namespace std;

    int smallerSideA = min(a.width, a.height);
    int smallerSideB = min(b.width, b.height);

    if (smallerSideA != smallerSideB)
        return clb::sort::TriCmp(smallerSideA, smallerSideB);

    // Tie-break on the larger side.
    int largerSideA = max(a.width, a.height);
    int largerSideB = max(b.width, b.height);

    return clb::sort::TriCmp(largerSideA, largerSideB);
}
*/
/*
int NodeSortCmp(const Rect &a, const Rect &b)
{
    if (a.x != b.x)
        return clb::sort::TriCmp(a.x, b.x);
    if (a.y != b.y)
        return clb::sort::TriCmp(a.y, b.y);
    if (a.width != b.width)
        return clb::sort::TriCmp(a.width, b.width);
    return clb::sort::TriCmp(a.height, b.height);
}
*/
bool IsContainedIn(const Rect &a, const Rect &b)
{
    return a.x >= b.x && a.y >= b.y
        && a.x+a.width <= b.x+b.width
        && a.y+a.height <= b.y+b.height;
}

BinarySearch::BinarySearch(int min, int max, int fuzziness)
{
    this->min = min;
    this->max = max;
    this->fuzziness = fuzziness;
}

int BinarySearch::reset()
{
    low = min;
    hign = max;
    current = (low+hign) >> 1;
    return current;
}

int BinarySearch::next(bool result)
{
    if(low >= hign) return -1;
    if(result){
        low = current + 1;
    }else{
        hign = current - 1;
    }
    current = (low+hign) >> 1;
    if(std::abs(hign-low) < fuzziness) return -1;
    return current;
}
