#ifdef ENABLE_OPENGL

#include <glad.h>

/* Pull in OpenGL headers */
/* following scissored from SDL_opengl.h */
#if defined(__APPLE__) || defined(HAVE_APPLE_OPENGL_FRAMEWORK)
#include <OpenGL/gl.h> /* Header File For The OpenGL Library */
#include <OpenGL/glu.h> /* Header File For The GLU Library */
#elif defined(__MACOS__)
#include <gl.h> /* Header File For The OpenGL Library */
#include <glu.h> /* Header File For The GLU Library */
#else
#include <GL/gl.h> /* Header File For The OpenGL Library */
#include <GL/glu.h> /* Header File For The GLU Library */
#endif

#endif
