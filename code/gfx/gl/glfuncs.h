#define GL_FUNC(NAME, name) static PFN##NAME##PROC name

/* 2.0 */
GL_FUNC(GLGENBUFFERS,               glGenBuffers);
GL_FUNC(GLBINDBUFFER,               glBindBuffer);
GL_FUNC(GLDELETEBUFFERS,            glDeleteBuffers);
GL_FUNC(GLBUFFERDATA,               glBufferData);
GL_FUNC(GLBUFFERSUBDATA,            glBufferSubData);
GL_FUNC(GLCREATESHADER,             glCreateShader);
GL_FUNC(GLCOMPILESHADER,            glCompileShader);
GL_FUNC(GLDELETESHADER,             glDeleteShader);
GL_FUNC(GLSHADERSOURCE,             glShaderSource);
GL_FUNC(GLGETSHADERIV,              glGetShaderiv);
GL_FUNC(GLATTACHSHADER,             glAttachShader);
GL_FUNC(GLGETSHADERINFOLOG,         glGetShaderInfoLog);
GL_FUNC(GLDETACHSHADER,             glDetachShader);
GL_FUNC(GLCREATEPROGRAM,            glCreateProgram);
GL_FUNC(GLLINKPROGRAM,              glLinkProgram);
GL_FUNC(GLUSEPROGRAM,               glUseProgram);
GL_FUNC(GLGETPROGRAMIV,             glGetProgramiv);
GL_FUNC(GLGETPROGRAMINFOLOG,        glGetProgramInfoLog);
GL_FUNC(GLDELETEPROGRAM,            glDeleteProgram);
GL_FUNC(GLENABLEVERTEXATTRIBARRAY,  glEnableVertexAttribArray);
GL_FUNC(GLDISABLEVERTEXATTRIBARRAY, glDisableVertexAttribArray);
GL_FUNC(GLVERTEXATTRIBPOINTER,      glVertexAttribPointer);
GL_FUNC(GLBINDATTRIBLOCATION,       glBindAttribLocation);
GL_FUNC(GLACTIVETEXTURE,            glActiveTexture);
GL_FUNC(GLBLENDFUNCSEPARATE,        glBlendFuncSeparate);
GL_FUNC(GLGETUNIFORMLOCATION,       glGetUniformLocation);
GL_FUNC(GLUNIFORM1F,                glUniform1f);
GL_FUNC(GLUNIFORM2F,                glUniform2f);
GL_FUNC(GLUNIFORM3F,                glUniform3f);
GL_FUNC(GLUNIFORM4F,                glUniform4f);
GL_FUNC(GLUNIFORM1I,                glUniform1i);
GL_FUNC(GLUNIFORM2I,                glUniform2i);
GL_FUNC(GLUNIFORM3I,                glUniform3i);
GL_FUNC(GLUNIFORM4I,                glUniform4i);
GL_FUNC(GLUNIFORM1FV,               glUniform1fv);
GL_FUNC(GLUNIFORM2FV,               glUniform2fv);
GL_FUNC(GLUNIFORM3FV,               glUniform3fv);
GL_FUNC(GLUNIFORM2IV,               glUniform2iv);
GL_FUNC(GLUNIFORMMATRIX4FV,         glUniformMatrix4fv);
GL_FUNC(GLVERTEXATTRIB2F,           glVertexAttrib2f);
GL_FUNC(GLVERTEXATTRIB3F,           glVertexAttrib3f);
GL_FUNC(GLVERTEXATTRIB4F,           glVertexAttrib4f);
GL_FUNC(GLVERTEXATTRIB3FV,          glVertexAttrib3fv);

/* 3.0 */
GL_FUNC(GLGENFRAMEBUFFERS,          glGenFramebuffers);
GL_FUNC(GLBINDFRAMEBUFFER,          glBindFramebuffer);
GL_FUNC(GLFRAMEBUFFERTEXTURE2D,     glFramebufferTexture2D);
GL_FUNC(GLCHECKFRAMEBUFFERSTATUS,   glCheckFramebufferStatus);
GL_FUNC(GLGENRENDERBUFFERS,         glGenRenderbuffers);
GL_FUNC(GLBINDRENDERBUFFER,         glBindRenderbuffer);
GL_FUNC(GLRENDERBUFFERSTORAGE,      glRenderbufferStorage);
GL_FUNC(GLFRAMEBUFFERRENDERBUFFER,  glFramebufferRenderbuffer);
GL_FUNC(GLGENERATEMIPMAP,           glGenerateMipmap);

/* ES 2.0 */
GL_FUNC(GLCLEARDEPTHF, glClearDepthf);
GL_FUNC(GLDEPTHRANGEF, glDepthRangef);

#define GL_LOAD_AND_CHECK(NAME, name) name = (PFN##NAME##PROC)GL_LOAD_FUNC(#name); if(!name) return false;

