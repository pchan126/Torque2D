#include "gui/containers/guiGridCtrl.h"

//------------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(GuiGridControl);

//------------------------------------------------------------------------------

GuiGridControl::GuiGridControl()
{
	mIsContainer = true;
}

//------------------------------------------------------------------------------

void GuiGridControl::initPersistFields()
{
	Parent::initPersistFields();

	addField("Rows",		TypeStringTableEntryVector, Offset(mGridRows, GuiGridControl), "Number of rows in the grid");
	addField("Columns",     TypeStringTableEntryVector, Offset(mGridCols, GuiGridControl), "Number of columns in the grid");
}

//------------------------------------------------------------------------------

bool GuiGridControl::onWake()
{
	if (!Parent::onWake())
        return false;

	return true;
}

//------------------------------------------------------------------------------

void GuiGridControl::onSleep()
{
	Parent::onSleep();
}

//------------------------------------------------------------------------------

void GuiGridControl::inspectPostApply()
{
    resize(getPosition(), getExtent());
}

//------------------------------------------------------------------------------

bool GuiGridControl::IsPointInGridControl(GuiControl* ctrl, const Point2I& pt)
{
	if (!mRowSizes.empty() && !mColSizes.empty() )
	{
		RectI gridRect = GetGridRect(ctrl);
		RectI ctrlRect = ctrl->getBounds();

		Point2I chkPt = gridRect.point + ctrlRect.point;
		Point2I chkBound = chkPt + ctrlRect.extent;

		if (pt.x >= chkPt.x && pt.x <= chkBound.x && pt.y >= chkPt.y && pt.y <= chkBound.y)
			return true;
		else
			return false;
	}
	else
    {
		return false;
    }

}

//------------------------------------------------------------------------------

void GuiGridControl::addObject(SimObject *obj)
{
	if (mRowSizes.size() <= 0 && mRowSizes.size() <= 0)
		AdjustGrid(mBounds.extent);

	GuiControl *ctrl = static_cast<GuiControl *>(obj);
	
    if (ctrl)
	{
		RectI ctrlRect = GetGridRect(ctrl);
		if (ctrl->getExtent().isZero())
		{
			ctrl->setExtent(ctrlRect.extent);
		}
		else
		{
			if (ctrl->getWidth() > ctrlRect.extent.x)
				ctrl->setWidth(ctrlRect.extent.x);
			if (ctrl->getHeight() > ctrlRect.extent.y)
				ctrl->setHeight(ctrlRect.extent.y);
		}

		Point2I pt = ctrl->getPosition();
		mOrginalControlPos.push_back(pt);
		pt += ctrlRect.point;
		ctrl->setPosition(pt);
	}

	Parent::addObject(obj);
}

//------------------------------------------------------------------------------

void GuiGridControl::removeObject(SimObject *obj)
{
	for(int idx =0; idx < objectList.size();idx++)
	{
		if ( objectList[idx] == obj )
		{
			mOrginalControlPos.erase(idx);
			break;
		}
	}

	Parent::removeObject(obj);
}

//------------------------------------------------------------------------------

bool GuiGridControl::resize(const Point2I &newPosition, const Point2I &newExtent)
{
    if (Parent::resize( newPosition, newExtent ))
        return false;

	setUpdate();

	Point2I actualNewExtent = Point2I(  std::max(mMinExtent.x, newExtent.x), std::max(mMinExtent.y, newExtent.y));
	mBounds.set(newPosition, actualNewExtent);

	bool bFirstResize = false;

	Vector<RectI> oldCtrlRect;
	
	if (mRowSizes.empty() && mColSizes.empty())
	{
		bFirstResize = true;
	}
	else
	{
		for(auto i:*this)
		{
			GuiControl *ctrl = static_cast<GuiControl *>(i);
			if (ctrl)
			{
				RectI newRect = GetGridRect(ctrl);		
				oldCtrlRect.push_back(newRect);
			}
		}

	}

	AdjustGrid(mBounds.extent);

	//resize and position all child controls.
	int idx = 0;
	for(auto i:*this)
	{
		GuiControl *ctrl = static_cast<GuiControl *>(i);
		if (ctrl)
		{
			RectI newRect = GetGridRect(ctrl);		
			
			if (ctrl->getExtent().x == 0 && ctrl->getExtent().y == 0)
				ctrl->setExtent(newRect.extent);

			ctrl->setPosition(mOrginalControlPos[idx] + newRect.point);

			if (bFirstResize)
			{
				ctrl->parentResized(newRect, newRect);
			}
			else
			{
				ctrl->parentResized(oldCtrlRect[idx++], newRect);
			}
		}
	}

	GuiControl *parent = getParent();

	if (parent)
		parent->childResized(this);

	setUpdate();
    return true;
}

