#include "mRect.h"


RectF RectF::limitedBy(const RectF& inRect, const RectF& limitRect)
{
    RectF ret;
    Point2F mSceneMin = inRect.point;
    Point2F mSceneMax = inRect.point + inRect.extent;

    // Yes, so is the limit area X less than the current view X?
    if ( limitRect.extent.x < inRect.extent.x )
    {
        // Yes, so calculate center of view.
        const F32 viewCenterX = limitRect.centre().x;

        // Half Camera Width.
        const F32 halfCameraX = inRect.extent.x * 0.5f;

        // Calculate Min/Max X.
        mSceneMin.x = viewCenterX - halfCameraX;
        mSceneMax.x = viewCenterX + halfCameraX;
    }
    else
    {
        // No, so calculate window min overlap.
        const F32 windowMinOverlapX = getMax(0.0f, limitRect.point.x - mSceneMin.x);

        // Calculate window max overlap.
        const F32 windowMaxOverlapX = getMin(0.0f, limitRect.point.x + limitRect.extent.x - mSceneMax.x);

        // Adjust Window.
        mSceneMin.x += windowMinOverlapX + windowMaxOverlapX;
        mSceneMax.x += windowMinOverlapX + windowMaxOverlapX;
    }

    // Is the limit area Y less than the current view Y?
    if ( limitRect.extent.y < inRect.extent.y )
    {
        // Yes, so calculate center of view.
        const F32 viewCenterY = limitRect.centre().y;

        // Half Camera Height.
        const F32 halfCameraY = inRect.extent.y * 0.5f;

        // Calculate Min/Max Y.
        mSceneMin.y = viewCenterY - halfCameraY;
        mSceneMax.y = viewCenterY + halfCameraY;
    }
    else
    {
        // No, so calculate window min overlap.
        const F32 windowMinOverlapY = getMax(0.0f, limitRect.point.y - mSceneMin.y);

        // Calculate window max overlap.
        const F32 windowMaxOverlapY = getMin(0.0f, limitRect.point.y + limitRect.extent.y - mSceneMax.y);

        // Adjust Window.
        mSceneMin.y += windowMinOverlapY + windowMaxOverlapY;
        mSceneMax.y += windowMinOverlapY + windowMaxOverlapY;
    }

    // Recalculate destination area.
    ret.point  = mSceneMin;
    ret.extent = mSceneMax - mSceneMin;
    return ret;
}
