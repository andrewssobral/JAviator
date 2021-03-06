/*--------------------------------------------------------------------------
 *
 * The code is part of JAviator project (http://javiator.cs.uni-salzburg.at)
 *
 *--------------------------------------------------------------------------
 * Date: 11-Apr-2007
 *--------------------------------------------------------------------------
 *
 * Copyright (c) 2006 The University of Salzburg.
 * All rights reserved. Permission is hereby granted, without written 
 * agreement and without license or royalty fees, to use, copy, modify, and 
 * distribute this software and its documentation for any purpose, provided 
 * that the above copyright notice and the following two paragraphs appear 
 * in all copies of this software.
 *
 * IN NO EVENT SHALL THE UNIVERSITY OF SALZBURG BE LIABLE TO ANY PARTY
 * FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF
 * THE UNIVERSITY OF SALZBURG HAS BEEN ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.

 * THE UNIVERSITY OF SALZBURG SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
 * SALZBURG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 *
 *--------------------------------------------------------------------------
 * Created by Silviu Craciunas <silviu.craciunas@gmail.com>
 *--------------------------------------------------------------------------
 */

package scene;

import javax.media.j3d.Appearance;
import javax.media.j3d.BranchGroup;
import javax.media.j3d.Geometry;
import javax.media.j3d.GeometryArray;

import javax.media.j3d.LineArray;
import javax.media.j3d.Material;
import javax.media.j3d.PolygonAttributes;
import javax.media.j3d.QuadArray;
import javax.media.j3d.Shape3D;
import javax.media.j3d.TexCoordGeneration;
import javax.media.j3d.Transform3D;
import javax.media.j3d.TransformGroup;
import javax.media.j3d.TransparencyAttributes;
import javax.vecmath.Color3f;

import javax.vecmath.Quat4f;
import javax.vecmath.Vector3f;
import javax.vecmath.Vector4f;

/**
 * 
 * The XYZ Axis
 * 
 * @author scraciunas
 * 
 */
public class Axis extends BranchGroup
{

    private TexCoordGeneration texgen = null;

    /**
     * create axis subgraph
     */
    public Axis( )
    {

        this.addChild( createSceneGraph( ) );
        this.setName( "Axes" );
        this.setCapability( BranchGroup.ALLOW_DETACH );

    } // end of axis constructor

    /**
     * @return
     */
    private TransformGroup createSceneGraph( )
    {

        TransformGroup trans = new TransformGroup( );
        trans.setCapability( TransformGroup.ALLOW_TRANSFORM_READ );
        trans.setCapability( TransformGroup.ALLOW_TRANSFORM_WRITE );

        createAppearance( );

        Vector4f planeS = new Vector4f( );
        texgen.getPlaneS( planeS );
        TransformGroup strans = createPlaneTrans( planeS );
        trans.addChild( strans );

        double[] pverts = { -1.0, 1.0, 0.0, -1.0, -1.0, 0.0, 1.0, -1.0, 0.0, 1.0, 1.0, 0.0 };

        float[] pnormals = { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f };

        QuadArray pgeom = new QuadArray( pverts.length / 3, GeometryArray.COORDINATES | GeometryArray.NORMALS );
        pgeom.setCapability( Geometry.ALLOW_INTERSECT );
        pgeom.setCoordinates( 0, pverts );
        pgeom.setNormals( 0, pnormals );
        Appearance appS = createPlaneApp( 1.0f, 0.5f, 0.5f );
        Shape3D polygonS = new Shape3D( pgeom, appS );
        strans.addChild( polygonS );

        Vector4f planeT = new Vector4f( );
        texgen.getPlaneT( planeT );
        TransformGroup ttrans = createPlaneTrans( planeT );
        trans.addChild( ttrans );

        Appearance appT = createPlaneApp( 0.5f, 1.0f, 0.5f );
        Shape3D polygonT = new Shape3D( pgeom, appT );
        ttrans.addChild( polygonT );

        // XYZ
        double[] vertsAx = { -2.0, 1.0, 0.0, 2.0, 1.0, 0.0, // X 
        0.0, -2.0, 0.0, 0.0, 2.0, 0.0, // Y
        0.0, 1.0, -2.0, 0.0, 1.0, 2.0 }; // Z 

        float[] colorsAx = { 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f };

        LineArray geomAx = new LineArray( vertsAx.length / 3, GeometryArray.COORDINATES | GeometryArray.COLOR_3 );
        geomAx.setCoordinates( 0, vertsAx );
        geomAx.setColors( 0, colorsAx );
        Shape3D axis = new Shape3D( geomAx );
        trans.addChild( axis );

        return trans;
    }

    /**
     * @param plane
     * @return
     */
    private TransformGroup createPlaneTrans( Vector4f plane )
    {
        Transform3D t3d = new Transform3D( );

        Vector3f zvec = new Vector3f( 0.0f, 0.0f, 1.0f );
        Vector3f plane3f = new Vector3f( plane.x, plane.y, plane.z );
        zvec.sub( plane3f );
        zvec.normalize( );

        Quat4f quat = new Quat4f( zvec.x, zvec.y, zvec.z, 0.0f );
        t3d.setRotation( quat );

        Transform3D zt3d = new Transform3D( );
        Vector3f ztranslate = new Vector3f( 0.0f, 0.0f, plane.w );
        zt3d.setTranslation( ztranslate );

        t3d.mul( zt3d );

        return new TransformGroup( t3d );
    }

    /**
     * @return
     */
    private Appearance createAppearance( )
    {
        Appearance app = new Appearance( );

        texgen = new TexCoordGeneration( TexCoordGeneration.OBJECT_LINEAR, TexCoordGeneration.TEXTURE_COORDINATE_2, new Vector4f( 1.0f, 0.0f, 0.0f, 0.0f ), // PlaneS
        new Vector4f( 0.0f, 1.0f, 0.0f, 0.0f ) ); // PlaneT
        app.setTexCoordGeneration( texgen );

        return app;
    }

    /**
     * @param r
     * @param g
     * @param b
     * @return
     */
    private Appearance createPlaneApp( float r, float g, float b )
    {
        Appearance app = new Appearance( );
        Material mat = new Material( );
        mat.setDiffuseColor( new Color3f( r, g, b ) );
        mat.setSpecularColor( new Color3f( 0.0f, 0.0f, 0.0f ) ); // ¹õ
        app.setMaterial( mat );
        TransparencyAttributes tattr = new TransparencyAttributes( TransparencyAttributes.BLENDED, 0.7f );
        app.setTransparencyAttributes( tattr );
        PolygonAttributes pattr = new PolygonAttributes( PolygonAttributes.POLYGON_FILL, PolygonAttributes.CULL_NONE, 0.0f, true );
        app.setPolygonAttributes( pattr );
        return app;
    }

} // end of class Axis
