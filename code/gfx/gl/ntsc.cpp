#define NTSC_CHROMA_CYCLES_PER_PIXEL            (1.0 / 12.0)

#define NTSC_CHROMA_AMPLITUDE                   1.0
#define NTSC_CHROMA_FREQ_BAND_WIDTH_OVER_TWO    (1.0f / (f32)NTSC_CHROMA_AMPLITUDE)

#define NTSC_CHROMA_FREQ_CUTOFF_LUMA            0.5f
#define NTSC_CHROMA_FREQ_CUTOFF_HIGH            (1.0f + NTSC_CHROMA_FREQ_BAND_WIDTH_OVER_TWO)
#define NTSC_CHROMA_FREQ_CUTOFF_LOW             (1.0f - NTSC_CHROMA_FREQ_BAND_WIDTH_OVER_TWO)
#define NTSC_CHROMA_SAMPLING_RATE               (1.0 / NTSC_CHROMA_CYCLES_PER_PIXEL)
#define NTSC_CHROMA_FREQ_CUTOFF_LUMA_NORM       ((2.0f * NTSC_CHROMA_FREQ_CUTOFF_LUMA) / (f32)NTSC_CHROMA_SAMPLING_RATE)
#define NTSC_CHROMA_FREQ_CUTOFF_HIGH_NORM       ((2.0f * NTSC_CHROMA_FREQ_CUTOFF_HIGH) / (f32)NTSC_CHROMA_SAMPLING_RATE)
#define NTSC_CHROMA_FREQ_CUTOFF_LOW_NORM        ((2.0f * NTSC_CHROMA_FREQ_CUTOFF_LOW) / (f32)NTSC_CHROMA_SAMPLING_RATE)
#define BLACKMAN_WINDOW_N                       ((u32)(6 * NTSC_CHROMA_SAMPLING_RATE) & 0xFFFFFFFE)
#define FILTER_SIZE                             71 // BLACKMAN_WINDOW_N - 1

#define NTSC_GAMMA                              2.2

static f32
NTSC_Lowpass(i32 i)
{
    f32 n = (f32)i;
    f32 Window = 0.42f - 0.5f * Cos(n * (2.0f * PI / BLACKMAN_WINDOW_N)) + 0.08f * Cos(n * (4.0f * PI / BLACKMAN_WINDOW_N));
    f32 Low = NTSC_CHROMA_FREQ_CUTOFF_LUMA_NORM * Sinc((n - 0.5f * BLACKMAN_WINDOW_N) * (NTSC_CHROMA_FREQ_CUTOFF_LUMA_NORM));
    return Window * Low;
}

static f32
NTSC_Bandpass(i32 i)
{
    f32 n = (f32)i;
    f32 Window = 0.42f - 0.5f * Cos(n * (2.0f * PI / BLACKMAN_WINDOW_N)) + 0.08f * Cos(n * (4.0f * PI / BLACKMAN_WINDOW_N));
    f32 High = NTSC_CHROMA_FREQ_CUTOFF_HIGH_NORM * Sinc((n - 0.5f * BLACKMAN_WINDOW_N) * (NTSC_CHROMA_FREQ_CUTOFF_HIGH_NORM));
    f32 Low = NTSC_CHROMA_FREQ_CUTOFF_LOW_NORM * Sinc((n - 0.5f * BLACKMAN_WINDOW_N) * (NTSC_CHROMA_FREQ_CUTOFF_LOW_NORM));
    return Window * (High - Low);
}

const char *NtscModulationShaderF = 
    // GLSL_VERSION
    "#version 150\n"
    GLSL_FRAG_PRECISION
    "varying vec2 UV;"
#if OPENGL_USETEXTUREBUFFER
    "uniform samplerBuffer " GLSL_UNIFORM_TEXTURE ";"
#else
    "uniform sampler2D " GLSL_UNIFORM_TEXTURE ";"
#endif
    "uniform float Phase;"
    "const float Los[4] = float[4](0.350, 0.518, 0.962, 1.550);"
    "const float His[4] = float[4](1.094, 1.506, 1.962, 1.962);"
    "const float Black = 0.518;"
    "const float White = 1.962;"
    "void main(){"
        "float n = floor(gl_FragCoord.x) - floor(mod(gl_FragCoord.y, 3.0)) * 4.0;"
        "n += Phase * 8.0;"
#if OPENGL_USETEXTUREBUFFER
        "vec2 F = clamp(UV, 0.0, 1.0);"
        "float Pixel = texelFetch(" GLSL_UNIFORM_TEXTURE ", int(F.x * 258.0) + int(F.y * 226.0) * 260).r * 255.0;"
#else
        "float Pixel = texture2D(" GLSL_UNIFORM_TEXTURE ", UV).r * 255.0;"
