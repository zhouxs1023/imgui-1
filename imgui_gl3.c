#include "imgui.h"
#include "stdio.h"

#define SHADER_STRING(a) #a

typedef struct
{
    GLuint triWindowUniform;
    GLuint triVAO;
    GLuint triVBO;
    GLuint triEBO;
    GLuint triProgram;

    GLuint fontWindowUniform;
    GLuint fontVAO;
    GLuint fontVBO;
    GLuint fontProgram;
    GLuint fontTexture;

} GL_State;

void InitGlState(IMContext *im, GL_State *state)
{

    // reused variables for tri, font, line, etc. rendering
    GLuint vertShader;
    GLuint fragShader;
    GLuint program;
    char infoLog[2048];

    ///////////////////////////////////////////////////////////////////
    // Tri and TriStrip shared rendering
    glGenVertexArrays(1, &state->triVAO);
    glBindVertexArray(state->triVAO);

    glGenBuffers(1, &state->triVBO);
    glBindBuffer(GL_ARRAY_BUFFER, state->triVBO);
    //glBufferData(GL_ARRAY_BUFFER, MAX_TRIANGLE_COUNT * 3 * sizeof(PosScissorColor), NULL, GL_STREAM_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        4,
        GL_SHORT,
        GL_FALSE,
        sizeof(PosScissorColor),
        (void *)0 //
    );
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        4,
        GL_SHORT,
        GL_FALSE,
        sizeof(PosScissorColor),
        (void *)(4 * sizeof(uint16_t)) //
    );
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2,
        4,
        GL_UNSIGNED_BYTE,
        GL_TRUE,
        sizeof(PosScissorColor),
        (void *)(8 * sizeof(uint16_t)) //
    );

    glGenBuffers(1, &state->triEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state->triEBO);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(im->drawBuffer.PSCElements), NULL, GL_STREAM_DRAW);

    //const GLchar *triVertSource = R"glsl(
    const GLchar *triVertSource = SHADER_STRING(
#version 330 core \n
        layout(location = 0) in vec4 position;
        layout(location = 1) in vec4 scissor;
        layout(location = 2) in vec4 color;
        out vec4 Color;
        out vec4 Scissor;
        uniform vec2 window;
        void main() {
            float scale_x = 2.f / window.x;
            float scale_y = -2.f / window.y;
            Color = color;
            Scissor = scissor;
            gl_Position = vec4(position.x * scale_x - 1.f, position.y * scale_y + 1.f, 1.0 / (position.z + 1.0), 1.0);
        }
        //)glsl";
    );
    //const GLchar *triFragSource = R"glsl(
    const GLchar *triFragSource = SHADER_STRING(
#version 330 core \n
        in vec4 Color;
        in vec4 Scissor;

        layout(origin_upper_left) in vec4 gl_FragCoord;

        out vec4 outColor;
        void main() {
            if (
                gl_FragCoord.x <= Scissor.x ||
                gl_FragCoord.x >= Scissor.y ||
                gl_FragCoord.y <= Scissor.z ||
                gl_FragCoord.y >= Scissor.w)
            {
                outColor = vec4(0.0, 0.0, 0.0, 0.0);
            }
            else
            {
                outColor = Color;
            }
        });
    //)glsl";

    infoLog[2048];

    printf("Compiling Vert Shader\n");
    vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, &triVertSource, NULL);
    glCompileShader(vertShader);

    glGetShaderInfoLog(vertShader,
                       2048,
                       NULL,
                       infoLog);
    infoLog[2047] = 0;
    printf("%s\n", infoLog);

    printf("Compiling Frag Shader\n");
    fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &triFragSource, NULL);
    glCompileShader(fragShader);

    glGetShaderInfoLog(fragShader,
                       2048,
                       NULL,
                       infoLog);
    infoLog[2047] = 0;
    printf("%s", infoLog);

    printf("Linking Program\n");
    program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);
    glGetProgramInfoLog(program, 2048, NULL, infoLog);
    printf("%s\n", infoLog);

    state->triProgram = program;

    glDetachShader(program, vertShader);
    glDetachShader(program, fragShader);

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    state->triWindowUniform = glGetUniformLocation(program, "window");

    ///////////////////////////////////////////////////////////////////
    // Font rendering
    glGenVertexArrays(1, &state->fontVAO);
    glBindVertexArray(state->fontVAO);

    glGenBuffers(1, &state->fontVBO);
    glBindBuffer(GL_ARRAY_BUFFER, state->fontVBO);
    glBufferData(GL_ARRAY_BUFFER, MAX_GLYPH_INSTANCES * 6 * sizeof(GlyphVert), NULL, GL_STREAM_DRAW);

    // Shader sources
    //const GLchar *fontVertSource = R"glsl(
    const GLchar *fontVertSource = SHADER_STRING(
#version 330 core \n
        layout(location = 0) in vec3 position;
        layout(location = 1) in vec4 scissor;
        layout(location = 2) in vec2 texCoords;
        layout(location = 3) in vec4 color;

        out vec4 Color;
        out vec4 Scissor;
        out vec2 TexCoords;

        uniform vec2 window;

        void main() {
            float scale_x = 2.f / window.x;
            float scale_y = -2.f / window.y;
            Color = color;
            Scissor = scissor;
            TexCoords = texCoords;
            gl_Position = vec4(position.x * scale_x - 1.f, position.y * scale_y + 1.f, 1.0 / (position.z + 1.0), 1.0);
        });
    //)glsl";
    //const GLchar *fontFragSource = R"glsl(
    const GLchar *fontFragSource = SHADER_STRING(
#version 330 core \n
        in vec4 Color;
        in vec4 Scissor;
        in vec2 TexCoords;
        layout(origin_upper_left) in vec4 gl_FragCoord;

        out vec4 outColor;

        uniform sampler2D fontBitmap;

        void main() {
            if (
                gl_FragCoord.x <= Scissor.x ||
                gl_FragCoord.x >= Scissor.y ||
                gl_FragCoord.y <= Scissor.z ||
                gl_FragCoord.y >= Scissor.w)
            {
                outColor = vec4(0.0, 0.0, 0.0, 0.0);
                //outColor = Color;
            }
            else
            {
                outColor = vec4(Color.xyz, Color.w * texture(fontBitmap, TexCoords).r);
                //outColor = Color;
            }
        });
    //)glsl";

    infoLog[2048];

    printf("Compiling Font Vert Shader\n");
    vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, &fontVertSource, NULL);
    glCompileShader(vertShader);

    glGetShaderInfoLog(vertShader,
                       2048,
                       NULL,
                       infoLog);
    infoLog[2047] = 0;
    printf("%s\n", infoLog);

    printf("Compiling Font Frag Shader\n");
    fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fontFragSource, NULL);
    glCompileShader(fragShader);

    glGetShaderInfoLog(fragShader,
                       2048,
                       NULL,
                       infoLog);
    infoLog[2047] = 0;
    printf("%s", infoLog);

    printf("Linking Font Program\n");
    program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);
    glGetProgramInfoLog(program, 2048, NULL, infoLog);
    printf("%s\n", infoLog);

    state->fontProgram = program;

    glDetachShader(program, vertShader);
    glDetachShader(program, fragShader);

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(GlyphVert),
        (void *)0 //
    );
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        4,
        GL_SHORT,
        GL_FALSE,
        sizeof(GlyphVert),
        (void *)(3 * sizeof(float)) //
    );
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(GlyphVert),
        (void *)(3 * sizeof(float) + 4 * sizeof(uint16_t)) //
    );
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(
        3,
        4,
        GL_UNSIGNED_BYTE,
        GL_TRUE,
        sizeof(GlyphVert),
        (void *)(3 * sizeof(float) + 4 * sizeof(uint16_t) + 2 * sizeof(float)) //
    );
    glGenTextures(1, &state->fontTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, state->fontTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 512, 512, 0, GL_RED, GL_UNSIGNED_BYTE, im->fontBitmap);
    free(im->fontBitmap);
    //glUniform1i(glGetUniformLocation(program, "fontBitmap"), 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    state->fontWindowUniform = glGetUniformLocation(program, "window");
}

