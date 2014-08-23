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

ConsoleMethodGroupBeginWithDocs(Sprite, SpriteBase)

/*! Sets the sprite texture flipping for each axis.
    @param flipX Whether or not to flip the texture along the x (horizontal) axis.
    @param flipY Whether or not to flip the texture along the y (vertical) axis.
    @return No return value.
*/
ConsoleMethodWithDocs(Sprite, setFlip, ConsoleVoid, 4, 4, (bool flipX, bool flipY))
{
    // Set Flip.
    object->setFlip( dAtob(argv[2]), dAtob(argv[3]) );
}

//-----------------------------------------------------------------------------

/*! Gets the flip for each axis.
    @return (bool flipX/bool flipY) Whether or not the texture is flipped along the x and y axis.
*/
ConsoleMethodWithDocs(Sprite, getFlip, ConsoleString, 2, 2, ())
{
    // Create Returnable Buffer.
    char* pBuffer = Con::getReturnBuffer(32);

    // Format Buffer.
    dSprintf(pBuffer, 32, "%d %d", object->getFlipX(), object->getFlipY());

    // Return Buffer.
    return pBuffer;
}

//-----------------------------------------------------------------------------

/*! Sets whether or not the texture is flipped horizontally.
    @param flipX Whether or not to flip the texture along the x (horizontal) axis.
    @return No return value.
*/
ConsoleMethodWithDocs(Sprite, setFlipX, ConsoleVoid, 3, 3, (bool flipX))
{
    // Set Flip.
    object->setFlipX( dAtob(argv[2]) );
}

//-----------------------------------------------------------------------------

/*! Sets whether or not the texture is flipped vertically.
    @param flipY Whether or not to flip the texture along the y (vertical) axis.
    @return No return value.
*/
ConsoleMethodWithDocs(Sprite, setFlipY, ConsoleVoid, 3, 3, (bool flipY))
{
    // Set Flip.
    object->setFlipY( dAtob(argv[2]) );
}

//-----------------------------------------------------------------------------

/*! Gets whether or not the texture is flipped horizontally.
    @return (bool flipX) Whether or not the texture is flipped along the x axis.
*/
ConsoleMethodWithDocs(Sprite, getFlipX, ConsoleBool, 2, 2, ())
{
   return object->getFlipX();
}

//-----------------------------------------------------------------------------

/*! Gets whether or not the texture is flipped vertically.
    @return (bool flipY) Whether or not the texture is flipped along the y axis.
*/
ConsoleMethodWithDocs(Sprite, getFlipY, ConsoleBool, 2, 2, ())
{
   return object->getFlipY();
}



//-----------------------------------------------------------------------------

ConsoleMethod(Sprite, setRows, void, 3, 3,     "(int rows) Sets the number of rows in the sprite mesh.\n"
              "@param rows The number of rows in the mesh."
              "@return No return value.")
{
    // Set Flip.
    object->setRows( dAtoi(argv[2]) );
}

//-----------------------------------------------------------------------------

ConsoleMethod(Sprite, setColumns, void, 3, 3,     "(int columns) Sets the number of columns to render.\n"
              "@param columns The number of columns in the mesh."
              "@return No return value.")
{
    // Set Flip.
    object->setColumns( dAtoi(argv[2]) );
}

//-----------------------------------------------------------------------------

ConsoleMethod(Sprite, getRows, S32, 2, 2,     "() Gets the number of rows in the sprites rendering mesh.\n"
              "@return (int rows) the number of rows that are rendered.")
{
    return object->getRows();
}

//-----------------------------------------------------------------------------

ConsoleMethod(Sprite, getColumns, S32, 2, 2,     "() Gets the number columns in the sprites rendering mesh."
              "@return (int columns) the number of columns that are rendered.")
{
    return object->getColumns();
}

ConsoleMethod(Sprite, setNode, void, 5, 5, "(obj1,row, column) Adds given list of objects to the SceneObjectSet.\n"
        "@param obj, row, column \n"
        "@return No return value")
{
    SceneObject *obj = Sim::findObject<SceneObject>(argv[2]);
    if(obj)
        object->setNode(obj, dAtoi(argv[3]), dAtoi(argv[4]));
    else
        Con::printf("Set::add: Object \"%s\" doesn't exist", argv[2]);
}


ConsoleMethodGroupEndWithDocs(Sprite)
