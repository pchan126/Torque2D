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

//function joypad_gui_leftstickx(%value)
//{  
////   echo("Joystick_gui_leftstick X" SPC %value);
//    GuiToy.xPos = %value;
//}
//
//function joypad_gui_leftsticky(%value)
//{  
////   echo("Joystick_gui_leftstick Y" SPC %value);
//    GuiToy.yPos = %value;
//}
//
//function joypad_gui_rightstickx(%value)
//{  
////   echo("Joystick_rightstick X" SPC %value);
//    GuiToy.rxPos = %value;
//}
//
//function joypad_gui_rightsticky(%value)
//{  
////   echo("Joystick_rightstick Y" SPC %value);
//    GuiToy.ryPos = %value;
//}

//function joypad_button0(%value)
//{
//   echo("joypad_button0" SPC %value);
//}
//
//function joypad_button1(%value)
//{
//   echo("joypad_button1" SPC %value);
//}

function GuiToy::create( %this )
{
    GuiToy.BackdropDomain = 31;
    GuiToy.BackgroundDomain = 25;
    GuiToy.ShipDomain = 20;
    GuiToy.GroundDomain = 18;
    GuiToy.ObstacleDomain = 15;
    GuiToy.ProjectileDomain = 16;
    GuiToy.TriggerDomain = 17;
    GuiToy.ForegroundDomain = 10;

    GuiToy.rotateSpeed = 180;
    GuiToy.maxBallSpeed = 20;
    GuiToy.ballSpeed = 5;

    // Reset the toy.
    GuiToy.reset();

    // Start the timer.
    GuiToy.startTimer( "updateShip", 50 );
}

function GuiToy::updateShip(%this)
{
   %shipPosition = Ship.getPosition();
//   echo ("updateShip" SPC %shipPosition);
//    %prog = ProgressBar.getValue();
//    if (%prog == 1.0)
//      %prog = 0.0;
//    else
//      %prog += 0.01;
//
//    ProgressBar.setValue(%prog);
//    ProgressBar.text = "Shields" SPC mRound(%prog*100) SPC "%";
//
//
//    %origin = -GuiToy.xPos SPC GuiToy.yPos;
//    Ship.applyForce( -%origin.x*200, -%origin.y*200 );
//
//    %polarVel = Ship.getLinearVelocityPolar();
//
//    if (%polarVel.y > 20.0)
//       Ship.setLinearVelocityPolar(%polarVel.x, 20.0);
//
//   if (Vector2Length(%origin) > 0.1)
//   {
//       %angle = mAtan( Vector2Sub( "0 0", %origin ) ) - 90;
//       Ship.RotateTo( %angle, GuiToy.rotateSpeed );
//
//       %adjustedSpeed = GuiToy.ballSpeed / GuiToy.maxBallSpeed;
//
//       %scaledVelocity = VectorScale(%relativePosition, %adjustedSpeed);
//    }
//
//    %origin2 = -GuiToy.rxPos SPC GuiToy.ryPos;
//
//   if (Vector2Length(%origin2) > 0.1)
//   {
////       echo("origin2" SPC %origin2);
//
//       %angle = mAtan( Vector2Sub( "0 0", %origin2 ) );
//       %position = Ship.getPosition();
//
//       %projectile = new Sprite()
//       {
//           Image = "GuiToy:pewpew";
//           position = %position;
//           size = "0.4 0.4";
//           SceneLayer = "2";
//           SceneGroup = "14";
//           minSpeed = "5";
//           maxSpeed = "15";
//           CollisionCallback = true;
//       };
//       %projectile.Lifetime = 3;
//       %projectile.setCollisionGroups( 16 );
//       %projectile.CollisionCallback = true;
//       %projectile.setLightType("CONSTLIGHT");
//       %projectile.setLinearVelocityPolar(%angle, 80.0);
//       SandboxScene.add( %projectile );
//   }
}

//-----------------------------------------------------------------------------

