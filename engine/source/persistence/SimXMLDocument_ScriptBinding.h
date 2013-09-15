#ifndef _XMLDOC_SCRIPT_H_
#define _XMLDOC_SCRIPT_H_

#include "console/console.h"
#include "persistence/SimXMLDocument.h"

ConsoleMethod(SimXMLDocument, reset, void, 2, 2,
"() Set this to default state at construction.\n"
        "@return No return value")
{
    object->reset();
}

ConsoleMethod(SimXMLDocument, loadFile, S32, 3, 3, "(string fileName) Load file from given filename.\n"
        "@param fileName The name of the desired file\n"
        "@return Returns 1 on success and 0 otherwise")
{
    return object->loadFile( argv[2] );
}

ConsoleMethod(SimXMLDocument, saveFile, S32, 3, 3, "(string fileName) Save file to given filename.\n"
        "@param fileName A string presenting the filename to save the XML document as\n"
        "@return Returns 1 on success, and 0 otherwise")
{
    return object->saveFile( argv[2] );
}

ConsoleMethod(SimXMLDocument, parse, S32, 3, 3, "(string txtXML) Create document from XML string.\n"
        "@param txtXML The text of the XML document\n"
        "@return Returns 1 on success")
{
    return object->parse( argv[2] );
}

ConsoleMethod(SimXMLDocument, clear, void, 2, 2, "() Clear contents of XML document.\n"
        "@return No return value")
{
    object->clear();
}

ConsoleMethod(SimXMLDocument, clearError, void, 2, 2,
"() Clear error description.\n"
        "@return No return value")
{
    object->clearError();
}

ConsoleMethod(SimXMLDocument, getErrorDesc, const char*, 2, 2,
"() Get current error description.\n"
        "@return Returns a string with the error description")
{
    // Create Returnable Buffer (duration of error description is unknown).
    char* pBuffer = Con::getReturnBuffer(256);
    // Format Buffer.
    dSprintf(pBuffer, 256, "%s", object->getErrorDesc());
    // Return Velocity.
    return pBuffer;
}

ConsoleMethod(SimXMLDocument, pushFirstChildElement, bool, 3, 3,
"(string name) Push first child element with given name onto stack.\n"
        "@param name The name of the child element"
        "@return returns true on success, false otherwise.")
{
    return object->pushFirstChildElement( argv[2] );
}

ConsoleMethod(SimXMLDocument, pushChildElement, bool, 3, 3,
"(int index) Push the child element at the given index onto stack.\n"
        "@param index A nonnegative integer representing the index of the child element\n"
        "@return Returns true on success, and false otherwise")
{
    return object->pushChildElement( dAtoi( argv[2] ) );
}

ConsoleMethod(SimXMLDocument, nextSiblingElement, bool, 3, 3,
"(string name) Set top element on stack to next element with given name.\n"
        "@param name The name of the element.\n"
        "@return Returns true on success, false otherwise")
{
    return object->nextSiblingElement( argv[2] );
}

ConsoleMethod(SimXMLDocument, elementValue, const char*, 2, 2,
"() Get element value if it exists.\n"
        "@return A string with the desired value, or empty if not found.")
{
    // Create Returnable Buffer (because duration of value is unknown).
    char* pBuffer = Con::getReturnBuffer(256);
    dSprintf(pBuffer, 256, "%s", object->elementValue());
    return pBuffer;
}

ConsoleMethod(SimXMLDocument, popElement, void, 2, 2,
"() Pop last element off of stack.\n"
        "@return No return value")
{
    object->popElement();
}

ConsoleMethod(SimXMLDocument, attribute, const char*, 3, 3,
"(string attribute) Get attribute value if it exists.\n"
        "@param attribute The desired SimXMLDocument attribute\n"
        "@return Returns the value of the attribute as a string, or an empty string on failure")
{
    // Create Returnable Buffer (because duration of attribute is unknown).
    char* pBuffer = Con::getReturnBuffer(256);
    // Format Buffer.
    dSprintf(pBuffer, 256, "%s", object->attribute( argv[2] ));
    // Return Velocity.
    return pBuffer;
}
ConsoleMethod(SimXMLDocument, attributeF32, F32, 3, 3,
"(string attribute) Get attribute value if it exists.\n"
        "@param attribute The desired SimXMLDocument attribute\n"
        "@return Returns the value of the attribute converted to 32-bit floating-point value from string")
{
    return dAtof( object->attribute( argv[2] ) );
}
ConsoleMethod(SimXMLDocument, attributeS32, S32, 3, 3,
"(string attribute) Get attribute value if it exists.\n"
        "@param attribute The desired SimXMLDocument attribute\n"
        "@return Returns the value of the attribute converted to 32-bit integer value from string")
{
    return dAtoi( object->attribute( argv[2] ) );
}

ConsoleMethod(SimXMLDocument, attributeExists, bool, 3, 3,
"(string attribute) Get true if named attribute exists.\n"
        "@param attribute The desired attribute's name\n"
        "@return Returns true if attribute exists and false otherwise")
{
    return object->attributeExists( argv[2] );
}