#endif
        "float Color = floor(mod(Pixel, 16.0));"
        "int Level = int(mod(Pixel / 16.0, 4.0));"
        "float Emphasis = floor(Pixel / 64.0);"
        "float Lo = Los[Level];"
        "float Hi = His[Level];"
        "if (Color == 0.0) Lo = Hi;"
        "if (Color > 12.0) Hi = Lo;"
        "float High = floor(mod(Color + n, 12.0) / 6.0);"
        "float Signal = mix(Lo, Hi, High);"
        "Signal = (Signal-Black) / (White-Black);"
        "gl_FragColor = vec4(Signal, 0.0, 0.0, 0.0);"
    "}\0";

const char *NtscDemodulationShaderF = 
    GLSL_VERSION
    GLSL_FRAG_PRECISION
    "varying vec2 UV;"
    "uniform sampler2D " GLSL_UNIFORM_NTSC ";"
    "uniform float LowpassLUT[" MACRO_STRING(FILTER_SIZE) "];"
    "uniform float BandpassLUT[" MACRO_STRING(FILTER_SIZE) "];"
    "uniform float Phase;"
    "const int FIR = " MACRO_STRING(FILTER_SIZE) ";"
    "const float PI = 3.14159265359;"
    "vec2 Osc(float x){"
        "x -= mod(gl_FragCoord.y, 3.0) * 4.0 - 3.0;"
        "x += Phase * 8.0;"
        "x = x*(" MACRO_STRING(NTSC_CHROMA_CYCLES_PER_PIXEL) "*2.0*PI);"
        "return vec2(sin(x),-cos(x));"
    "}"
    "void main(){"
        "float n = floor(gl_FragCoord.x);"
        "vec2 O = Osc(n);"
        "float Y = 0.0;"
        "float IQ = 0.0;"
        "n -= float(FIR)/2.0;"
        "for (int i = 0; i < FIR; ++i){"
            "vec2 S = vec2((n + float(i)) / (312.0 * 8.0), UV.y);"
            "Y += texture2D(" GLSL_UNIFORM_NTSC ", S).r * LowpassLUT[i];"
            "IQ += texture2D(" GLSL_UNIFORM_NTSC ", S).r * BandpassLUT[i];"
        "}"

        "vec2 IQSig = IQ * O;"
        "gl_FragColor = vec4(Y, IQSig, 0.0);"
    "}\0";

const char *NtscChromaShaderF = 
    GLSL_VERSION
    GLSL_FRAG_PRECISION
    "varying vec2 UV;"
    "uniform sampler2D " GLSL_UNIFORM_NTSC ";"
    "uniform float LowpassLUT[" MACRO_STRING(FILTER_SIZE) "];"
    "const int FIR = " MACRO_STRING(FILTER_SIZE) ";"
    "const float PI = 3.14159265359;"
    "const mat3 YIQ2RGB = mat3(1.000, 1.000, 1.000,"
                              "0.956,-0.272,-1.106,"
                              "0.621,-0.647, 1.703);"

    "void main(){"
        "float Luma = texture2D(" GLSL_UNIFORM_NTSC ", UV).r;"
        "vec2 Chroma = vec2(0.0);"
        "float n = floor(gl_FragCoord.x);"
        "n -= float(FIR)/2.0;"
        "for (int i = 0; i < FIR; ++i){"
            "vec2 S = vec2((n + float(i)) / (312.0 * 8.0), UV.y);"
            "Chroma += texture2D(" GLSL_UNIFORM_NTSC ", S).yz * LowpassLUT[i];"
        "}"
        "vec3 Color = YIQ2RGB * vec3(Luma, Chroma * " MACRO_STRING(NTSC_CHROMA_AMPLITUDE) ");"
        "gl_FragColor.rgb = pow(Color, vec3(1.0/" MACRO_STRING(NTSC_GAMMA) "));"
        "gl_FragColor.a = 1.0;"
    "}\0";

const char* NtscBlitShaderF = 
    GLSL_VERSION
    GLSL_FRAG_PRECISION
    "varying vec2 UV;"
    "uniform sampler2D " GLSL_UNIFORM_NTSC ";"
    "float Gaus(float pos,float scale){return exp2(scale*pos*pos);}"
    "void main(){"
        "vec2 Res = vec2(256.0,226.0);"
        "float Y = (floor(UV.y * Res.y) + 0.5) / Res.y;"
        "float fY = (UV.y - Y) * Res.y;"
        "float X = UV.x;"
        "if (X < 0.0 || X > 280.0 / 320.0) X = 300.0 / 320.0;"
        "gl_FragColor.rgb = texture2D(" GLSL_UNIFORM_NTSC ",vec2(X,Y)).rgb;"
        "gl_FragColor.rgb *= Gaus(fY,-12.0)*0.5+0.5;"
    "}\0";

