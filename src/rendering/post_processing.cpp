#include "post_processing.h"
#include "rendering/shader.h"
#include "rendering/texture.h"
#include "core/logger.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>
#include <random>

namespace vge {

// ============================================
// Full-Screen Quad Helpers
// ============================================

static float quadVertices[] = {
    // positions   // texCoords
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
};

static void CreateFullscreenQuad(uint32_t& vao, uint32_t& vbo) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);
}

// ============================================
// Tone Mapping
// ============================================

void ToneMappingEffect::Initialize(uint32_t width, uint32_t height) {
    CreateFullscreenQuad(vao, vbo);
    
    // ACES Filmic tone mapping shader
    const char* vertexShader = R"(
        #version 330 core
        layout(location = 0) in vec2 aPos;
        layout(location = 1) in vec2 aTexCoord;
        out vec2 TexCoord;
        void main() {
            gl_Position = vec4(aPos, 0.0, 1.0);
            TexCoord = aTexCoord;
        }
    )";
    
    const char* fragmentShader = R"(
        #version 330 core
        in vec2 TexCoord;
        out vec4 FragColor;
        uniform sampler2D screenTexture;
        uniform float exposure;
        uniform float gamma;
        uniform int toneOperator;
        
        vec3 ACESToneMapping(vec3 color) {
            const float A = 2.51f;
            const float B = 0.03f;
            const float C = 2.43f;
            const float D = 0.59f;
            const float E = 0.14f;
            return clamp((color * (A * color + B)) / (color * (C * color + D) + E), 0.0, 1.0);
        }
        
        vec3 ReinhardToneMapping(vec3 color) {
            return color / (color + vec3(1.0));
        }
        
        vec3 Uncharted2ToneMapping(vec3 color) {
            const float A = 0.15;
            const float B = 0.50;
            const float C = 0.10;
            const float D = 0.20;
            const float E = 0.02;
            const float F = 0.30;
            const float W = 11.2;
            color *= exposure;
            vec3 curr = ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
            vec3 whiteScale = 1.0 / (((vec3(W) * (A * vec3(W) + C * B) + D * E) / (vec3(W) * (A * vec3(W) + B) + D * F)) - E / F);
            return curr * whiteScale;
        }
        
        void main() {
            vec3 color = texture(screenTexture, TexCoord).rgb;
            color *= exposure;
            
            if (toneOperator == 0) {
                color = ACESToneMapping(color);
            } else if (toneOperator == 1) {
                color = ReinhardToneMapping(color);
            } else if (toneOperator == 2) {
                color = Uncharted2ToneMapping(color);
            }
            
            // Gamma correction
            color = pow(color, vec3(1.0 / gamma));
            
            FragColor = vec4(color, 1.0);
        }
    )";
    
    shader = std::make_unique<Shader>();
    shader->Compile(vertexShader, fragmentShader);
    
    Logger::Info("[PostProcess] Tone mapping initialized");
}

void ToneMappingEffect::Shutdown() {
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    shader.reset();
}

void ToneMappingEffect::Render(uint32_t inputTexture, uint32_t outputFBO,
                                uint32_t width, uint32_t height) {
    if (!enabled || !shader) return;
    
    glBindFramebuffer(GL_FRAMEBUFFER, outputFBO);
    glViewport(0, 0, width, height);
    
    shader->Use();
    shader->SetInt("screenTexture", 0);
    shader->SetFloat("exposure", exposure);
    shader->SetFloat("gamma", gamma);
    shader->SetInt("toneOperator", static_cast<int>(toneOperator));
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTexture);
    
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

// ============================================
// Bloom
// ============================================

