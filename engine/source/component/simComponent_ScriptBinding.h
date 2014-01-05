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

//////////////////////////////////////////////////////////////////////////
// Console Methods
//////////////////////////////////////////////////////////////////////////

ConsoleMethod( SimComponent, addComponents, bool, 3, 64, "%obj.addComponents( %compObjName, %compObjName2, ... );\n"
"Adds additional components to current list.\n"
"@param Up to 62 component names\n"
"@return Returns true on success, false otherwise.")
{
    for(S32 i = 2; i < argc; i++)
    {
        SimComponent *obj = dynamic_cast<SimComponent*> (Sim::findObject(argv[i]) );
        if(obj)
            object->addComponent(obj);
        else
            Con::printf("SimComponent::addComponents - Invalid Component Object \"%s\"", argv[i]);
    }
    return true;
}

ConsoleMethod( SimComponent, removeComponents, bool, 3, 64, "%obj.removeComponents( %compObjName, %compObjName2, ... );\n"
                                                            "Removes components by name from current list.\n"
                                                            "@param objNamex Up to 62 component names\n"
                                                            "@return Returns true on success, false otherwise.")
{
    for(S32 i = 2; i < argc; i++)
    {
        SimComponent *obj = dynamic_cast<SimComponent*> (Sim::findObject(argv[i]) );
        if(obj)
            object->removeComponent(obj);
        else
            Con::printf("SimComponent::removeComponents - Invalid Component Object \"%s\"", argv[i]);
    }
    return true;
}

ConsoleMethod( SimComponent, getComponentCount, S32, 2, 2, "() Get the current component count\n"
                                                            "@return The number of components in the list as an integer")
{
    return (S32)object->getComponentCount();
}

ConsoleMethod( SimComponent, getComponent, S32, 3, 3, "(idx) Get the component corresponding to the given index.\n"
                                                    "@param idx An integer index value corresponding to the desired component.\n"
                                                    "@return The id of the component at the given index as an integer")
{
    S32 idx = dAtoi(argv[2]);
    if(idx < 0 || idx >= (S32)object->getComponentCount())
    {
        Con::errorf("SimComponent::getComponent - Invalid index %d", idx);
        return 0;
    }

    SimComponent *c = object->getComponent(idx);
    return c ? c->getId() : 0;
}

ConsoleMethod(SimComponent, setEnabled, void, 3, 3, "(enabled) Sets or unsets the enabled flag\n"
"@param enabled Boolean value\n"
"@return No return value")
{
    object->setEnabled(dAtob(argv[2]));
}

ConsoleMethod(SimComponent, isEnabled, bool, 2, 2, "() Check whether SimComponent is currently enabled\n"
"@return true if enabled and false if not")
{
    return object->isEnabled();
}