//------------------------------------------------------------------------------

RectI GuiGridControl::GetGridRect(GuiControl* ctrl)
{
	S32 col = dAtoi(ctrl->getDataField( StringTable->insert("Col"), NULL));
	S32 row = dAtoi(ctrl->getDataField( StringTable->insert("Row"), NULL));
	S32 colSpan = dAtoi(ctrl->getDataField( StringTable->insert("ColSpan"), NULL));
	S32 rowSpan = dAtoi(ctrl->getDataField( StringTable->insert("RowSpan"), NULL));

	AssertFatal (col < mColSizes.size(), "Col is out of bounds");
	AssertFatal (row < mRowSizes.size(), "Row is out of bounds");

	if (colSpan < 1)
        colSpan = 1;

	if (rowSpan < 1)
        rowSpan = 1;

	RectI newRect(0,0,0,0);

	for(int i = 0; i < col; i++)
	{
		newRect.point.x += mColSizes[i];
	}

	for(int i =col; i < col+colSpan; i++)
	{
		newRect.extent.x += mColSizes[i];
	}
	
	for(int i = 0; i < row; i++)
	{
		newRect.point.y += mRowSizes[i];
	}

	for(int i =row; i < row+rowSpan; i++)
	{
		newRect.extent.y += mRowSizes[i];
	}

	return newRect;
}

//------------------------------------------------------------------------------

void GuiGridControl::AdjustGrid(const Point2I& newExtent)
{
	mColSizes.clear();
	mRowSizes.clear();
	AdjustGridItems(newExtent.x, mGridCols, mColSizes);
	AdjustGridItems(newExtent.y, mGridRows, mRowSizes);
}

//------------------------------------------------------------------------------

void GuiGridControl::AdjustGridItems(S32 size, Vector<StringTableEntry>& strItems, Vector<S32>& items)
{
	Vector<GridItem> GridItems;
	S32 bFoundStar = false;
	S32 IndexRemaining = -1;
	S32 totalSize = 0;
	S32 idx =0;

	//First step : Convert the string based column data into a GridItem vector.
	for(auto col = strItems.begin(); col != strItems.end(); ++col, idx++)
	{
		StringTableEntry str = *col;

		auto len = dStrlen(str);
		AssertFatal(len >= 1, "Item can not be blank.");
		
		//we support three types of values (absolute size in pixels, percentage based, and remaining size in pixels).
		if (str[0] == '*') // use the remaining space left in the columns.
		{
			AssertFatal(!bFoundStar, "Can only use one * item field");
			GridItem gi;
			gi.IsAbsolute = false;
			gi.IsPercentage = false;
			gi.IsRemaining = true;
			gi.Size = 0;
			GridItems.push_back(gi);
		}
		else if ( len > 1 && str[len-1] == '%' ) //percentage based
		{
			char* tmp = new char[len-1];
			dStrncpy(tmp, str, len-1);
			int perc = dAtoi(tmp);
			delete tmp;

			GridItem gi;
			gi.IsAbsolute = false;
			gi.IsPercentage = true;
			gi.IsRemaining = false;
			gi.Size = perc;
			GridItems.push_back(gi);

		}
		else //standard absolute pixel based
		{
			int px = dAtoi(str);

			GridItem gi;
			gi.IsAbsolute = true;
			gi.IsPercentage = false;
			gi.IsRemaining = false;
			gi.Size = px;
			GridItems.push_back(gi);

			totalSize += px;
		}
	}

    //step two: iterate the grid columns again, and fill in any percentage based sizing, and setup the correct grid array.
	int remainingSize = size - totalSize;
	int sizeForPerc = remainingSize;
	for(int i = 0; i < GridItems.size(); ++i)
	{
		GridItem gi = GridItems[i];

		if (gi.IsAbsolute)
		{
			items.push_back(gi.Size);
		}
		else if (gi.IsPercentage)
		{
			F32 perc = gi.Size / 100.0f;
			S32 realSize = sizeForPerc * (S32)perc;
			remainingSize -= realSize;
			items.push_back(realSize);
		}
		else if(gi.IsRemaining)
		{
			//place holder for the moment.
			items.push_back(0);
			IndexRemaining = i;
		}			
	}

	if (IndexRemaining >= 0)
	{
		items[IndexRemaining] = remainingSize;
		remainingSize = 0;
	}
}
