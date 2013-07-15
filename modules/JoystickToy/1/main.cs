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

function joypad_leftstickx(%value)  
{  
    JoystickToy.stick1.x = %value;
    if (VectorLen(JoystickToy.stick1.x SPC JoystickToy.stick1.y SPC 0) < 1)
    	%vec = (JoystickToy.stick1.x SPC JoystickToy.stick1.y SPC 0);
    else
	    %vec = VectorNormalize( JoystickToy.stick1.x SPC JoystickToy.stick1.y SPC 0);

	JoystickToy.stick1.setPosition(%vec.x*2 -3, %vec.y*2 -3);
}

function joypad_leftsticky(%value)
{  
    JoystickToy.stick1.y = -%value;
    if (VectorLen(JoystickToy.stick1.x SPC JoystickToy.stick1.y SPC 0) < 1)
    	%vec = (JoystickToy.stick1.x SPC JoystickToy.stick1.y SPC 0);
    else
	    %vec = VectorNormalize( JoystickToy.stick1.x SPC JoystickToy.stick1.y SPC 0);
	JoystickToy.stick1.setPosition(%vec.x*2 -3, %vec.y*2 -3);
}

function joypad_rightstickx(%value)  
{  
    JoystickToy.stick2.x = %value;
    if (VectorLen(JoystickToy.stick2.x SPC JoystickToy.stick2.y SPC 0) < 1)
    	%vec = (JoystickToy.stick2.x SPC JoystickToy.stick2.y SPC 0);
    else
	    %vec = VectorNormalize( JoystickToy.stick2.x SPC JoystickToy.stick2.y SPC 0);
	JoystickToy.stick2.setPosition(%vec.x*2 +3, %vec.y*2 -3);
}  

function joypad_rightsticky(%value)
{  
    JoystickToy.stick2.y = -%value;
    if (VectorLen(JoystickToy.stick2.x SPC JoystickToy.stick2.y SPC 0) < 1)
    	%vec = (JoystickToy.stick2.x SPC JoystickToy.stick2.y SPC 0);
    else
	    %vec = VectorNormalize( JoystickToy.stick2.x SPC JoystickToy.stick2.y SPC 0);
	JoystickToy.stick2.setPosition(%vec.x*2 +3, %vec.y*2 -3);
}  

function joypad_button0(%value)
{
	if (%value == 1)
	    JoystickToy.circle0.BlendColor = Blue;
	else
	    JoystickToy.circle0.BlendColor = Black;
}

function joypad_button1(%value)
{
	if (%value == 1)
	    JoystickToy.circle1.BlendColor = Green;
	else
	    JoystickToy.circle1.BlendColor = Black;
}

function joypad_button2(%value)
{
	if (%value == 1)
	    JoystickToy.circle2.BlendColor = Red;
	else
	    JoystickToy.circle2.BlendColor = Black;
}

function joypad_button3(%value)
{
	if (%value == 1)
	    JoystickToy.circle3.BlendColor = Yellow;
	else
	    JoystickToy.circle3.BlendColor = Black;
}



function JoystickToy::create( %this )
{
    JoystickToy.CameraWidth = 20;
    JoystickToy.CameraHeight = 15;
    JoystickToy.BackdropDomain = 31;

   GlobalActionMap.bind(joystick, xaxis, "joypad_leftstickx");
   GlobalActionMap.bind(joystick, yaxis, "joypad_leftsticky");
   GlobalActionMap.bind(joystick, zaxis, "joypad_rightstickx");
   GlobalActionMap.bind(joystick, rzaxis, "joypad_rightsticky");

   GlobalActionMap.bind(joystick, button0, "joypad_button0");
   GlobalActionMap.bind(joystick, button1, "joypad_button1");
   GlobalActionMap.bind(joystick, button2, "joypad_button2");
   GlobalActionMap.bind(joystick, button3, "joypad_button3");


    if ( $platform $= "iOS" )
    {
        JoystickToy.joystick1 = new GuiJoystickCtrl()
        {
            Profile = SandboxWindowProfile;
            Position = "0 0";
            Extent = "480 600";
        };
        JoystickToy.joystick2 = new GuiJoystickCtrl()
        {
            Profile = SandboxWindowProfile;
            Position = "0 0";
            Extent = "480 600";
        };
        // Add it as a child window.
        SandboxWindow.add( JoystickToy.joystick1 );
        SandboxWindow.add( JoystickToy.joystick2 );

        // Add window to the toy so it is destroyed.
        JoystickToy.add( Joystick.joystick1 );
        JoystickToy.add( JoystickToy.joystick2 );

        %extent = SandboxWindow.getExtent();
        JoystickToy.joystick1.setExtent(%extent.x/2, %extent.y-40);
        JoystickToy.joystick1.Xevent = "xaxis";
        JoystickToy.joystick1.Yevent = "yaxis";
        JoystickToy.joystick1.CircleImage = "ToyAssets:Circle1";
        JoystickToy.joystick1.StickImage = "ToyAssets:Circle2";

        JoystickToy.joystick2.setExtent(%extent.x/2, %extent.y-40);
        JoystickToy.joystick2.setPosition(%extent.x/2, 0);
        JoystickToy.joystick2.Xevent = "zaxis";
        JoystickToy.joystick2.Yevent = "rzaxis";
        JoystickToy.joystick2.CircleImage = "ToyAssets:Circle1";
        JoystickToy.joystick2.StickImage = "ToyAssets:Circle2";
    }
	JoystickToy.reset();
}

//-----------------------------------------------------------------------------

function JoystickToy::destroy( %this )
{
    // Deactivate the package.
    deactivatePackage( JoystickToyPackage );
}

