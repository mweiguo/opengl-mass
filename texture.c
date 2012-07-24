/**
 * gcc texture.c -Ithirdparty/freeglut-2.8.0/include -I../sgt/comicviewer/subframeanalyzer -Lthirdparty/freeglut-2.8.0/src/.libs/ -lglut -lopengl32 -lglu32 -o texture
 */
#include <GL/freeglut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <utl_globals.h>
#include <stdio.h>
#include <time.h>

GLuint texid = 0;
char texfile[256];
char texfile2[256];
void init ()
{
    FILE* fp;
    int raw_image_size;
    char* rawimage;
    int para[2];
    int tick;

    if ( NULL==(fp=fopen( texfile, "rb" )) ) {
        printf ( "tex file %s can not opened\n", texfile );
    } else {
        tick = clock();
        fseek ( fp, 0, SEEK_END );
        raw_image_size = ftell ( fp );
        fseek ( fp, 0, SEEK_SET );
        rawimage = (char*)malloc ( raw_image_size );
        fread ( rawimage, raw_image_size, 1, fp );
        fclose( fp );
        printf ( "read image from disk take %d tick\n", tick );
        /* enable */
        glEnable ( GL_TEXTURE_2D );

        /* texture */
        glGenTextures ( 1, &texid );
        glBindTexture ( GL_TEXTURE_2D, texid );
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
/*         glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); */
/*         glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); */
        tick = clock();
        glTexImage2D ( GL_TEXTURE_2D, 0, 3, ((JPEGIMAGE*)rawimage)->width, ((JPEGIMAGE*)rawimage)->height, 0, 
                       ((JPEGIMAGE*)rawimage)->color_components==3 ? GL_RGB : GL_RGBA, 
                       GL_UNSIGNED_BYTE, ((JPEGIMAGE*)rawimage)->image_buffer );
/*         glTexImage2D ( GL_TEXTURE_2D, 0, 3, ((JPEGIMAGE*)rawimage)->width, ((JPEGIMAGE*)rawimage)->height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0 ); */
        printf ( "upload image to video memory take %d ticks\n", clock()-tick );
        glBindTexture ( GL_TEXTURE_2D, 0 );
        free ( rawimage );

        if ( NULL!=(fp=fopen( texfile2, "rb" )) ) {
            tick = clock();
            fseek ( fp, 0, SEEK_END );
            raw_image_size = ftell ( fp );
            fseek ( fp, 0, SEEK_SET );
            rawimage = (char*)malloc ( raw_image_size );
            fread ( rawimage, raw_image_size, 1, fp );
            fclose( fp );
            printf ( "read image2 from disk take %d tick\n", tick );

            tick = clock();
            glBindTexture ( GL_TEXTURE_2D, texid );
            glTexSubImage2D ( GL_TEXTURE_2D, 0, 0, 0, ((JPEGIMAGE*)rawimage)->width, ((JPEGIMAGE*)rawimage)->height, 
                              ((JPEGIMAGE*)rawimage)->color_components==3 ? GL_RGB : GL_RGBA, 
                              GL_UNSIGNED_BYTE, ((JPEGIMAGE*)rawimage)->image_buffer );
            printf ( "upload image2 to video memory take %d ticks\n", clock()-tick );
            free ( rawimage );
        }


    }
    glClearColor ( 0.4f, 0.4f, 0.4f, 1.0f );

    /* matrix */
    glMatrixMode ( GL_PROJECTION );
    glLoadIdentity ();
    glOrtho ( -1, 1, -1, 1, 1, 100 );
    glMatrixMode ( GL_MODELVIEW );
    printf ( "init ok\n" );
}

void display ()
{
    glClear ( GL_COLOR_BUFFER_BIT );
    glLoadIdentity ();
    gluLookAt ( 0, 0, 2, 0, 0, 0, 0, 1, 0 );

    if ( texid == 0 ) {
        glBegin ( GL_QUADS );
        glVertex2f ( -1.0f, -1.0f );
        glVertex2f ( -1.0f,  1.0f );
        glVertex2f (  1.0f,  1.0f );
        glVertex2f (  1.0f, -1.0f );
        glEnd ();
    } else {
        glBindTexture ( GL_TEXTURE_2D, texid );
        glBegin ( GL_QUADS );
            glTexCoord2f ( 0.0f, 1.0f ); glVertex2f ( -1.0f, -1.0f );
            glTexCoord2f ( 0.0f, 0.0f ); glVertex2f ( -1.0f,  1.0f );
            glTexCoord2f ( 1.0f, 0.0f ); glVertex2f (  1.0f,  1.0f );
            glTexCoord2f ( 1.0f, 1.0f ); glVertex2f (  1.0f, -1.0f );
        glEnd ();
        glBindTexture ( GL_TEXTURE_2D, 0 );
    }
    
    glutSwapBuffers ();
}

void keyboard ( unsigned char key, int x, int y )
{
    if ( key == 0x1b )
        exit (0);
}

int main ( int argc, char* argv[] )
{
    if ( argc != 2 && argc != 3 ) {
        printf ( "usage : texture first_texture_file second_texture_file\n");
        return 1;
    }

    glutInit ( &argc, argv );
    glutInitWindowPosition ( 0, 0 );
    glutInitWindowSize ( 400, 300 );
    glutInitDisplayMode ( GLUT_RGBA | GLUT_DOUBLE );
    glutCreateWindow ( argv[0] );
    
    if ( argc > 1)
        strcpy ( texfile, argv[1] );
    if ( argc > 2)
        strcpy ( texfile2, argv[2] );

    init ();
    glutDisplayFunc ( display );
    glutKeyboardFunc ( keyboard );

    glutMainLoop ();
    return 0;
}
