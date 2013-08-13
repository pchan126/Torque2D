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

function GuiToy::create( %this )
{
    // Reset the toy.    
    GuiToy.reset();
}

//-----------------------------------------------------------------------------

function GuiToy::destroy( %this )
{
}

//-----------------------------------------------------------------------------

function GuiToy::reset( %this )
{
    // Clear the scene.
    SandboxScene.clear();
    %text = "Hoody Hoo";
    %labelExtent = "50 10";
	GuiToy.joystick1 = new GuiMoveCtrl(moveCtrl1)
	{
		Profile = SandboxWindowProfile;
		Position = "-900 100";
		Extent = "1000 150";
       isContainer = "1";

    };

  new GuiSpriteCtrl(textBG1)
  {
    Profile="GuiToolboxProfile";
    HorizSizing="width";
    VertSizing="height";
    Position="0 0";
    Extent="550 120";
    MinExtent="8 8";
    Visible="1";
	HelpTag="0";
	Image="ToyAssets:textBGD1";
	};

        new GuiTextCtrl(text1)
            {
                text = %text;
                Extent = "400 80";
                HorizSizing = "relative";
                VertSizing = "relative";
                Profile = "GuiRightJustifyProfile";
                canSaveDynamicFields = "0";
                isContainer = "0";
                Position = "100 20";
                MinExtent = "8 2";
                canSave = "0";
                Visible = "1";
                Active = "0";
                tooltipprofile = "GuiToolTipProfile";
                tooltipWidth = "0";
                maxLength = "255";
                truncate = "0";
            };

    SandboxWindow.add( moveCtrl1 );
    moveCtrl1.add( textBG1 );
    moveCtrl1.add( text1 );
//    text1.resizeWidthToText();
//    text1.startTimer("onTimer", 500, 0);
    text1.count = 0;
	GuiToy.joystick1.setTargetPosition("-50 100");
	GuiToy.joystick1.startMove(1.0);
	GuiToy.joystick1.moveStatus = 1;
    MainOverlay.setVisible(0);
}

//function text1::onTimer(%this)
//{
//   %this.count++;
//   %this.setText(%this.count);
////   %this.resizeWidthToText();
//}


function moveCtrl1::onMoveToComplete(%this)
{
   if (%this.moveStatus == 1)
   {
      text1.count++;
      text1.setText(text1.count);
      echo (text1.extent.y);
   }

	%this.moveStatus = -%this.moveStatus;
	%pos = %this.getPosition();

	%this.setTargetPosition(%pos.x+(900*%this.moveStatus), %pos.y );
   %this.schedule( 1000, "startMove");
//	%this.startMove();

}