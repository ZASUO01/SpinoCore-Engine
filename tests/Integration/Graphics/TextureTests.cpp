// SPDX-License-Identifier: MIT
// Copyright (c) 2026 ZASUO01
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <ios>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>
#include <gtest/gtest.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_image/SDL_image.h>
#include <glad/glad.h>
#include "GLTestContext.h"
#include "Graphics/Resources/Texture.h"

namespace {
    namespace fs = std::filesystem;
    using SpinoCore::Graphics::Resources::Texture;
    using SpinoCore::Tests::Pixel;

    class TempDirectory final {
    public:
        TempDirectory() {
            std::random_device rd;
            mPath = fs::temp_directory_path() / ("spinocore-texture-" + std::to_string(rd()) + std::to_string(rd()));
            fs::create_directories(mPath);
        }
        ~TempDirectory() {
            std::error_code ignored;
            fs::remove_all(mPath, ignored);
        }
        TempDirectory(const TempDirectory&) = delete;
        TempDirectory& operator=(const TempDirectory&) = delete;

        [[nodiscard]] const fs::path& Path() const { return mPath; }

    private:
        fs::path mPath;
    };

    struct Image final {
        int width;
        int height;
        std::vector<Pixel> pixels;

        [[nodiscard]] Pixel At(const int x, const int y) const {
            return pixels[(static_cast<std::size_t>(y) * static_cast<std::size_t>(width)) + static_cast<std::size_t>(x)];
        }
    };

    [[nodiscard]] Image MakeImage(const int width, const int height, const bool translucent) {
        Image image{width, height, {}};
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                image.pixels.push_back({static_cast<std::uint8_t>(20 + 40 * x), static_cast<std::uint8_t>(30 + 60 * y),
                                        static_cast<std::uint8_t>(200 - 10 * x - 20 * y),
                                        static_cast<std::uint8_t>(translucent ? 100 + 30 * x : 255)});
            }
        }
        return image;
    }

    [[nodiscard]] SDL_Surface* ToSurface(const Image& image, const SDL_PixelFormat format) {
        SDL_Surface* surface = SDL_CreateSurface(image.width, image.height, format);
        if (!surface) return nullptr;
        for (int y = 0; y < image.height; ++y) {
            for (int x = 0; x < image.width; ++x) {
                const auto p = image.At(x, y);
                SDL_WriteSurfacePixel(surface, x, y, p[0], p[1], p[2], p[3]);
            }
        }
        return surface;
    }

    [[nodiscard]] bool SavePNG(const Image& image, const fs::path& path) {
        SDL_Surface* surface = ToSurface(image, SDL_PIXELFORMAT_RGBA32);
        const auto utf8 = path.u8string();
        const bool ok = surface && IMG_SavePNG(surface, reinterpret_cast<const char*>(utf8.c_str()));
        SDL_DestroySurface(surface);
        return ok;
    }

    [[nodiscard]] bool SaveRGB24BMP(const Image& image, const fs::path& path) {
        SDL_Surface* surface = ToSurface(image, SDL_PIXELFORMAT_RGB24);
        const auto utf8 = path.u8string();
        const bool ok = surface && SDL_SaveBMP(surface, reinterpret_cast<const char*>(utf8.c_str()));
        SDL_DestroySurface(surface);
        return ok;
    }

    void WriteBytes(const fs::path& path, const std::string_view bytes) {
        std::ofstream out(path, std::ios::binary);
        out << bytes;
    }

    [[nodiscard]] std::vector<Pixel> ReadTexture(const Texture& texture) {
        const auto size = texture.GetSize();
        std::vector<Pixel> pixels(static_cast<std::size_t>(size.width * size.height));
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glGetTextureImage(texture.GetID(), 0, GL_RGBA, GL_UNSIGNED_BYTE,
                          static_cast<GLsizei>(pixels.size() * sizeof(Pixel)), pixels.data());
        glPixelStorei(GL_PACK_ALIGNMENT, 4);
        return pixels;
    }

    [[nodiscard]] GLint GetInteger(const GLenum name) {
        GLint value = 0;
        glGetIntegerv(name, &value);
        return value;
    }

    class TextureTests : public SpinoCore::Tests::GLTestContext {
    protected:
        void ExpectSameContent(const Texture& texture, const Image& image, const bool forceOpaque) const {
            ASSERT_EQ(texture.GetSize().width, image.width);
            ASSERT_EQ(texture.GetSize().height, image.height);
            const auto pixels = ReadTexture(texture);
            for (int y = 0; y < image.height; ++y) {
                for (int x = 0; x < image.width; ++x) {
                    auto expected = image.At(x, y);
                    if (forceOpaque) expected[3] = 255;
                    EXPECT_EQ(pixels[static_cast<std::size_t>(y * image.width + x)], expected) << "at (" << x << "," << y << ")";
                }
            }
        }

        TempDirectory dir;
    };
}

