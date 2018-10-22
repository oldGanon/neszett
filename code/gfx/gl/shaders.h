#define GLSL_FRAG_PRECISION \
    "\n#if (GL_FRAGMENT_PRECISION_HIGH == 1)\n" \
    "precision highp float;" \
    "\n#else\n" \
    "precision mediump float;" \
    "\n#endif\n"

#define GLSL_VERSION "#version 130\n"
#define GLSL_VERT_POSITION "P"
#define GLSL_UNIFORM_TEXTURE "T"
#define GLSL_UNIFORM_PALETTE "PAL"

const char* BlitShaderV = 
    GLSL_VERSION
    "attribute vec2 " GLSL_VERT_POSITION ";"
    "varying vec2 UV;"
    "void main(){"
    "    gl_Position = vec4(" GLSL_VERT_POSITION "*2.0-1.0,0.0,1.0);"
    "    UV = " GLSL_VERT_POSITION ";"
    "    UV.y = 1.0 - UV.y;"
    "}";

const char *BlitShaderF =
    GLSL_VERSION
    GLSL_FRAG_PRECISION
    "varying vec2 UV;"
    "uniform sampler2D " GLSL_UNIFORM_TEXTURE ";"
    "uniform sampler2D " GLSL_UNIFORM_PALETTE ";"
    "void main(){"
        "float i = texture2D(" GLSL_UNIFORM_TEXTURE ",UV).r * 4.0;"
        "gl_FragColor = texture2D(" GLSL_UNIFORM_PALETTE ",vec2(i,0.5));"
    "}";

static u32
OpenGL_CompileShader(u32 Type, const char *Shader)
{
    u32 ShaderID = glCreateShader(Type);
    glShaderSource(ShaderID, 1, &Shader, 0);
    glCompileShader(ShaderID);
    i32 ShaderCompiled = GL_FALSE;
    glGetShaderiv(ShaderID, GL_COMPILE_STATUS, &ShaderCompiled);
    if (ShaderCompiled != GL_TRUE)
    {
        i32 LogLength;
        glGetShaderiv(ShaderID, GL_INFO_LOG_LENGTH, &LogLength);
        char Error[1024] = { };
        glGetShaderInfoLog(ShaderID, 1024, &LogLength, Error);
        glDeleteShader(ShaderID);
        Api_Error(String(Error, LogLength));
        return 0;
    }
    return ShaderID;
}

static u32
OpenGL_LoadProgram(const char* Vertex, const char* Fragment)
{
    /* COMPILE SHADERS */
    u32 VertexShader = OpenGL_CompileShader(GL_VERTEX_SHADER, Vertex);
    if (!VertexShader) return 0;
    u32 FragmentShader = OpenGL_CompileShader(GL_FRAGMENT_SHADER, Fragment);
    if (!FragmentShader) return 0;

    /* CREATE PROGRAM */
    u32 ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShader);
    glAttachShader(ProgramID, FragmentShader);

    /* LINK PROGRAM */
    glLinkProgram(ProgramID);
    i32 ProgramSuccess = GL_TRUE;
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &ProgramSuccess);
    if (ProgramSuccess != GL_TRUE)
    {
        i32 LogLength;
        glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &LogLength);
        char Error[1024] = { };
        glGetProgramInfoLog(ProgramID, 1024, &LogLength, Error);
        glDeleteProgram(ProgramID);
        Api_Error(String(Error, LogLength));
        return 0;
    }

    /* LINK ATTRIBS AND SET UNIFORMS */
    glUseProgram(ProgramID);
    glBindAttribLocation(ProgramID, 0, GLSL_VERT_POSITION);
    glLinkProgram(ProgramID);
    glUniform1i(glGetUniformLocation(ProgramID, GLSL_UNIFORM_TEXTURE), 0);
    glUniform1i(glGetUniformLocation(ProgramID, GLSL_UNIFORM_PALETTE), 1);
    glUseProgram(0);

    /* CLEAN UP */
    glDetachShader(ProgramID, VertexShader);
    glDetachShader(ProgramID, FragmentShader);
    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);

    return ProgramID;
}
