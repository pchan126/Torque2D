ConsoleMethod( GuiInspector, inspect, void, 3, 3, "(obj) Goes through the object's fields and autogenerates editor boxes\n"
"@return No return value.")
{
    SimObject * target = Sim::findObject(argv[2]);
    if(!target)
    {
        if(dAtoi(argv[2]) > 0)
        Con::warnf("%s::inspect(): invalid object: %s", argv[0], argv[2]);

        object->clearGroups();
        return;
    }

    object->inspectObject(target);
}


ConsoleMethod( GuiInspector, getInspectObject, const char*, 2, 2, "() - Returns currently inspected object\n"
"@return The Object's ID as a string.")
{
    SimObject *pSimObject = object->getInspectObject();
    if( pSimObject != nullptr )
    return pSimObject->getIdString();

    return "";
}

ConsoleMethod( GuiInspectorDynamicField, renameField, void, 3,3, "field.renameField(newDynamicFieldName);" )
{
    object->renameField( StringTable->insert(argv[2]) );
}


ConsoleMethod( GuiInspector, setName, void, 3, 3, "(NewObjectName) Set object name.\n"
        "@return No return value.")
{
    object->setName(argv[2]);
}


ConsoleMethod( GuiInspectorField, apply, void, 3,3, "(newValue) Applies the given value to the field\n"
        "@return No return value." )
{
    object->setData( argv[2] );
}

ConsoleMethod(GuiInspectorDynamicGroup, inspectGroup, bool, 2, 2, "() Refreshes the dynamic fields in the inspector.\n"
        "@return Returns true on success.")
{
    return object->inspectGroup();
}


ConsoleMethod( GuiInspectorDynamicGroup, addDynamicField, void, 2, 2, "obj.addDynamicField();" )
{
    object->addDynamicField();
}