TEST_F(TextureTests, LoadsAPngWithExactPixelsAndSize) {
    const auto image = MakeImage(3, 2, true);
    const auto path = dir.Path() / "rgba.png";
    ASSERT_TRUE(SavePNG(image, path)) << SDL_GetError();

    const Texture texture{path};

    EXPECT_NE(texture.GetID(), 0u);
    EXPECT_TRUE(glIsTexture(texture.GetID()));
    ExpectSameContent(texture, image, false);
    EXPECT_EQ(glGetError(), static_cast<GLenum>(GL_NO_ERROR));
}

TEST_F(TextureTests, FirstTextureRowIsTheTopOfTheImage) {
    const Image image{1, 2, {{255, 0, 0, 255}, {0, 0, 255, 255}}};
    const auto path = dir.Path() / "top-red.png";
    ASSERT_TRUE(SavePNG(image, path)) << SDL_GetError();

    const Texture texture{path};
    const auto pixels = ReadTexture(texture);

    EXPECT_EQ(pixels[0], (Pixel{255, 0, 0, 255}));
    EXPECT_EQ(pixels[1], (Pixel{0, 0, 255, 255}));
}

TEST_F(TextureTests, OddWidthsKeepEveryRowAligned) {
    const auto image = MakeImage(5, 3, true);
    const auto path = dir.Path() / "odd.png";
    ASSERT_TRUE(SavePNG(image, path)) << SDL_GetError();

    const Texture texture{path};
    ExpectSameContent(texture, image, false);
}

TEST_F(TextureTests, ConvertsRgbImagesToOpaqueRgba) {

    const auto image = MakeImage(3, 3, false);
    const auto path = dir.Path() / "rgb24.bmp";
    ASSERT_TRUE(SaveRGB24BMP(image, path)) << SDL_GetError();

    const Texture texture{path};
    ExpectSameContent(texture, image, true);
}

TEST_F(TextureTests, LoadsPathsWithNonAsciiCharacters) {
    const auto image = MakeImage(2, 2, false);
    const auto path = dir.Path() / fs::path(u8"ação.png");
    ASSERT_TRUE(SavePNG(image, path)) << SDL_GetError();

    const Texture texture{path};
    ExpectSameContent(texture, image, false);
}

TEST_F(TextureTests, UsesLinearFiltering) {
    const auto path = dir.Path() / "filters.png";
    ASSERT_TRUE(SavePNG(MakeImage(2, 2, false), path)) << SDL_GetError();
    const Texture texture{path};

    GLint minFilter = 0;
    GLint magFilter = 0;
    glGetTextureParameteriv(texture.GetID(), GL_TEXTURE_MIN_FILTER, &minFilter);
    glGetTextureParameteriv(texture.GetID(), GL_TEXTURE_MAG_FILTER, &magFilter);
    EXPECT_EQ(minFilter, GL_LINEAR);
    EXPECT_EQ(magFilter, GL_LINEAR);
}

TEST_F(TextureTests, ClampsToEdgeOnBothAxes) {
    const auto path = dir.Path() / "wrap.png";
    ASSERT_TRUE(SavePNG(MakeImage(2, 2, false), path)) << SDL_GetError();
    const Texture texture{path};

    GLint wrapS = 0;
    GLint wrapT = 0;
    glGetTextureParameteriv(texture.GetID(), GL_TEXTURE_WRAP_S, &wrapS);
    glGetTextureParameteriv(texture.GetID(), GL_TEXTURE_WRAP_T, &wrapT);
    EXPECT_EQ(wrapS, GL_CLAMP_TO_EDGE);
    EXPECT_EQ(wrapT, GL_CLAMP_TO_EDGE);
}