//-----------------------------------------------------------------------------

function JoystickToy::reset( %this )
{
    // Clear the scene.
    SandboxScene.clear();    
    
    // Set a typical Earth gravity.
    SandboxScene.setGravity( 0, 0 );
    
    SandboxScene.setAmbientLight( 1.0, 1.0, 1.0, 1.0);

    // Camera Configuration
    SandboxWindow.setCameraPosition( 0, 0 );
    SandboxWindow.setCameraAngle( 0 );
    SandboxWindow.setCameraSize( JoystickToy.CameraWidth, JoystickToy.CameraHeight );

    // Create the scene contents in a roughly left to right order.

    // Background.
    %this.createBackground();
}

// -----------------------------------------------------------------------------

function JoystickToy::createBackground(%this)
{  
    %obj = new Sprite(circle1);
    %obj.setBodyType( "static" );
    %obj.setImage( "ToyAssets:circle1" );
    %obj.BlendColor = White;
    %obj.setSize( 4, 4 );
    %obj.setPosition(-3, -3);
    %obj.setSceneLayer( JoystickToy.BackdropDomain );
    %obj.setSceneGroup( JoystickToy.BackdropDomain );
    %obj.setCollisionSuppress();
    %obj.setAwake( false );
    %obj.setActive( false );
    SandboxScene.add( %obj );
    JoystickToy.circle1 = %obj;

    %obj = new Sprite(circle2);
    %obj.setBodyType( "static" );
    %obj.setImage( "ToyAssets:circle1" );
    %obj.BlendColor = White;
    %obj.setSize( 4, 4 );
    %obj.setPosition(3, -3);
    %obj.setSceneLayer( JoystickToy.BackdropDomain );
    %obj.setSceneGroup( JoystickToy.BackdropDomain );
    %obj.setCollisionSuppress();
    %obj.setAwake( false );
    %obj.setActive( false );
    SandboxScene.add( %obj );
    JoystickToy.circle2 = %obj;

    %obj = new Sprite(stick1);
    %obj.setBodyType( "static" );
    %obj.setImage( "ToyAssets:circle2" );
    %obj.BlendColor = White;
    %obj.setSize( 2, 2 );
    %obj.setPosition(-3, -3);
    %obj.setSceneLayer( JoystickToy.BackdropDomain-1);
    %obj.setSceneGroup( JoystickToy.BackdropDomain );
    %obj.setCollisionSuppress();
    %obj.setAwake( false );
    %obj.setActive( false );
    SandboxScene.add( %obj );
    JoystickToy.stick1 = %obj;

    %obj = new Sprite(stick2);
    %obj.setBodyType( "static" );
    %obj.setImage( "ToyAssets:circle2" );
    %obj.BlendColor = White;
    %obj.setSize( 2, 2 );
    %obj.setPosition(3, -3);
    %obj.setSceneLayer( JoystickToy.BackdropDomain-1);
    %obj.setSceneGroup( JoystickToy.BackdropDomain );
    %obj.setCollisionSuppress();
    %obj.setAwake( false );
    %obj.setActive( false );
    SandboxScene.add( %obj );
    JoystickToy.stick2 = %obj;
    
    %obj = new Sprite(circle0);
    %obj.setBodyType( "static" );
    %obj.setImage( "ToyAssets:BlankCircle" );
    %obj.BlendColor = Black;
    %obj.setSize( 1, 1 );
    %obj.setPosition(3, 2);
    %obj.setSceneLayer( JoystickToy.BackdropDomain-1);
    %obj.setSceneGroup( JoystickToy.BackdropDomain );
    %obj.setCollisionSuppress();
    %obj.setAwake( false );
    %obj.setActive( false );
    SandboxScene.add( %obj );
    JoystickToy.circle0 = %obj;

    %obj = new Sprite(circle1);
    %obj.setBodyType( "static" );
    %obj.setImage( "ToyAssets:BlankCircle" );
    %obj.BlendColor = Black;
    %obj.setSize( 1, 1 );
    %obj.setPosition(4, 1);
    %obj.setSceneLayer( JoystickToy.BackdropDomain-1);
    %obj.setSceneGroup( JoystickToy.BackdropDomain );
    %obj.setCollisionSuppress();
    %obj.setAwake( false );
    %obj.setActive( false );
    SandboxScene.add( %obj );
    JoystickToy.circle1 = %obj;

    %obj = new Sprite(circle2);
    %obj.setBodyType( "static" );
    %obj.setImage( "ToyAssets:BlankCircle" );
    %obj.BlendColor = Black;
    %obj.setSize( 1, 1 );
    %obj.setPosition(5, 2);
    %obj.setSceneLayer( JoystickToy.BackdropDomain-1);
    %obj.setSceneGroup( JoystickToy.BackdropDomain );
    %obj.setCollisionSuppress();
    %obj.setAwake( false );
    %obj.setActive( false );
    SandboxScene.add( %obj );
    JoystickToy.circle2 = %obj;

    %obj = new Sprite(circle3);
    %obj.setBodyType( "static" );
    %obj.setImage( "ToyAssets:BlankCircle" );
    %obj.BlendColor = Black;
    %obj.setSize( 1, 1 );
    %obj.setPosition(4, 3);
    %obj.setSceneLayer( JoystickToy.BackdropDomain-1);
    %obj.setSceneGroup( JoystickToy.BackdropDomain );
    %obj.setCollisionSuppress();
    %obj.setAwake( false );
    %obj.setActive( false );
    SandboxScene.add( %obj );
    JoystickToy.circle3 = %obj;
}