void BloomEffect::Initialize(uint32_t width, uint32_t height) {
    CreateFullscreenQuad(vao, vbo);
    
    // Downsample shader
    const char* downsampleVert = R"(
        #version 330 core
        layout(location = 0) in vec2 aPos;
        layout(location = 1) in vec2 aTexCoord;
        out vec2 TexCoord;
        void main() {
            gl_Position = vec4(aPos, 0.0, 1.0);
            TexCoord = aTexCoord;
        }
    )";
    
    const char* downsampleFrag = R"(
        #version 330 core
        in vec2 TexCoord;
        out vec4 FragColor;
        uniform sampler2D srcTexture;
        uniform vec2 srcResolution;
        
        void main() {
            vec2 texelSize = 1.0 / srcResolution;
            vec3 result = vec3(0.0);
            
            // 4-tap bilinear filter
            vec2 offset = texelSize * 0.5;
            result += texture(srcTexture, TexCoord + vec2(-offset.x, -offset.y)).rgb;
            result += texture(srcTexture, TexCoord + vec2( offset.x, -offset.y)).rgb;
            result += texture(srcTexture, TexCoord + vec2(-offset.x,  offset.y)).rgb;
            result += texture(srcTexture, TexCoord + vec2( offset.x,  offset.y)).rgb;
            
            FragColor = vec4(result * 0.25, 1.0);
        }
    )";
    
    downsampleShader = std::make_unique<Shader>();
    downsampleShader->Compile(downsampleVert, downsampleFrag);
    
    // Upsample shader
    const char* upsampleFrag = R"(
        #version 330 core
        in vec2 TexCoord;
        out vec4 FragColor;
        uniform sampler2D srcTexture;
        uniform float filterRadius;
        
        void main() {
            vec2 texelSize = 1.0 / vec2(textureSize(srcTexture, 0));
            vec3 result = vec3(0.0);
            
            // 9-tap tent filter
            for (int x = -1; x <= 1; ++x) {
                for (int y = -1; y <= 1; ++y) {
                    vec2 offset = vec2(float(x), float(y)) * texelSize * filterRadius;
                    result += texture(srcTexture, TexCoord + offset).rgb;
                }
            }
            
            FragColor = vec4(result / 9.0, 1.0);
        }
    )";
    
    upsampleShader = std::make_unique<Shader>();
    upsampleShader->Compile(downsampleVert, upsampleFrag);
    
    // Combine shader
    const char* combineFrag = R"(
        #version 330 core
        in vec2 TexCoord;
        out vec4 FragColor;
        uniform sampler2D sceneTexture;
        uniform sampler2D bloomTexture;
        uniform float intensity;
        
        void main() {
            vec3 scene = texture(sceneTexture, TexCoord).rgb;
            vec3 bloom = texture(bloomTexture, TexCoord).rgb;
            FragColor = vec4(scene + bloom * intensity, 1.0);
        }
    )";
    
    combineShader = std::make_unique<Shader>();
    combineShader->Compile(downsampleVert, combineFrag);
    
    // Create mip chain
    uint32_t mipCount = 5;
    mips.resize(mipCount);
    
    uint32_t mipWidth = width / 2;
    uint32_t mipHeight = height / 2;
    
    for (uint32_t i = 0; i < mipCount; ++i) {
        mips[i].width = mipWidth;
        mips[i].height = mipHeight;
        
        glGenFramebuffers(1, &mips[i].fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, mips[i].fbo);
        
        glGenTextures(1, &mips[i].texture);
        glBindTexture(GL_TEXTURE_2D, mips[i].texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, mipWidth, mipHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mips[i].texture, 0);
        
        mipWidth /= 2;
        mipHeight /= 2;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    Logger::Info("[PostProcess] Bloom initialized with " + std::to_string(mipCount) + " mips");
}

void BloomEffect::Shutdown() {
    for (auto& mip : mips) {
        if (mip.fbo) glDeleteFramebuffers(1, &mip.fbo);
        if (mip.texture) glDeleteTextures(1, &mip.texture);
    }
    mips.clear();
    
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    
    downsampleShader.reset();
    upsampleShader.reset();
    combineShader.reset();
}

void BloomEffect::Render(uint32_t inputTexture, uint32_t outputFBO,
                          uint32_t width, uint32_t height) {
    if (!enabled || mips.empty()) return;
    
    // Downsample
    Downsample(inputTexture);
    
    // Upsample
    Upsample();
    
    // Combine with scene
    glBindFramebuffer(GL_FRAMEBUFFER, outputFBO);
    glViewport(0, 0, width, height);
    
    combineShader->Use();
    combineShader->SetInt("sceneTexture", 0);
    combineShader->SetInt("bloomTexture", 1);
    combineShader->SetFloat("intensity", intensity);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mips[0].texture);
    
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void BloomEffect::Downsample(uint32_t sourceTexture) {
    downsampleShader->Use();
    
    for (size_t i = 0; i < mips.size(); ++i) {
        glBindFramebuffer(GL_FRAMEBUFFER, mips[i].fbo);
        glViewport(0, 0, mips[i].width, mips[i].height);
        
        downsampleShader->SetInt("srcTexture", 0);
        downsampleShader->SetVec2("srcResolution", 
            glm::vec2(i == 0 ? width : mips[i-1].width, i == 0 ? height : mips[i-1].height));
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, i == 0 ? sourceTexture : mips[i-1].texture);
        
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}

