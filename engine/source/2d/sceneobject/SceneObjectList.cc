//-----------------------------------------------------------------------------
// Copyright (c) 2013 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include "sceneObjectList.h"
#include "2d/sceneObject/sceneObject.h"

//-----------------------------------------------------------------------------

void SceneObjectList::pushBack(SceneObject* obj)
{
	if (std::find(begin(),end(),obj) == end())
		push_back(obj);
}	

//-----------------------------------------------------------------------------

void SceneObjectList::pushBackForce(SceneObject* obj)
{
	iterator itr = std::find(begin(),end(),obj);
	if (itr == end()) 
	{
		push_back(obj);
	}
	else 
	{
		// Move to the back...
		SceneObject* pBack = *itr;
		removeStable(pBack);
		push_back(pBack);
	}
}	

//-----------------------------------------------------------------------------

void SceneObjectList::remove(SceneObject* obj)
{
	iterator ptr = std::find(begin(),end(),obj);
	if (ptr != end()) 
		erase(ptr);
}

//-----------------------------------------------------------------------------

void SceneObjectList::removeStable(SceneObject* obj)
{
	iterator ptr = std::find(begin(),end(),obj);
	if (ptr != end()) 
		erase(ptr);
}

//-----------------------------------------------------------------------------

void SceneObjectList::sortId()
{
    std::sort(begin(), end(), compareId);
}	

//-----------------------------------------------------------------------------

bool SceneObjectList::compareId(const SceneObject* a, const SceneObject* b)
{
    return a->getId() < b->getId();
}
