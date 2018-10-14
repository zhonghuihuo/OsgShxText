/* OpenSceneGraph example, osgshape.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#include <osg/Geode>
#include "ShxText.h"
#include <osg/ShapeDrawable>
#include <osg/Material>
#include <osg/Texture2D>
#include <osgUtil/ShaderGen>
#include <osgText/Text>

#include <osgViewer/Viewer>
#include <osgGA/StateSetManipulator>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osg/PositionAttitudeTransform>
#include <osgGA/StateSetManipulator>
#include <osg/Math>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>

const int rowColSize = 5;
osg::Group* createTexts()
{
	osg::ref_ptr<osg::Group> pGroup = new osg::Group;

    osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet();
	stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

	pGroup->setStateSet(stateset);
	auto strFontFileCn = "C:\\Windows\\Fonts\\simsun.ttc";

	auto font = osgText::readRefFontFile(strFontFileCn);

    osg::ref_ptr<osg::Geode> geode = new osg::Geode();
    for (int i = 0; i < rowColSize; ++i)
    {
        for (int j = 0; j < rowColSize; ++j)
        {
#define TextType ShxText
            osg::ref_ptr<ShxText> pText = new ShxText();
            pText->setCharacterSize(15);
            pText->SetText(L"Hello, 中国");
            pText->SetFontFile("hztxt_e.shx", "hztxt.shx");//THFont.shx gbcbig.shx

            osg::ref_ptr<osg::Geometry> pLine = new osg::Geometry;
            osg::Vec2Array* pVertexArray = new osg::Vec2Array;
            pVertexArray->push_back(osg::Vec2(100 * i, 20 * j));
            pVertexArray->push_back(osg::Vec2(100 * i + pText->length(), 20 * j + 0));
            pLine->setVertexArray(pVertexArray);
            osg::Vec3Array* pColorArray = new osg::Vec3Array(osg::Array::BIND_OVERALL);
            pColorArray->push_back(osg::Vec3(1, 1, 0));
            pLine->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP, 0, 2));
            geode->addDrawable(pLine);

//#define TextType osgText::Text
            //osg::ref_ptr<osgText::Text> pText = new osgText::Text;
            //pText->setText(L"Hello, 中国");
            //pText->setCharacterSize(10);
            //pText->setFont(font);

            if (i == 0)
                pText->setCharacterSizeMode(TextType::SCREEN_COORDS);
            else if (i == 1)
                pText->setCharacterSizeMode(TextType::OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT);

            if (j == 0)
                pText->setRotation(osg::Quat(osg::PI_4f, osg::Z_AXIS));

            pText->setPosition(osg::Vec3(100 * i, 20 * j, 0));

            geode->addDrawable(pText);
        }
    }
    pGroup->addChild(geode);
	return pGroup.release();
}

int main(int argc, char **argv)
{
	// construct the viewer.
	osgViewer::Viewer viewer;
	//viewer.setUpViewInWindow(0, 0, 1920, 540);
    osg::ref_ptr<osg::GraphicsContext::Traits> traits =
        new osg::GraphicsContext::Traits;
    traits->x = 50;
    traits->y = 50;
    traits->width = 800;
    traits->height = 600;
    traits->windowDecoration = true;
    traits->doubleBuffer = true;
    //traits->samples = 4;

    osg::ref_ptr<osg::GraphicsContext> gc =
        osg::GraphicsContext::createGraphicsContext(traits.get());
    osg::ref_ptr<osg::Camera> camera = viewer.getCamera();
    camera->setGraphicsContext(gc);
    camera->setViewport(
        new osg::Viewport(0, 0, traits->width, traits->height));
    camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    camera->setClearColor(osg::Vec4f(0.2f, 0.2f, 0.4f, 1.0f));

	// add model to viewer.
	viewer.setSceneData(createTexts());

	// add the state manipulator
	viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));

	// add the stats handler
	viewer.addEventHandler(new osgViewer::StatsHandler);
	osg::ref_ptr<osgGA::TrackballManipulator> man = new osgGA::TrackballManipulator();
	man->setHomePosition(osg::Vec3d(50 * rowColSize, 10 * rowColSize, 150 * rowColSize), osg::Vec3d(50 * rowColSize, 10 * rowColSize, 0), osg::Vec3d(0, 1, 0));
	viewer.setCameraManipulator(man.get());
	viewer.realize();

	return viewer.run();
}