function GuiToy::makeMissile(%this, %worldPosition, %target)
{
   %shipPosition = %target.getPosition();

   %missile = new Sprite()
   {
      class = "rocket";
      Image = "GuiToy:rocket1";
      position = %worldPosition;
      size = "7 7";
      CollisionCallback = true;
      lifetime = "10";
      SceneLayer = "0";
   };
    %missile.target = %target;
    %missile.createCircleCollisionShape(1);
    %missile.setSceneGroup( GuiToy.ProjectileDomain );
    %missile.setCollisionGroups( GuiToy.ObstacleDomain, GuiToy.GroundDomain );
    %missile.setUpdateCallback(true);

    SandboxScene.add( %missile );

    // Attach the exhaust output to the truck body.
    %missile.EngineExhaust = %this.createBonfire( %worldPosition.x, %worldPosition.y, 1, 3 );
    %missile.EngineExhaust.setSceneGroup(20);
    %missile.EngineExhaust.setCollisionSuppress(true);
    %joint = SandboxScene.createWeldJoint( %missile, %missile.EngineExhaust, "0.0 -1.0", "0 0" );
//    %jointId = SandboxScene.createRevoluteJoint( %missile, %missile.EngineExhaust, "0.0 -1.0", "0 0" );
    SandboxScene.setRevoluteJointLimit( %jointId, 0, 0 );

    %missile.setPosition(%worldPosition);
    return %missile;
}

//-----------------------------------------------------------------------------

function GuiToy::onTouchDown(%this, %touchID, %worldPosition)
{
   %this.makeMissile( %worldPosition, Ship );
   %this.makeMissile(Vector2Add(%worldPosition, "0 4"), Ship);
   %this.makeMissile(Vector2Add(%worldPosition, "0 -4"), Ship);
   %this.makeMissile(Vector2Add(%worldPosition, "4 0"), Ship);
   %this.makeMissile(Vector2Add(%worldPosition, "-4 0"), Ship);
}

function rocket::onUpdate(%this)
{
   %shipPosition = Ship.getPosition();
   %angle = mAtan( Vector2Sub( %shipPosition, %this.getPosition() ) ) - 90;
   %this.setAngle(%angle);
   %this.setLinearVelocityPolar( %angle +90, GuiToy.maxBallSpeed*2 );
}

function rocket::onCollision(%this)
{
//   echo ("rocket::onCollision");
    Ship.shields -= 10;
    if (Ship.shields < 0)
       Ship.shields = 0;

    ProgressBar.setValue( Ship.shields / 100 );
    ProgressBar.text = "Shields" SPC ProgressBar.getValue()*100;
    %this.safedelete();
}

function rocket::onRemove(%this)
{
//   echo ("rocket::onremove");
   %this.EngineExhaust.delete();
}

function GuiToy::MoveShip(%this, %touchID, %worldPosition)
{
    %target = new Trigger()
    {
       class = "ShipTarget";
       position = %worldPosition;  
       size = "5.4 5.4";
       EnterCallback = true;
    };
    %target.createCircleCollisionShape(1);
    %target.setCollisionShapeIsSensor(0, true);
    %target.setBodyType( "static" );
    %target.setSceneGroup( GuiToy.ObstacleDomain );
    %target.setAwake( true );
    %target.setActive( true );
    SandboxScene.add( %target );

   %shipPosition = Ship.getPosition();
   %angle = mAtan( Vector2Sub( %worldPosition, %shipPosition ) ) - 90;
   Ship.RotateTo( %angle, GuiToy.rotateSpeed );

    // Move the sight to the touched position.
    Ship.targetPosition = %worldPosition;
}


function Ship::onRotateToComplete( %this, %angle)
{
   Ship.setLinearVelocityPolar( %angle +90, GuiToy.maxBallSpeed );
}

