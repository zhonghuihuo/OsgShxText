#pragma once

struct IGlyphCallback
{
    virtual void glBegin(int mode) = 0;
    virtual void glVertex2d(double x, double y) = 0;
    virtual void glEnd() = 0;
};
