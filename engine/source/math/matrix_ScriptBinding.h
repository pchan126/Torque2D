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

ConsoleFunctionGroupBegin(MatrixMath, "Matrix manipulation functions.");

/*! @defgroup MatrixMathFunctions Matrix Math
	@ingroup TorqueScriptFunctions
	@{
*/

/*! Use the matrixCreate function to create a transform matrix from a three-element floating-point translation vector and a four-element floating-point rotation vector.
    @param posVec A three-element floating-point translation vector: \PosX PosY PosZ\.
    @param rotVec A four-element floating-point rotation vector: \RotX RotY RotZ\.
    @param These re rotations about the specified axes.
    @return Returns a transform matrix of the form \PosX PosY PosZ RotX RotY RotZ theta\.
    @sa MatrixCreateFromEuler
*/
ConsoleFunctionWithDocs( MatrixCreate, ConsoleString, 3, 3, ( posVec , rotVec ))
{
   Point3F pos;
   Point3F axis;
   F32     angle;
   
   dSscanf(argv[1], "%g %g %g", &pos.x, &pos.y, &pos.z);
   dSscanf(argv[2], "%g %g %g %g", &axis.x, &axis.y, &axis.z, &angle);
   angle = mDegToRad(angle);
   
   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer, 255, "%g %g %g %g %g %g %g",
            pos.x, pos.y, pos.z,
            axis.x, axis.y, axis.z,
            angle);
   return returnBuffer;

}

/*! Use the MatrixCreateFromEuler function to calculate a transform matrix from a three-element floating-point rotation vector.
    @param rotVec A three-element floating-point rotation vector: \RotX RotY RotZ\. These are rotations about the specified axes.
    @return Returns a transform matrix of the form \0 0 0 X Y Z theta\.
    @sa MatrixCreate
*/
ConsoleFunctionWithDocs( MatrixCreateFromEuler, ConsoleString, 2, 2,  ( rotVec ))
{
   EulerF rot;
   dSscanf(argv[1], "%g %g %g", &rot.x, &rot.y, &rot.z);
   
   QuatF rotQ(rot);
   
   Point3F axis = rotQ.getAxis();
   F32     angle = rotQ.getAngle();
   
   char* ret = Con::getReturnBuffer(256);
   dSprintf(ret, 255, "0 0 0 %g %g %g %g",axis.x,axis.y,axis.z,angle);
   return ret;

}


/*! Use the MatrixMultiply function to multiply two seven-element transform matrices to produce a new seven element matrix.
    @param transformA A seven-element transform matrix of the form \PosX PosY PosZ RotX RotY RotZ theta\.
    @param transformB A seven-element transform matrix of the form \PosX PosY PosZ RotX RotY RotZ theta\.
    @return Returns a seven-element matrix resulting from transiformA x transformB.
    @sa MatrixMulPoint, MatrixMulVector
*/
ConsoleFunctionWithDocs( MatrixMultiply, ConsoleString, 3, 3, ( transformA , transformB ))
{
   Point3F pos1;
   Point3F axis1;
   F32 angle1;
   dSscanf(argv[1], "%g %g %g %g %g %g %g", &pos1.x, &pos1.y, &pos1.z, &axis1.x, &axis1.y, &axis1.z, &angle1);
   
   DualQuatF temp1(QuatF(axis1, angle1), pos1);
   
   Point3F pos2;
   Point3F axis2;
   F32 angle2;
   dSscanf(argv[1], "%g %g %g %g %g %g %g", &pos2.x, &pos2.y, &pos2.z, &axis2.x, &axis2.y, &axis2.z, &angle2);
   
   DualQuatF temp2(QuatF(axis2, angle2), pos2);
   
   temp1*=temp2;
   
   Point3F pos = temp1.getTranslation();
   Point3F axis = temp1.getRotation().getAxis();
   F32 angle = temp1.getRotation().getAngle();
   
   char* ret = Con::getReturnBuffer(256);
   dSprintf(ret, 255, "%g %g %g %g %g %g %g",
            pos.x, pos.y, pos.z,
            axis.x, axis.y, axis.z,
            angle);
   return ret;

}


/*! Use the MatrixMulVector function to multiply a seven-element transform matrix with a three-element matrix.
    @param transform A seven-element transform matrix of the form \PosX PosY PosZ RotX RotY RotZ theta\.
    @param vector A three-element vector.
    @return Returns three-element resulting from vector * transform.
    @sa MatrixMulPoint, MatrixMultiply
*/
ConsoleFunctionWithDocs( MatrixMulVector, ConsoleString, 3, 3, ( transform , vector ))
{
   Point3F pos1;
   Point3F axis1;
   F32 angle1;
   dSscanf(argv[1], "%g %g %g %g %g %g %g", &pos1.x, &pos1.y, &pos1.z, &axis1.x, &axis1.y, &axis1.z, &angle1);
   
   DualQuatF temp1(QuatF(axis1, angle1), pos1);
   MatrixF   mat1 = temp1.toMatrix();
   
   Point3F vec1;
   dSscanf(argv[2], "%g %g %g", &vec1.x, &vec1.y, &vec1.z);
   
   Point3F result;
   mat1.mulV(vec1, &result);
   
   char* ret = Con::getReturnBuffer(256);
   dSprintf(ret, 255, "%g %g %g", result.x, result.y, result.z);
   return ret;

}

/*! Use the MatrixMulPoint function to multiply a seven element transform matrix by a three element point vector, producing a three element position vector.
    @param transform A seven-element transform matrix.
    @param point A three-element point/position vector.
    @return Returns a three-element position vector.
    @sa MatrixMultiply, MatrixMulVector
*/
ConsoleFunctionWithDocs( MatrixMulPoint, ConsoleString, 3, 3, ( transform , point ))
{
   Point3F pos1;
   Point3F axis1;
   F32 angle1;
   dSscanf(argv[1], "%g %g %g %g %g %g %g", &pos1.x, &pos1.y, &pos1.z, &axis1.x, &axis1.y, &axis1.z, &angle1);
   
   DualQuatF temp1(QuatF(axis1, angle1), pos1);
   MatrixF   mat1 = temp1.toMatrix();
   
   Point3F vec1;
   dSscanf(argv[2], "%g %g %g", &vec1.x, &vec1.y, &vec1.z);
   
   Point3F result;
   mat1.mulP(vec1, &result);
   
   char* ret = Con::getReturnBuffer(256);
   dSprintf(ret, 255, "%g %g %g", result.x, result.y, result.z);
   return ret;

}

ConsoleFunctionGroupEnd(MatrixMath);

/*! @} */ // group MatrixMathFunctions