function ShipTarget::onEnter(%this, %enter)
{
   Ship.setLinearVelocityPolar( 0, 0);
   %this.safeDelete();
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

   %this.createBackground();

//   %this.createJoysticks();

   GuiToy.ShipBody = %this.spawnShip("0 0");

//    // Attach the exhaust output to the truck body.   
//    GuiToy.EngineExhaust = %this.createBonfire( 0, 0, 1, 3 );
//    GuiToy.EngineExhaust.setSceneGroup(20);
//    GuiToy.EngineExhaust.setCollisionSuppress(true);
//    %joint = SandboxScene.createWeldJoint( GuiToy.ShipBody, GuiToy.EngineExhaust, "0.0 -1.8", "0 0" );
//    %jointId = SandboxScene.createRevoluteJoint( GuiToy.ShipBody, GuiToy.EngineExhaust, "0.0 -1.8", "0 0" );
//    SandboxScene.setRevoluteJointLimit( %jointId, 0, 0 );
//    //    SandboxScene.setRevoluteJointLimit( %joint, 0, 0 );
//    GuiToy.ShipBody.setCollisionGroups( 0 );

    SandboxWindow.mount( GuiToy.ShipBody, "0 0", 0, true, false );

    new GuiImageProgressCtrl(ProgressBar)
    {
      Profile="GuiProgressProfile";
      HorizSizing="width";
      VertSizing="height";
      Position="0 50";
      Extent="550 50";
      MinExtent="8 8";
      Visible="1";
      HelpTag="0";
      ProgressImage="GuiToy:redbar";
      text = %text;
      opaque= "0";
    };
    SandboxWindow.add( ProgressBar );
    
    Ship.shields = 100;
    ProgressBar.setValue( Ship.shields / 100 );
    ProgressBar.text = "Shields" SPC ProgressBar.getValue()*100;

    new GuiImageSlider(sliderProgressBar)
    {
      Profile="GuiSliderProfile";
      HorizSizing="width";
      VertSizing="height";
      Position="0 115";
      Extent="550 50";
      MinExtent="8 8";
      Visible="1";
      HelpTag="0";
      ProgressImage="GuiToy:redbar";
      text = %text;
      opaque= "0";
    };
    SandboxWindow.add( sliderProgressBar );

//  new GuiSpriteCtrl(textBG1)
//  {
//    Profile="GuiToolboxProfile";
//    HorizSizing="width";
//    VertSizing="height";
//    Position="0 0";
//    Extent="550 120";
//    MinExtent="8 8";
//    Visible="1";
//	HelpTag="0";
//	Image="ToyAssets:textBGD1";
//	};
//
//        new GuiTextCtrl(text1)
//            {
//                text = %text;
//                Extent = "400 80";
//                HorizSizing = "relative";
//                VertSizing = "relative";
//                Profile = "GuiRightJustifyProfile";
//                canSaveDynamicFields = "0";
//                isContainer = "0";
//                Position = "100 20";
//                MinExtent = "8 2";
//                canSave = "0";
//                Visible = "1";
//                Active = "0";
//                tooltipprofile = "GuiToolTipProfile";
//                tooltipWidth = "0";
//                maxLength = "255";
//                truncate = "0";
//            };
//
//    SandboxWindow.add( moveCtrl1 );
//    moveCtrl1.add( textBG1 );
//    moveCtrl1.add( text1 );
////    text1.resizeWidthToText();
////    text1.startTimer("onTimer", 500, 0);
//    text1.count = 0;
//	GuiToy.joystick1.setTargetPosition("-50 100");
//	GuiToy.joystick1.startMove(1.0);
//	GuiToy.joystick1.moveStatus = 1;
    MainOverlay.setVisible(0);
}

//function text1::onTimer(%this)
//{
//   %this.count++;
//   %this.setText(%this.count);
////   %this.resizeWidthToText();
//}

function GuiToy::spawnShip(%this, %position)
{
    %db = new Sprite(Ship)
    {
        Image = "GuiToy:ship1";
        position = %position;
        size = "10 10";
//        SceneLayer = "1";
//        SceneGroup = "14";
        minSpeed = "5";
        maxSpeed = "15";
        CollisionCallback = true;
        Angle = "180";
    };
    %db.setSceneLayer(0);
    %db.createCircleCollisionShape(2);
    %db.setSceneGroup( GuiToy.ObstacleDomain );
    %db.setCollisionGroups( GuiToy.ObstacleDomain, GuiToy.GroundDomain );

    %db.setLinearDamping( 0.5 );
    SandboxScene.add( %db );
    return %db;
}