void BloomEffect::Upsample() {
    upsampleShader->Use();
    upsampleShader->SetFloat("filterRadius", radius);
    
    // Enable additive blending for upsampling
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glBlendEquation(GL_FUNC_ADD);
    
    for (int i = static_cast<int>(mips.size()) - 1; i > 0; --i) {
        glBindFramebuffer(GL_FRAMEBUFFER, mips[i-1].fbo);
        glViewport(0, 0, mips[i-1].width, mips[i-1].height);
        
        upsampleShader->SetInt("srcTexture", 0);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mips[i].texture);
        
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    
    glDisable(GL_BLEND);
}

// ============================================
// FXAA
// ============================================

void FXAAEffect::Initialize(uint32_t width, uint32_t height) {
    CreateFullscreenQuad(vao, vbo);
    
    const char* vertexShader = R"(
        #version 330 core
        layout(location = 0) in vec2 aPos;
        layout(location = 1) in vec2 aTexCoord;
        out vec2 TexCoord;
        void main() {
            gl_Position = vec4(aPos, 0.0, 1.0);
            TexCoord = aTexCoord;
        }
    )";
    
    const char* fragmentShader = R"(
        #version 330 core
        in vec2 TexCoord;
        out vec4 FragColor;
        uniform sampler2D screenTexture;
        uniform vec2 inverseScreenSize;
        uniform float quality;
        uniform float edgeThreshold;
        
        void main() {
            vec2 texelSize = inverseScreenSize;
            vec3 colorCenter = texture(screenTexture, TexCoord).rgb;
            
            // Sample neighbors
            vec3 colorN = texture(screenTexture, TexCoord + vec2(0.0, texelSize.y)).rgb;
            vec3 colorS = texture(screenTexture, TexCoord + vec2(0.0, -texelSize.y)).rgb;
            vec3 colorE = texture(screenTexture, TexCoord + vec2(texelSize.x, 0.0)).rgb;
            vec3 colorW = texture(screenTexture, TexCoord + vec2(-texelSize.x, 0.0)).rgb;
            
            // Luma
            float lumaCenter = dot(colorCenter, vec3(0.299, 0.587, 0.114));
            float lumaN = dot(colorN, vec3(0.299, 0.587, 0.114));
            float lumaS = dot(colorS, vec3(0.299, 0.587, 0.114));
            float lumaE = dot(colorE, vec3(0.299, 0.587, 0.114));
            float lumaW = dot(colorW, vec3(0.299, 0.587, 0.114));
            
            float lumaMin = min(lumaCenter, min(min(lumaN, lumaS), min(lumaE, lumaW)));
            float lumaMax = max(lumaCenter, max(max(lumaN, lumaS), max(lumaE, lumaW)));
            float lumaRange = lumaMax - lumaMin;
            
            if (lumaRange < max(edgeThreshold, lumaMax * edgeThreshold)) {
                FragColor = vec4(colorCenter, 1.0);
                return;
            }
            
            // Simple 4-tap average for edges
            vec3 result = (colorCenter * 4.0 + colorN + colorS + colorE + colorW) / 8.0;
            FragColor = vec4(result, 1.0);
        }
    )";
    
    shader = std::make_unique<Shader>();
    shader->Compile(vertexShader, fragmentShader);
    
    Logger::Info("[PostProcess] FXAA initialized");
}

void FXAAEffect::Shutdown() {
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    shader.reset();
}

void FXAAEffect::Render(uint32_t inputTexture, uint32_t outputFBO,
                         uint32_t width, uint32_t height) {
    if (!enabled || !shader) return;
    
    glBindFramebuffer(GL_FRAMEBUFFER, outputFBO);
    glViewport(0, 0, width, height);
    
    shader->Use();
    shader->SetInt("screenTexture", 0);
    shader->SetVec2("inverseScreenSize", glm::vec2(1.0f / width, 1.0f / height));
    shader->SetFloat("quality", quality);
    shader->SetFloat("edgeThreshold", edgeThreshold);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTexture);
    
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

// ============================================
// SSAO
// ============================================