ConsoleMethod(SimXMLDocument, firstAttribute, const char*, 2, 2,
"() Obtain the name of the current element's first attribute.\n"
        "@return A string with the name of the first attribute")
{
    const char* name = object->firstAttribute();

    // Create Returnable Buffer (because duration of attribute is unknown).
    char* pBuffer = Con::getReturnBuffer(dStrlen(name)+1);
    dStrcpy(pBuffer, name);
    return pBuffer;
}

ConsoleMethod(SimXMLDocument, lastAttribute, const char*, 2, 2,
"() Obtain the name of the current element's last attribute.\n"
        "@return A string with the name of the last attribute")
{
    const char* name = object->lastAttribute();

    // Create Returnable Buffer (because duration of attribute is unknown).
    char* pBuffer = Con::getReturnBuffer(dStrlen(name)+1);
    dStrcpy(pBuffer, name);
    return pBuffer;
}

ConsoleMethod(SimXMLDocument, nextAttribute, const char*, 2, 2,
"() Get the name of the next attribute for the current element after a call to firstAttribute().\n"
        "@return A string with the name of the next attribute")
{
    const char* name = object->nextAttribute();

    // Create Returnable Buffer (because duration of attribute is unknown).
    char* pBuffer = Con::getReturnBuffer(dStrlen(name)+1);
    dStrcpy(pBuffer, name);
    return pBuffer;
}

ConsoleMethod(SimXMLDocument, prevAttribute, const char*, 2, 2,
"() Get the name of the previous attribute for the current element after a call to lastAttribute().\n"
        "@return A string with the name of the previous attribute")
{
    const char* name = object->prevAttribute();

    // Create Returnable Buffer (because duration of attribute is unknown).
    char* pBuffer = Con::getReturnBuffer(dStrlen(name)+1);
    dStrcpy(pBuffer, name);
    return pBuffer;
}

ConsoleMethod(SimXMLDocument, setAttribute, void, 4, 4,
"(string attributeName, string attributeValue) Set attribute of top stack element to given value.\n"
        "@param attributeName The name of the attribute of the element you wish to set\n"
        "@param attributeValue The value you wish to set the given attribute to\n"
        "@return No return value.")
{
    object->setAttribute(argv[2], argv[3]);
}

ConsoleMethod(SimXMLDocument, setObjectAttributes, void, 3, 3,
"(string attributeValue) Set attribute of top stack element to given value.\n"
        "@param attributeValue The value you wish to set the given attribute to\n"
        "@return No return value.")
{
    object->setObjectAttributes(argv[2]);
}

ConsoleMethod(SimXMLDocument, pushNewElement, void, 3, 3,
"(string name) Create new element as child of current stack element "
        "and push new element on to stack.\n"
        "@param name The anme of the new element\n"
        "@return No return value")
{
    object->pushNewElement( argv[2] );
}

ConsoleMethod(SimXMLDocument, addNewElement, void, 3, 3,
"(string name) Create new element as child of current stack element "
        "and push new element on to stack.\n"
        "@param name The anme of the new element\n"
        "@return No return value")
{
    object->addNewElement( argv[2] );
}

ConsoleMethod(SimXMLDocument, addHeader, void, 2, 2, "() Add XML header to document.\n"
        "@return No return value.")
{
    object->addHeader();
}

ConsoleMethod(SimXMLDocument, addComment, void, 3, 3, "(string comment) Add the given comment as a child of current stack element.\n"
        "@param comment The desired comment to add\n"
        "@return No return value.")
{
    object->addComment(argv[2]);
}

ConsoleMethod(SimXMLDocument, readComment, const char*, 3, 3, "(S32 index) Returns the comment at the specified index.\n"
        "@param index The index of the desired comment as a nonnegative integer value\n"
        "@return The comment as a string")
{
    return object->readComment( dAtoi( argv[2] ) );
}

ConsoleMethod(SimXMLDocument, addText, void, 3, 3, "(string text) Add the given text as a child of current stack element.\n"
        "@param text The desired text to add\n"
        "@return No return value.")
{
    object->addText(argv[2]);
}

ConsoleMethod(SimXMLDocument, getText, const char*, 2, 2, "() Gets the text from the current stack element.\n"
        "@return Returns the desired text, or empty string on failure")
{
    const char* text = object->getText();
    if( !text )
        return "";

    char* pBuffer = Con::getReturnBuffer(dStrlen( text ) + 1);
    dStrcpy( pBuffer, text );
    return pBuffer;
}

ConsoleMethod(SimXMLDocument, removeText, void, 2, 2, "() Remove any text on the current stack element.\n"
        "@return No return value\n")
{
    object->removeText();
}

ConsoleMethod(SimXMLDocument, addData, void, 3, 3, "(string text) Add the given text as a child of current stack element.\n"
        "@param The desired text to add\n"
        "@return No return value")
{
    object->addData(argv[2]);
}

ConsoleMethod(SimXMLDocument, getData, const char*, 2, 2, "() Gets the text from the current stack element.\n"
        "@return Returns the desired text or empty string if failed (no text)")
{
    const char* text = object->getData();
    if( !text )
        return "";

    char* pBuffer = Con::getReturnBuffer(dStrlen( text ) + 1);
    dStrcpy( pBuffer, text );
    return pBuffer;
}

#endif