//function GuiToy::createJoysticks(%this)
//{
//    if ( $platform $= "iOS" )
//    {
//        GuiToy.joystick1 = new GuiJoystickCtrl()
//        {
//            Profile = SandboxWindowProfile;
//            Position = "0 300";
//            Extent = "150 150";
//        };
//        GuiToy.joystick2 = new GuiJoystickCtrl()
//        {
//            Profile = SandboxWindowProfile;
//            Position = "0 0";
//            Extent = "480 600";
//        };
//
//        // Add it as a child window.
//        SandboxWindow.add( GuiToy.joystick1 );
//        SandboxWindow.add( GuiToy.joystick2 );
//
//        // Add window to the toy so it is destroyed.
//        GuiToy.add( GuiToy.joystick1 );
//        GuiToy.add( GuiToy.joystick2 );
//
//        GlobalActionMap.bind(joystick, xaxis, "joypad_gui_leftstickx");
//        GlobalActionMap.bind(joystick, yaxis, "joypad_gui_leftsticky");
//        GlobalActionMap.bind(joystick, zaxis, "joypad_gui_rightstickx");
//        GlobalActionMap.bind(joystick, rzaxis, "joypad_gui_rightsticky");
//
//        %extent = SandboxWindow.getExtent();
//
//        GuiToy.joystick1.setExtent(500,500);
//        GuiToy.joystick1.setPosition(0,%extent.y-40-GuiToy.joystick1.extent.y);
//        GuiToy.joystick1.Xevent = "xaxis";
//        GuiToy.joystick1.Yevent = "yaxis";
//        GuiToy.joystick1.CircleImage = "ToyAssets:Circle1";
//        GuiToy.joystick1.StickImage = "ToyAssets:Circle2";
//
//        GuiToy.joystick2.setExtent(500,500);
//        GuiToy.joystick2.setPosition(%extent.x-GuiToy.joystick2.extent.x,%extent.y-40-GuiToy.joystick2.extent.y);
//        GuiToy.joystick2.Xevent = "zaxis";
//        GuiToy.joystick2.Yevent = "rzaxis";
//        GuiToy.joystick2.CircleImage = "ToyAssets:Circle1";
//        GuiToy.joystick2.StickImage = "ToyAssets:Circle2";
//    }
//}

function GuiToy::createBackground(%this)
{
    %obj = new Sprite();
    %obj.setBodyType( "static" );
    %obj.setImage( "GuiToy:Nebula01bg" );
    %obj.setSize( 128, 128);
    %obj.setSceneLayer( 10 );
    %obj.setCollisionSuppress();
    %obj.setAwake( false );
    %obj.setActive( false );
    %obj.setRows(3);
    %obj.setColumns(3);
    SandboxScene.add( %obj );
    SandboxScene.setCameraPositionScale(10, 0.02, 0.02);
    
    %rock = new Sprite()
    {
        Image = "GuiToy:stone_9";
        position = "31 23";
        size = "24 24";
        SceneLayer = "4";
        SceneGroup = "14";
        CollisionCallback = true;
    };
    SandboxScene.add( %rock );

    %rock = new Sprite()
    {
        Image = "GuiToy:stone_10";
        position = "-2 23";
        size = "14 14";
        SceneLayer = "4";
        SceneGroup = "14";
        CollisionCallback = true;
    };
    SandboxScene.add( %rock );

    %rock = new Sprite()
    {
        Image = "GuiToy:stone_11";
        position = "11 3";
        size = "19 19";
        SceneLayer = "4";
        SceneGroup = "14";
        CollisionCallback = true;
    };
    SandboxScene.add( %rock );

    SandboxScene.setCameraPositionScale(4, 0.5, 0.5);
}

function GuiToy::createBonfire(%this, %x, %y, %scale, %layer)
{
    // Create an impact explosion at the projectiles position.
    %particlePlayer = new ParticlePlayer();
    %particlePlayer.SetPosition( %x, %y );
    %particlePlayer.SceneLayer = %layer;
    %particlePlayer.ParticleInterpolation = true;
    %particlePlayer.Particle = "ToyAssets:pointTest";
    %particlePlayer.SizeScale = %scale;
    %particlePlayer.CameraIdleDistance = GuiToy.CameraWidth * 0.8;
    %particlePlayer.setLightType("CONSTLIGHT");
    %particlePlayer.Angle = 180;
    SandboxScene.add( %particlePlayer );
    return %particlePlayer;
}

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