void SSAOEffect::Initialize(uint32_t width, uint32_t height) {
    CreateFullscreenQuad(vao, vbo);
    
    const char* vertexShader = R"(
        #version 330 core
        layout(location = 0) in vec2 aPos;
        layout(location = 1) in vec2 aTexCoord;
        out vec2 TexCoord;
        void main() {
            gl_Position = vec4(aPos, 0.0, 1.0);
            TexCoord = aTexCoord;
        }
    )";
    
    const char* ssaoFragment = R"(
        #version 330 core
        in vec2 TexCoord;
        out float FragColor;
        uniform sampler2D gPosition;
        uniform sampler2D gNormal;
        uniform sampler2D texNoise;
        uniform vec3 samples[64];
        uniform mat4 projection;
        uniform float radius;
        uniform float bias;
        uniform vec2 noiseScale;
        
        void main() {
            vec3 fragPos = texture(gPosition, TexCoord).xyz;
            vec3 normal = normalize(texture(gNormal, TexCoord).xyz);
            vec3 randomVec = normalize(texture(texNoise, TexCoord * noiseScale).xyz);
            
            vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
            vec3 bitangent = cross(normal, tangent);
            mat3 TBN = mat3(tangent, bitangent, normal);
            
            float occlusion = 0.0;
            for(int i = 0; i < 64; ++i) {
                vec3 samplePos = TBN * samples[i];
                samplePos = fragPos + samplePos * radius;
                
                vec4 offset = vec4(samplePos, 1.0);
                offset = projection * offset;
                offset.xyz = offset.xyz / offset.w * 0.5 + 0.5;
                
                float sampleDepth = texture(gPosition, offset.xy).z;
                float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
                occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
            }
            
            FragColor = 1.0 - (occlusion / 64.0);
        }
    )";
    
    ssaoShader = std::make_unique<Shader>();
    ssaoShader->Compile(vertexShader, ssaoFragment);
    
    const char* blurFragment = R"(
        #version 330 core
        in vec2 TexCoord;
        out float FragColor;
        uniform sampler2D ssaoInput;
        
        void main() {
            vec2 texelSize = 1.0 / vec2(textureSize(ssaoInput, 0));
            float result = 0.0;
            
            for (int x = -2; x <= 2; ++x) {
                for (int y = -2; y <= 2; ++y) {
                    vec2 offset = vec2(float(x), float(y)) * texelSize;
                    result += texture(ssaoInput, TexCoord + offset).r;
                }
            }
            
            FragColor = result / 25.0;
        }
    )";
    
    blurShader = std::make_unique<Shader>();
    blurShader->Compile(vertexShader, blurFragment);
    
    // Create SSAO FBO
    glGenFramebuffers(1, &ssaoFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
    
    glGenTextures(1, &ssaoTexture);
    glBindTexture(GL_TEXTURE_2D, ssaoTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoTexture, 0);
    
    // Create blur FBO
    glGenFramebuffers(1, &blurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, blurFBO);
    
    glGenTextures(1, &blurTexture);
    glBindTexture(GL_TEXTURE_2D, blurTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurTexture, 0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    GenerateKernel();
    GenerateNoise();
    
    Logger::Info("[PostProcess] SSAO initialized");
}

void SSAOEffect::Shutdown() {
    if (ssaoFBO) glDeleteFramebuffers(1, &ssaoFBO);
    if (ssaoTexture) glDeleteTextures(1, &ssaoTexture);
    if (blurFBO) glDeleteFramebuffers(1, &blurFBO);
    if (blurTexture) glDeleteTextures(1, &blurTexture);
    if (noiseTexture) glDeleteTextures(1, &noiseTexture);
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    
    ssaoShader.reset();
    blurShader.reset();
}

void SSAOEffect::Render(uint32_t inputTexture, uint32_t outputFBO,
                         uint32_t width, uint32_t height) {
    if (!enabled || !ssaoShader) return;
    
    // SSAO pass
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);
    
    ssaoShader->Use();
    ssaoShader->SetInt("gPosition", 0);
    ssaoShader->SetInt("gNormal", 1);
    ssaoShader->SetInt("texNoise", 2);
    ssaoShader->SetFloat("radius", radius);
    ssaoShader->SetFloat("bias", bias);
    ssaoShader->SetVec2("noiseScale", glm::vec2(width / 4.0f, height / 4.0f));
    ssaoShader->SetMat4("projection", projMatrix);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, positionTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalTexture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    
    // Upload kernel samples
    for (size_t i = 0; i < kernel.size() && i < 64; ++i) {
        ssaoShader->SetVec3("samples[" + std::to_string(i) + "]", 
            glm::vec3(kernel[i].x, kernel[i].y, kernel[i].z));
    }
    
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    // Blur pass
    glBindFramebuffer(GL_FRAMEBUFFER, blurFBO);
    glViewport(0, 0, width, height);
    
    blurShader->Use();
    blurShader->SetInt("ssaoInput", 0);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ssaoTexture);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    
    // Output is in blurTexture
    if (outputFBO != blurFBO) {
        // Copy to output
        glBindFramebuffer(GL_READ_FRAMEBUFFER, blurFBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, outputFBO);
        glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }
}

void SSAOEffect::SetGBuffer(uint32_t positionTex, uint32_t normalTex, uint32_t depthTex) {
    positionTexture = positionTex;
    normalTexture = normalTex;
    depthTexture = depthTex;
}

