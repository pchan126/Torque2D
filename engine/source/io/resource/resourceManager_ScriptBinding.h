ConsoleFunction( getModPaths, const char*, 1, 1, "() Use the getModPaths function to get the current mod path information.\n"
        "@return Returns a string equivalent to the complete current mod path, that is all pads that are visible to the file manager.\n"
        "@sa setModPaths")
{
    return( ResourceManager->getModPaths() );
}

#ifdef TORQUE_DEBUG

ConsoleFunction(dumpResources, void, 2, 2, "(onlyLoaded?) Use the dumpLoadedResources function to dump a listing of the currently in-use resources to the console. This will include such things as sound files, font files, etc.\n"
        "For this to work, the engine must have been compiled with TORQUE_DEBUG defined.\n"
        "@return No return value.\n"
        "@sa purgeResources")
{
    const bool onlyLoaded = argc == 2 ? dAtob(argv[1]) : true;
    ResourceManager->dumpResources(onlyLoaded);
}

#endif


ConsoleFunction(addResPath, void, 2, 3, "(path, [ignoreZips=false]) Add a path to the resource manager")
{
    if( argc > 2 )
    ResourceManager->addPath(argv[1], dAtob(argv[2]));
    else
            ResourceManager->addPath(argv[1]);
}


ConsoleFunction(removeResPath, void, 2, 2, "(pathExpression) Remove a path from the resource manager. Path is an expression as in findFirstFile()")
{
    ResourceManager->removePath(argv[1]);
}


// Mod paths aren't used in tools applications.
// See : addResPath/removeResPath console functions
ConsoleFunction( setModPaths, void, 2, 2, "( path ) Use the setModPaths function to set the current mod path to the value specified in path.\n"
        "@param path A string containing a semi-colon (;) separated list of game and mod paths.\n"
        "@return No return value.\n"
        "@sa getModPaths")
{
    char buf[512];
    dStrncpy(buf, argv[1], sizeof(buf) - 1);
    buf[511] = '\0';

    Vector<char *> paths;
    char* temp = dStrtok( buf, ";" );
    while ( temp )
    {
        if ( temp[0] )
            paths.push_back(temp);

        temp = dStrtok( nullptr, ";" );
    }

    ResourceManager->setModPaths( paths.size(), (const char**) paths.address() );
}

ConsoleFunction( purgeResources, void, 1, 1, "() Use the purgeResources function to purge all game resources.\n"
        "@return No return value.\n"
        "@sa clearTextureHolds, dumpResourceStats, dumpTextureStats, flushTextureCache")
{
    ResourceManager->purge();
}