TEST_F(TextureTests, StoresEightBitsPerChannel) {
    const auto path = dir.Path() / "format.png";
    ASSERT_TRUE(SavePNG(MakeImage(2, 2, false), path)) << SDL_GetError();
    const Texture texture{path};

    GLint internalFormat = 0;
    glGetTextureLevelParameteriv(texture.GetID(), 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);
    EXPECT_EQ(internalFormat, GL_RGBA8);
}

TEST_F(TextureTests, RestoresUnpackStateAndBinding) {
    const auto path = dir.Path() / "state.png";
    ASSERT_TRUE(SavePNG(MakeImage(3, 2, false), path)) << SDL_GetError();

    const Texture texture{path};

    EXPECT_EQ(GetInteger(GL_UNPACK_ROW_LENGTH), 0);
    EXPECT_EQ(GetInteger(GL_UNPACK_ALIGNMENT), 4);
    EXPECT_EQ(GetInteger(GL_TEXTURE_BINDING_2D), 0);
}

TEST_F(TextureTests, ErrorsLeftByEarlierGLCallsDoNotFailTheLoad) {
    const auto path = dir.Path() / "pending-error.png";
    ASSERT_TRUE(SavePNG(MakeImage(2, 2, false), path)) << SDL_GetError();

    glEnable(0xFFFF);
    EXPECT_NO_THROW({ const Texture texture{path}; });
}

TEST_F(TextureTests, SetActiveBindsToTheGivenUnit) {
    const auto path = dir.Path() / "slot.png";
    ASSERT_TRUE(SavePNG(MakeImage(2, 2, false), path)) << SDL_GetError();
    const Texture texture{path};

    texture.SetActive(2u);

    EXPECT_EQ(GetInteger(GL_ACTIVE_TEXTURE), GL_TEXTURE2);
    EXPECT_EQ(GetInteger(GL_TEXTURE_BINDING_2D), static_cast<GLint>(texture.GetID()));

    glActiveTexture(GL_TEXTURE0);
    EXPECT_EQ(GetInteger(GL_TEXTURE_BINDING_2D), 0);
}

TEST_F(TextureTests, DestructorReleasesTheGLTexture) {
    const auto path = dir.Path() / "release.png";
    ASSERT_TRUE(SavePNG(MakeImage(2, 2, false), path)) << SDL_GetError();

    GLuint id = 0;
    {
        const Texture texture{path};
        id = texture.GetID();
        ASSERT_TRUE(glIsTexture(id));
    }
    EXPECT_FALSE(glIsTexture(id));
}

TEST_F(TextureTests, EachTextureGetsItsOwnId) {
    const auto path = dir.Path() / "shared.png";
    ASSERT_TRUE(SavePNG(MakeImage(2, 2, false), path)) << SDL_GetError();

    const Texture first{path};
    const Texture second{path};
    EXPECT_NE(first.GetID(), second.GetID());
}

TEST_F(TextureTests, MissingFileThrows) {
    EXPECT_THROW({ const Texture texture{dir.Path() / "missing.png"}; }, std::runtime_error);
}

TEST_F(TextureTests, FileThatIsNotAnImageThrows) {
    const auto path = dir.Path() / "fake.png";
    WriteBytes(path, "this is not a png");
    EXPECT_THROW({ const Texture texture{path}; }, std::runtime_error);
}

TEST_F(TextureTests, EmptyFileThrows) {
    const auto path = dir.Path() / "empty.png";
    WriteBytes(path, "");
    EXPECT_THROW({ const Texture texture{path}; }, std::runtime_error);
}

TEST_F(TextureTests, FailedLoadLeavesNoGLErrorBehind) {
    EXPECT_THROW({ const Texture texture{dir.Path() / "missing.png"}; }, std::runtime_error);
    EXPECT_EQ(glGetError(), static_cast<GLenum>(GL_NO_ERROR));
}