void DrawContext(IMContext *im, GL_State *state)
{
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(PRIM_RESTART);
    glDisable(GL_CULL_FACE);

    /////////////////////////////////////////////////////////////////
    // Tri rendering
    if (im->drawBuffer.triCount > 0 || im->drawBuffer.triStripVertCount > 0)
    {
        glBindVertexArray(state->triVAO);
        glUseProgram(state->triProgram);
        glUniform2f(state->triWindowUniform, im->windowWidth, im->windowHeight);
        glBindBuffer(GL_ARRAY_BUFFER, state->triVBO);
#if 1
        if (im->drawBuffer.triCount > 0)
        {
            glBufferData(GL_ARRAY_BUFFER, MAX_TRIANGLE_COUNT * 3 * sizeof(PosScissorColor), NULL, GL_STREAM_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, 0, im->drawBuffer.triCount * 3 * sizeof(PosScissorColor), im->drawBuffer.triVerts);
            glDrawArrays(GL_TRIANGLES, 0, im->drawBuffer.triCount * 3);
        }
        if (im->drawBuffer.triStripVertCount > 0)
        {
            glBufferData(GL_ARRAY_BUFFER, im->drawBuffer.triStripVertCount * sizeof(PosScissorColor), im->drawBuffer.triStripVerts, GL_STREAM_DRAW);
            //glBufferData(GL_ARRAY_BUFFER, MAX_TRISTRIP_VERT_COUNT * sizeof(PosScissorColor), NULL, GL_STREAM_DRAW);
            //glBufferSubData(GL_ARRAY_BUFFER, 0, im->drawBuffer.triStripVertCount * sizeof(PosScissorColor), im->drawBuffer.triStripVerts);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, im->drawBuffer.triStripVertCount);
        }
#endif
        if (im->drawBuffer.PSCVertCount > 0)
        {
            glBufferData(GL_ARRAY_BUFFER, im->drawBuffer.PSCVertCount * sizeof(PosScissorColor), im->drawBuffer.PSCVerts, GL_STREAM_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state->triEBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, im->drawBuffer.PSCElementCount * sizeof(uint16_t), im->drawBuffer.PSCElements, GL_STREAM_DRAW);
            //glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_PSC_ELEMENT_COUNT * sizeof(uint16_t), NULL, GL_STREAM_DRAW);
            //glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, im->drawBuffer.PSCElementCount * sizeof(uint16_t), im->drawBuffer.PSCElements);
            glDrawElements(GL_TRIANGLE_STRIP, im->drawBuffer.PSCElementCount, GL_UNSIGNED_SHORT, 0);
        }
    }

    /////////////////////////////////////////////////////////////////
    // Font Rndering
    if (im->drawBuffer.glyphCount > 0)
    {
        glBindVertexArray(state->fontVAO);
        glUseProgram(state->fontProgram);
        glUniform2f(state->fontWindowUniform, im->windowWidth, im->windowHeight);
        glBindBuffer(GL_ARRAY_BUFFER, state->fontVBO);
        glBufferData(GL_ARRAY_BUFFER, im->drawBuffer.glyphCount * 6 * sizeof(GlyphVert), im->drawBuffer.glyphVerts, GL_STREAM_DRAW);
        //glBufferData(GL_ARRAY_BUFFER, MAX_GLYPH_INSTANCES * 6 * sizeof(GlyphVert), NULL, GL_STREAM_DRAW);
        //glBufferSubData(GL_ARRAY_BUFFER, 0, im->drawBuffer.glyphCount * 6 * sizeof(GlyphVert), im->drawBuffer.glyphVerts);
        glDrawArrays(GL_TRIANGLES, 0, im->drawBuffer.glyphCount * 6);
    }

    ResetDrawBuffer(&im->drawBuffer);
}