static void
NTSC_Init()
{
    u64 filtersize = FILTER_SIZE;
    f32 Norm = 0.0f;
    f32 LP[FILTER_SIZE] = { };
    for (i32 i = 0; i < FILTER_SIZE; ++i)
    {
        LP[i] = NTSC_Lowpass(i);
        Norm += LP[i];
    }
    Norm = 0.0f;
    f32 BP[FILTER_SIZE] = { };
    for (i32 i = 0; i < FILTER_SIZE; ++i)
    {
        BP[i] = NTSC_Bandpass(i);
        Norm += BP[i];
    }

    {   /* MODULATION SHADER */
        GL.NtscShader[0] = OpenGL_LoadProgram(BlitShaderFlippedV, NtscModulationShaderF);
        glUseProgram(GL.NtscShader[0]);
        GL.NtscPhaseUniform[0] = glGetUniformLocation(GL.NtscShader[0], "Phase");
        glUseProgram(0);
    }

    {   /* DEMODULATION SHADER */
        GL.NtscShader[1] = OpenGL_LoadProgram(BlitShaderV, NtscDemodulationShaderF);
        glUseProgram(GL.NtscShader[1]);
        i32 UniformLocation = glGetUniformLocation(GL.NtscShader[1], "LowpassLUT[0]");
        if (UniformLocation != -1) glUniform1fv(UniformLocation, FILTER_SIZE, LP);
        UniformLocation = glGetUniformLocation(GL.NtscShader[1], "BandpassLUT[0]");
        if (UniformLocation != -1) glUniform1fv(UniformLocation, FILTER_SIZE, BP);
        GL.NtscPhaseUniform[1] = UniformLocation = glGetUniformLocation(GL.NtscShader[1], "Phase");
        glUseProgram(0);
    }

    {   /* CHROMA SHADER */
        GL.NtscShader[2] = OpenGL_LoadProgram(BlitShaderV, NtscChromaShaderF);
        glUseProgram(GL.NtscShader[2]);
        i32 UniformLocation = glGetUniformLocation(GL.NtscShader[2], "LowpassLUT[0]");
        if (UniformLocation != -1) glUniform1fv(UniformLocation, FILTER_SIZE, LP);
        glUseProgram(0);
    }

    {   /* CRT BLIT SHADER */
        GL.NtscShader[3] = OpenGL_LoadProgram(BlitShaderFittedV, NtscBlitShaderF);
    }

    glGenFramebuffers(1, &GL.NtscFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, GL.NtscFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenTextures(3, GL.NtscTextures);
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, GL.NtscTextures[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, 312 * 8, 226, 0, GL_RED, GL_FLOAT, 0);

    glBindTexture(GL_TEXTURE_2D, GL.NtscTextures[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 312 * 8, 226, 0, GL_RGB, GL_FLOAT, 0);

    glBindTexture(GL_TEXTURE_2D, GL.NtscTextures[2]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, 312 * 8, 226, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
}

static void
NTSC_Blit(ivec2 WindowDim)
{
    f32 Phase = (f32)Atomic_Get(&GlobalPhase);

    /* MODULATE */
    glScissor(0, 0, 312 * 8, 226);
    glViewport(0, 0, 312 * 8, 226);
    glBindFramebuffer(GL_FRAMEBUFFER, GL.NtscFramebuffer);
    glUseProgram(GL.NtscShader[0]);
    glUniform1f(GL.NtscPhaseUniform[0], Phase);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                           GL_TEXTURE_2D, GL.NtscTextures[0], 0);
    OpenGL_DrawFullscreenQuad();

    /* DEMODULATE */
    glUseProgram(GL.NtscShader[1]);
    glActiveTexture(GL_TEXTURE0 + 2);
    glUniform1f(GL.NtscPhaseUniform[1], Phase);
    glBindTexture(GL_TEXTURE_2D, GL.NtscTextures[0]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                           GL_TEXTURE_2D, GL.NtscTextures[1], 0);
    OpenGL_DrawFullscreenQuad();

    /* CHROMA */
    glUseProgram(GL.NtscShader[2]);
    glBindTexture(GL_TEXTURE_2D, GL.NtscTextures[1]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                           GL_TEXTURE_2D, GL.NtscTextures[2], 0);
    OpenGL_DrawFullscreenQuad();

    /* BLIT */
    glViewport(0, 0, WindowDim.x, WindowDim.y);
    glScissor(0, 0, WindowDim.x, WindowDim.y);
    glUseProgram(GL.NtscShader[3]);
    if (WindowDim.x != GL.WindowDim.x || WindowDim.y != GL.WindowDim.y)
        glUniform2f(glGetUniformLocation(GL.NtscShader[3], "WindowRes"), (f32)WindowDim.x, (f32)WindowDim.y);
    glBindTexture(GL_TEXTURE_2D, GL.NtscTextures[2]);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    OpenGL_DrawFullscreenQuad();
}