TEST_F(TextureTests, BlankTextureIsAValidGLTexture) {
    const auto texture = std::make_unique<Texture>();

    EXPECT_TRUE(glIsTexture(texture->GetID()));
    EXPECT_EQ(glGetError(), static_cast<GLenum>(GL_NO_ERROR));
}

TEST_F(TextureTests, BlankTextureIsASinglePixel) {
    const auto texture = std::make_unique<Texture>();

    EXPECT_EQ(texture->GetSize().width, 1);
    EXPECT_EQ(texture->GetSize().height, 1);
}

TEST_F(TextureTests, BlankTextureIsOpaqueWhite) {
    const auto texture = std::make_unique<Texture>();

    const auto pixels = ReadTexture(*texture);

    ASSERT_EQ(pixels.size(), 1u);
    EXPECT_EQ(pixels[0], (Pixel{255, 255, 255, 255}));
}

TEST_F(TextureTests, BlankTextureUsesTheSameParametersAsLoadedOnes) {
    const auto texture = std::make_unique<Texture>();

    GLint internalFormat = 0;
    GLint minFilter = 0;
    GLint magFilter = 0;
    GLint wrapS = 0;
    GLint wrapT = 0;
    glGetTextureLevelParameteriv(texture->GetID(), 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);
    glGetTextureParameteriv(texture->GetID(), GL_TEXTURE_MIN_FILTER, &minFilter);
    glGetTextureParameteriv(texture->GetID(), GL_TEXTURE_MAG_FILTER, &magFilter);
    glGetTextureParameteriv(texture->GetID(), GL_TEXTURE_WRAP_S, &wrapS);
    glGetTextureParameteriv(texture->GetID(), GL_TEXTURE_WRAP_T, &wrapT);

    EXPECT_EQ(internalFormat, GL_RGBA8);
    EXPECT_EQ(minFilter, GL_LINEAR);
    EXPECT_EQ(magFilter, GL_LINEAR);
    EXPECT_EQ(wrapS, GL_CLAMP_TO_EDGE);
    EXPECT_EQ(wrapT, GL_CLAMP_TO_EDGE);
}

TEST_F(TextureTests, BlankTextureRestoresUnpackStateAndBinding) {
    const auto texture = std::make_unique<Texture>();

    EXPECT_EQ(GetInteger(GL_UNPACK_ROW_LENGTH), 0);
    EXPECT_EQ(GetInteger(GL_UNPACK_ALIGNMENT), 4);
    EXPECT_EQ(GetInteger(GL_TEXTURE_BINDING_2D), 0);
}

TEST_F(TextureTests, EachBlankTextureIsIndependent) {
    const auto first = std::make_unique<Texture>();
    const GLuint firstId = first->GetID();
    {
        const auto second = std::make_unique<Texture>();
        EXPECT_NE(second->GetID(), firstId);
    }
    EXPECT_TRUE(glIsTexture(firstId));
}

TEST_F(TextureTests, BlankTextureIsReleasedWithItsOwner) {
    GLuint id = 0;
    {
        const auto texture = std::make_unique<Texture>();
        id = texture->GetID();
        ASSERT_TRUE(glIsTexture(id));
    }
    EXPECT_FALSE(glIsTexture(id));
}

TEST_F(TextureTests, CopiesAnRgbaSurfaceWithExactPixelsAndSize) {
    const auto image = MakeImage(3, 2, true);
    SDL_Surface* surface = ToSurface(image, SDL_PIXELFORMAT_RGBA32);
    ASSERT_NE(surface, nullptr) << SDL_GetError();

    const Texture texture{surface};
    SDL_DestroySurface(surface);

    ExpectSameContent(texture, image, false);
}

TEST_F(TextureTests, ConvertsSurfacesInOtherPixelFormats) {
    const auto image = MakeImage(3, 2, true);
    SDL_Surface* surface = ToSurface(image, SDL_PIXELFORMAT_ARGB8888);
    ASSERT_NE(surface, nullptr) << SDL_GetError();

    const Texture texture{surface};
    SDL_DestroySurface(surface);

    ExpectSameContent(texture, image, false);
}

TEST_F(TextureTests, NullSurfaceThrows) {
    EXPECT_THROW({ const Texture texture{static_cast<SDL_Surface*>(nullptr)}; }, std::invalid_argument);
}
