/***************************************************************************
* Copyright (C) 2017, Deping Chen, cdp97531@sina.com
*
* All rights reserved.
* For permission requests, write to the author.
*
* This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
* KIND, either express or implied.
***************************************************************************/
#pragma once

struct IGlyphCallback
{
    virtual void glBegin(int mode) = 0;
    virtual void glVertex2d(double x, double y) = 0;
    virtual void glEnd() = 0;
};
