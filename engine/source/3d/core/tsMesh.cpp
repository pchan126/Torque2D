//
// Created by Paul L Jan on 2013-08-22.
// Copyright (c) 2013 Michael Perry. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//


#include "tsMesh.h"

void TSMesh::innerRender(TSVertexBufferHandle& vb) {
    if ( !vb.isValid() )
        return;

    GFX->setVertexBuffer( vb );

//    for( U32 p = 0; p < primitives.size(); p++ )
//            GFX->drawPrimitive( p );
}
