#include "glfuncs.h"
#include "shaders.h"

struct gl_state
{
	u32 Texture;
    u32 Palette;
    u32 SquareVBO;
    u32 BlitShader;
};

global gl_state GL = { };

static u8 NESPalette[196]
{
    0x6b, 0x6b, 0x6b,  0x00, 0x1b, 0x88,  0x21, 0x00, 0x9a,  0x40, 0x00, 0x8c, 
    0x60, 0x00, 0x67,  0x64, 0x00, 0x1e,  0x59, 0x08, 0x00,  0x48, 0x16, 0x00, 
    0x28, 0x36, 0x00,  0x00, 0x45, 0x00,  0x00, 0x49, 0x08,  0x00, 0x42, 0x1d,
    0x00, 0x36, 0x59,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,
    0xb4, 0xb4, 0xb4,  0x15, 0x55, 0xd3,  0x43, 0x37, 0xef,  0x74, 0x25, 0xdf,
    0x9c, 0x19, 0xb9,  0xac, 0x0f, 0x64,  0xaa, 0x2c, 0x00,  0x8a, 0x4b, 0x00,
    0x66, 0x6b, 0x00,  0x21, 0x83, 0x00,  0x00, 0x8a, 0x00,  0x00, 0x81, 0x44, 
    0x00, 0x76, 0x91,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,
    0xff, 0xff, 0xff,  0x63, 0xb2, 0xff,  0x7c, 0x9c, 0xff,  0xc0, 0x7d, 0xfe,
    0xe9, 0x77, 0xff,  0xf5, 0x72, 0xcd,  0xf4, 0x88, 0x6b,  0xdd, 0xa0, 0x29, 
    0xbd, 0xbd, 0x0a,  0x89, 0xd2, 0x0e,  0x5c, 0xde, 0x3e,  0x4b, 0xd8, 0x86,
    0x4d, 0xcf, 0xd2,  0x52, 0x52, 0x52,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,
    0xff, 0xff, 0xff,  0xbc, 0xdf, 0xff,  0xd2, 0xd2, 0xff,  0xe1, 0xc8, 0xff, 
    0xef, 0xc7, 0xff,  0xff, 0xc3, 0xe1,  0xff, 0xca, 0xc6,  0xf2, 0xda, 0xad,
    0xeb, 0xe3, 0xa0,  0xd2, 0xed, 0xa2,  0xbc, 0xf4, 0xb4,  0xb5, 0xf1, 0xce,
    0xb6, 0xec, 0xf1,  0xbf, 0xbf, 0xbf,  0x00, 0x00, 0x00,  0x00, 0x00, 0x00,
};

static b32
OpenGL_CheckError()
{
    GLenum Error = glGetError();
    if (!Error) return Error;
    Api_Error(TSPrint("Opengl Error Code: 0x%X\n", Error));
    Assert(!Error);
    return Error;
}

static void
OpenGL_DrawFullscreenQuad()
{
    glBindBuffer(GL_ARRAY_BUFFER, GL.SquareVBO);
    glVertexAttribPointer(0, 2, GL_UNSIGNED_BYTE, GL_FALSE, 2, 0);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(0);
}

static void
OpenGL_LoadPalette(string Filename)
{
    mi Bytes;
    u8 *Palette = (u8 *)File_ReadEntireFile(Filename, &Bytes);
    if (!Palette) return;
    if (Bytes != 192) return;
    Memory_Copy(NESPalette, Palette, 192);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, GL.Palette);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 64, 1, GL_RGB, GL_UNSIGNED_BYTE, NESPalette);
}

static b32
OpenGL_Init()
{
    glGenTextures(1, &GL.Texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GL.Texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 256, 224, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
    
    glGenTextures(1, &GL.Palette);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, GL.Palette);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, NESPalette);
    
    OpenGL_LoadPalette(S("palette.pal"));
	
    GL.BlitShader = OpenGL_LoadProgram(BlitShaderV, BlitShaderF);

	{   /* SQUARE VAO */
        const u8 Vs[] = { 0, 0, 1, 0, 1, 1, 1, 1, 0, 1, 0, 0 };
        glGenBuffers(1, &GL.SquareVBO);
        glBindBuffer(GL_ARRAY_BUFFER, GL.SquareVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vs), Vs, GL_STATIC_DRAW);
    }

    return OpenGL_CheckError();
}

static void
OpenGL_Blit(irect DrawArea)
{
	ivec2 DrawMin = Min(DrawArea);
    ivec2 DrawDim = Dim(DrawArea);
    glViewport(DrawMin.x, DrawMin.y, DrawDim.x, DrawDim.y);
    glScissor(DrawMin.x, DrawMin.y, DrawDim.x, DrawDim.y);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(GL.BlitShader);
    OpenGL_DrawFullscreenQuad();
}

static void
OpenGL_Clear(ivec2 WindowDim)
{
    glViewport(0, 0, WindowDim.x, WindowDim.y);
    glScissor(0, 0, WindowDim.x, WindowDim.y);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);
}

static void
OpenGL_Frame(u8 *Screen, u32 Background)
{
    Background = (Background & 63) * 3;
    f32 R = NESPalette[Background + 0] / 255.0f;
    f32 G = NESPalette[Background + 1] / 255.0f;
    f32 B = NESPalette[Background + 2] / 255.0f;
    glClearColor(R, G, B, 1.0f);

	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GL.Texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 224, GL_RED, GL_UNSIGNED_BYTE, Screen + (8*256));
}