static b32
GL_LoadFunctions()
{
    GL_LOAD_AND_CHECK(GLGENBUFFERS,               glGenBuffers);
    GL_LOAD_AND_CHECK(GLBINDBUFFER,               glBindBuffer);
    GL_LOAD_AND_CHECK(GLDELETEBUFFERS,            glDeleteBuffers);
    GL_LOAD_AND_CHECK(GLBUFFERDATA,               glBufferData);
    GL_LOAD_AND_CHECK(GLCREATEPROGRAM,            glCreateProgram);
    GL_LOAD_AND_CHECK(GLCREATESHADER,             glCreateShader);
    GL_LOAD_AND_CHECK(GLDELETESHADER,             glDeleteShader);
    GL_LOAD_AND_CHECK(GLSHADERSOURCE,             glShaderSource);
    GL_LOAD_AND_CHECK(GLCOMPILESHADER,            glCompileShader);
    GL_LOAD_AND_CHECK(GLGETSHADERIV,              glGetShaderiv);
    GL_LOAD_AND_CHECK(GLATTACHSHADER,             glAttachShader);
    GL_LOAD_AND_CHECK(GLGETSHADERINFOLOG,         glGetShaderInfoLog);
    GL_LOAD_AND_CHECK(GLDETACHSHADER,             glDetachShader);
    GL_LOAD_AND_CHECK(GLLINKPROGRAM,              glLinkProgram);
    GL_LOAD_AND_CHECK(GLGETPROGRAMIV,             glGetProgramiv);
    GL_LOAD_AND_CHECK(GLUSEPROGRAM,               glUseProgram);
    GL_LOAD_AND_CHECK(GLGETPROGRAMINFOLOG,        glGetProgramInfoLog);
    GL_LOAD_AND_CHECK(GLDELETEPROGRAM,            glDeleteProgram);
    GL_LOAD_AND_CHECK(GLENABLEVERTEXATTRIBARRAY,  glEnableVertexAttribArray);
    GL_LOAD_AND_CHECK(GLDISABLEVERTEXATTRIBARRAY, glDisableVertexAttribArray);
    GL_LOAD_AND_CHECK(GLVERTEXATTRIBPOINTER,      glVertexAttribPointer);
    GL_LOAD_AND_CHECK(GLGETUNIFORMLOCATION,       glGetUniformLocation);
    GL_LOAD_AND_CHECK(GLACTIVETEXTURE,            glActiveTexture);
    GL_LOAD_AND_CHECK(GLBUFFERSUBDATA,            glBufferSubData);
    GL_LOAD_AND_CHECK(GLBINDATTRIBLOCATION,       glBindAttribLocation);
    GL_LOAD_AND_CHECK(GLBLENDFUNCSEPARATE,        glBlendFuncSeparate);
    GL_LOAD_AND_CHECK(GLUNIFORM1F,                glUniform1f);
    GL_LOAD_AND_CHECK(GLUNIFORM2F,                glUniform2f);
    GL_LOAD_AND_CHECK(GLUNIFORM3F,                glUniform3f);
    GL_LOAD_AND_CHECK(GLUNIFORM4F,                glUniform4f);
    GL_LOAD_AND_CHECK(GLUNIFORM1I,                glUniform1i);
    GL_LOAD_AND_CHECK(GLUNIFORM2I,                glUniform2i);
    GL_LOAD_AND_CHECK(GLUNIFORM3I,                glUniform3i);
    GL_LOAD_AND_CHECK(GLUNIFORM4I,                glUniform4i);
    GL_LOAD_AND_CHECK(GLUNIFORM1FV,               glUniform1fv);
    GL_LOAD_AND_CHECK(GLUNIFORM2FV,               glUniform2fv);
    GL_LOAD_AND_CHECK(GLUNIFORM3FV,               glUniform3fv);
    GL_LOAD_AND_CHECK(GLUNIFORM2IV,               glUniform2iv);
    GL_LOAD_AND_CHECK(GLUNIFORMMATRIX4FV,         glUniformMatrix4fv);
    GL_LOAD_AND_CHECK(GLVERTEXATTRIB2F,           glVertexAttrib2f);
    GL_LOAD_AND_CHECK(GLVERTEXATTRIB3F,           glVertexAttrib3f);
    GL_LOAD_AND_CHECK(GLVERTEXATTRIB4F,           glVertexAttrib4f);
    GL_LOAD_AND_CHECK(GLVERTEXATTRIB3FV,          glVertexAttrib3fv);

    /* 3.0 */
    GL_LOAD_AND_CHECK(GLGENFRAMEBUFFERS,          glGenFramebuffers);
    GL_LOAD_AND_CHECK(GLBINDFRAMEBUFFER,          glBindFramebuffer);
    GL_LOAD_AND_CHECK(GLFRAMEBUFFERTEXTURE2D,     glFramebufferTexture2D);
    GL_LOAD_AND_CHECK(GLCHECKFRAMEBUFFERSTATUS,   glCheckFramebufferStatus);
    GL_LOAD_AND_CHECK(GLGENRENDERBUFFERS,         glGenRenderbuffers);
    GL_LOAD_AND_CHECK(GLBINDRENDERBUFFER,         glBindRenderbuffer);
    GL_LOAD_AND_CHECK(GLRENDERBUFFERSTORAGE,      glRenderbufferStorage);
    GL_LOAD_AND_CHECK(GLFRAMEBUFFERRENDERBUFFER,  glFramebufferRenderbuffer);
    GL_LOAD_AND_CHECK(GLGENERATEMIPMAP,           glGenerateMipmap);

    /* ES 2.0 */
    GL_LOAD_AND_CHECK(GLCLEARDEPTHF, glClearDepthf);
    GL_LOAD_AND_CHECK(GLDEPTHRANGEF, glDepthRangef);

    return true;
}
