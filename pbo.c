/**
 * gcc pbo.c -Ithirdparty/freeglut-2.8.0/include -Ithirdparty/glew-1.6.0/include -I../sgt/comicviewer/subframeanalyzer -Lthirdparty/freeglut-2.8.0/src/.libs/ -Lthirdparty/glew-1.6.0/lib -lglew32 -lglut -lopengl32 -lglu32 -o pbo
 */
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <utl_globals.h>
#include <stdio.h>
#include <time.h>

GLuint texid = 0;
char texfile[256];
GLuint pbo[2];
int frame_no = 0;
JPEGIMAGE* g_rawimage = 0;

void init ()
{
    FILE* fp;
    int raw_image_size;
    JPEGIMAGE* rawimage;
    int tick;

    glClearColor ( 0.4f, 0.4f, 0.4f, 1.0f );
//    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);      // 4-byte pixel alignment
    glEnable ( GL_TEXTURE_2D );

    /*image*/
    if ( NULL==(fp=fopen( texfile, "rb" )) ) {
        printf ( "tex file %s can not opened\n", texfile );
    } else {
        tick = clock();
        fseek ( fp, 0, SEEK_END );
        raw_image_size = ftell ( fp );
        fseek ( fp, 0, SEEK_SET );
        rawimage = (JPEGIMAGE*)malloc ( raw_image_size );
        fread ( rawimage, raw_image_size, 1, fp );
        fclose( fp );
        g_rawimage = rawimage;
    }

    /* matrix */
    glMatrixMode ( GL_PROJECTION );
    glLoadIdentity ();
    glOrtho ( -1, 1, -1, 1, 1, 100 );
    glMatrixMode ( GL_MODELVIEW );

    /* texture */
    glGenTextures ( 1, &texid );
    glBindTexture ( GL_TEXTURE_2D, texid );
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
    glTexImage2D  ( GL_TEXTURE_2D, 0, GL_RGBA, rawimage->width, rawimage->height, 0, 
                    rawimage->color_components==4 ? GL_RGBA : GL_RGB,
                    GL_UNSIGNED_BYTE, g_rawimage->image_buffer );
    glBindTexture ( GL_TEXTURE_2D, 0 );

    /* generate pixel buffer object */
    glGenBuffers (2, pbo);
    glBindBuffer (GL_PIXEL_UNPACK_BUFFER, pbo[0] );
    glBufferData (GL_PIXEL_UNPACK_BUFFER, rawimage->width * rawimage->height * rawimage->color_components, 0, GL_DYNAMIC_DRAW/*GL_STREAM_DRAW*/ );
    glBindBuffer (GL_PIXEL_UNPACK_BUFFER, pbo[1] );
    glBufferData (GL_PIXEL_UNPACK_BUFFER, rawimage->width * rawimage->height * rawimage->color_components, 0, GL_DYNAMIC_DRAW/*GL_STREAM_DRAW*/ );
    glBindBuffer (GL_PIXEL_UNPACK_BUFFER, 0 );

    printf ( "init ok\n" );
}

void update_texture ( JPEGIMAGE* rawimage )
{
    int index = frame_no % 2;
    int next_index = (frame_no + 1) % 2;
    char* mBuffer;
    clock_t tick, bind_tick, data_tick, map_tick, unmap_tick, unbind_tick, memcpy_tick;
    ++frame_no;

    tick = clock();
    glBindTexture   (GL_TEXTURE_2D, texid );
    glBindBuffer    (GL_PIXEL_UNPACK_BUFFER, pbo[index] );
    glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, rawimage->width, rawimage->height, 
                     rawimage->color_components == 4 ? GL_RGBA : GL_RGB, 
                     GL_UNSIGNED_BYTE, 0 );
/*     printf ( "\tstage1(upload) take %d tick\n", clock() - tick ); */

    tick = clock();
/*     bind_tick = clock (); */
    glBindBuffer    (GL_PIXEL_UNPACK_BUFFER, pbo[next_index] );
/*     bind_tick = clock () - bind_tick; */

/*     data_tick = clock (); */
/*     glBufferData    (GL_PIXEL_UNPACK_BUFFER, rawimage->width * rawimage->height * rawimage->color_components, 0, GL_STREAM_DRAW ); */
/*     data_tick = clock () - data_tick; */
    
/*     map_tick = clock (); */
    mBuffer = glMapBuffer (GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY );
/*     map_tick = clock () - map_tick; */

    if ( mBuffer ) {
/*         memcpy_tick = clock(); */
        memcpy ( mBuffer, rawimage->image_buffer, rawimage->buffer_length );
/*         memcpy_tick = clock() - memcpy_tick; */

/*         unmap_tick = clock(); */
        glUnmapBuffer ( GL_PIXEL_UNPACK_BUFFER );
/*         unmap_tick = clock() - unmap_tick; */
    }
/*     unbind_tick = clock(); */
    glBindBuffer    (GL_PIXEL_UNPACK_BUFFER, 0 );
/*     unbind_tick = clock() - unbind_tick; */
/*     printf ( "\tstage2(write new data to mapped memory) take %d tick\n", clock() - tick ); */
/*     printf ( "\tbind=%d, bufferdata=%d, mapbuffer=%d, memcpy=%d, unmap=%d, unbind=%d\n",  */
/*              bind_tick, data_tick, map_tick, memcpy_tick, unmap_tick, unbind_tick ); */
}

void display ()
{
    clock_t tick;
    glClear ( GL_COLOR_BUFFER_BIT );
    glLoadIdentity ();
    gluLookAt ( 0, 0, 2, 0, 0, 0, 0, 1, 0 );

    printf ( "------------------------------------------------------------\n" );
    tick = clock();
    update_texture ( g_rawimage );
    printf ( "upload image from memory to video card memory take %d tick\n", clock() - tick );

    tick = clock();
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
    printf ( "render take %d tick\n", clock() - tick );
}

void keyboard ( unsigned char key, int x, int y )
{
    if ( key == 0x1b )
        exit (0);
}

int main ( int argc, char* argv[] )
{
    if ( argc != 2 ) {
        printf ( "usage : pbo raw_image_file\n" );
        return 1;
    }
    strcpy ( texfile, argv[1] );
    glutInit ( &argc, argv );
    glutInitWindowPosition ( 0, 0 );
    glutInitWindowSize ( 400, 300 );
    glutInitDisplayMode ( GLUT_RGBA | GLUT_DOUBLE );
    glutCreateWindow ( argv[0] );
    
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        /* Problem: glewInit failed, something is seriously wrong. */
        printf("Error: %s\n", glewGetErrorString(err));
    }
    printf( "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

    init ();
    glutDisplayFunc ( display );
    glutKeyboardFunc ( keyboard );

    glutMainLoop ();
    return 0;
}