void SSAOEffect::GenerateKernel() {
    kernel.reserve(64);
    std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
    std::default_random_engine generator;
    
    for (uint32_t i = 0; i < 64; ++i) {
        Vec3 sample(
            randomFloats(generator) * 2.0f - 1.0f,
            randomFloats(generator) * 2.0f - 1.0f,
            randomFloats(generator)
        );
        sample = sample.Normalized() * randomFloats(generator);
        
        // Scale samples to be more aligned with center
        float scale = static_cast<float>(i) / 64.0f;
        scale = 0.1f + (scale * scale) * 0.9f;
        sample = sample * scale;
        
        kernel.push_back(sample);
    }
}

void SSAOEffect::GenerateNoise() {
    std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
    std::default_random_engine generator;
    
    std::vector<Vec3> noise;
    for (uint32_t i = 0; i < 16; ++i) {
        noise.push_back(Vec3(
            randomFloats(generator) * 2.0f - 1.0f,
            randomFloats(generator) * 2.0f - 1.0f,
            0.0f
        ));
    }
    
    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &noise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

// ============================================
// Post-Process Stack
// ============================================

PostProcessStack::PostProcessStack() = default;
PostProcessStack::~PostProcessStack() { Shutdown(); }

void PostProcessStack::Initialize(uint32_t width, uint32_t height) {
    CreateBuffers(width, height);
    
    // Add default effects
    auto toneMapping = std::make_shared<ToneMappingEffect>();
    toneMapping->Initialize(width, height);
    AddEffect(toneMapping);
    
    Logger::Info("[PostProcess] Stack initialized with " + std::to_string(effects.size()) + " effects");
}

void PostProcessStack::Shutdown() {
    for (auto& effect : effects) {
        if (effect) effect->Shutdown();
    }
    effects.clear();
    DestroyBuffers();
}

void PostProcessStack::CreateBuffers(uint32_t width, uint32_t height) {
    currentWidth = width;
    currentHeight = height;
    
    for (int i = 0; i < 2; ++i) {
        glGenFramebuffers(1, &fbo[i]);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo[i]);
        
        glGenTextures(1, &texture[i]);
        glBindTexture(GL_TEXTURE_2D, texture[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture[i], 0);
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessStack::DestroyBuffers() {
    for (int i = 0; i < 2; ++i) {
        if (fbo[i]) glDeleteFramebuffers(1, &fbo[i]);
        if (texture[i]) glDeleteTextures(1, &texture[i]);
        fbo[i] = 0;
        texture[i] = 0;
    }
}

void PostProcessStack::AddEffect(std::shared_ptr<PostProcessEffect> effect) {
    if (effect) {
        effects.push_back(effect);
    }
}

void PostProcessStack::RemoveEffect(const std::string& name) {
    effects.erase(
        std::remove_if(effects.begin(), effects.end(),
            [&name](const std::shared_ptr<PostProcessEffect>& effect) {
                return effect && effect->GetName() == name;
            }),
        effects.end()
    );
}

std::shared_ptr<PostProcessEffect> PostProcessStack::GetEffect(const std::string& name) {
    for (auto& effect : effects) {
        if (effect && effect->GetName() == name) {
            return effect;
        }
    }
    return nullptr;
}

void PostProcessStack::Render(uint32_t inputTexture, uint32_t finalOutputFBO,
                               uint32_t width, uint32_t height) {
    if (effects.empty()) {
        // No effects, just copy
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, finalOutputFBO);
        glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        return;
    }
    
    uint32_t readTex = inputTexture;
    uint32_t writeFBO = fbo[0];
    int pingPong = 0;
    
    for (size_t i = 0; i < effects.size(); ++i) {
        auto& effect = effects[i];
        if (!effect || !effect->IsEnabled()) continue;
        
        bool isLast = (i == effects.size() - 1);
        uint32_t outputFBO = isLast ? finalOutputFBO : fbo[1 -        pingPong];
        
        effect->Render(readTex, outputFBO, width, height);
        
        readTex = texture[1 - pingPong];
        pingPong = 1 - pingPong;
    }
}

void PostProcessStack::Resize(uint32_t width, uint32_t height) {
    DestroyBuffers();
    CreateBuffers(width, height);
    
    for (auto& effect : effects) {
        if (effect) {
            effect->Shutdown();
            effect->Initialize(width, height);
        }
    }
}

std::vector<std::string> PostProcessStack::GetEffectNames() const {
    std::vector<std::string> names;
    for (const auto& effect : effects) {
        if (effect) {
            names.push_back(effect->GetName());
        }
    }
    return names;
}

} // namespace vge
