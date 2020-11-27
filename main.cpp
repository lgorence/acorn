#include <GLFW/glfw3.h>
#include "include/gpu/gl/GrGLAssembleInterface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include <cstdio>
#include <cstdlib>
#include <ctime>

SkSurface* sSurface = nullptr;
sk_sp<GrDirectContext> sContext;

void error_callback(int error, const char* description) {
    fputs(description, stderr);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

static GrGLFuncPtr glfw_get(void *ctx, const char name[]) {
    SkASSERT(nullptr == ctx);
    SkASSERT(glfwGetCurrentContext());
    return glfwGetProcAddress(name);
}

void init_skia(int w, int h) {
    SkASSERT(glfwGetCurrentContext() != nullptr);

    auto interface = GrGLMakeAssembledInterface(nullptr, glfw_get);

    sContext = GrDirectContext::MakeGL(interface);
    SkASSERT(sContext);

    GrGLFramebufferInfo framebufferInfo;
    framebufferInfo.fFBOID = 0; // assume default framebuffer
    // We are always using OpenGL and we use RGBA8 internal format for both RGBA and BGRA configs in OpenGL.
    //(replace line below with this one to enable correct color spaces) framebufferInfo.fFormat = GL_SRGB8_ALPHA8;
    framebufferInfo.fFormat = GL_RGBA8;

    SkColorType colorType = kBGRA_8888_SkColorType;
    GrBackendRenderTarget backendRenderTarget(w, h,
                                              0, // sample count
                                              0, // stencil bits
                                              framebufferInfo);

    //(replace line below with this one to enable correct color spaces) sSurface = SkSurface::MakeFromBackendRenderTarget(sContext, backendRenderTarget, kBottomLeft_GrSurfaceOrigin, colorType, SkColorSpace::MakeSRGB(), nullptr).release();
    sSurface = SkSurface::MakeFromBackendRenderTarget(sContext.get(), backendRenderTarget, kBottomLeft_GrSurfaceOrigin, colorType, nullptr, nullptr).release();
    if (sSurface == nullptr) abort();
}

void cleanup_skia() {
    delete sSurface;
    sContext.release();
}

int kWidth = 720;
int kHeight = 150;

int main(void) {
    GLFWwindow* window;
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //(uncomment to enable correct color spaces) glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);
    glfwWindowHint(GLFW_STENCIL_BITS, 0);
    //glfwWindowHint(GLFW_ALPHA_BITS, GLFW_TRUE);
    glfwWindowHint(GLFW_DEPTH_BITS, 0);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_FOCUSED, GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    auto vmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    kWidth = vmode->width;
    kHeight = vmode->height * 0.03f;
    printf("kHeight = %d\n", kHeight);

    window = glfwCreateWindow(kWidth, kHeight, "Simple example", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    //(uncomment to enable correct color spaces) glEnable(GL_FRAMEBUFFER_SRGB);

    init_skia(kWidth, kHeight);

    glfwSwapInterval(1);
    glfwSetKeyCallback(window, key_callback);

    // Draw to the surface via its SkCanvas.
    SkCanvas* canvas = sSurface->getCanvas(); // We don't manage this pointer's lifetime.

    using namespace skia::textlayout;

    TextStyle ts;
    ts.setFontSize(kHeight * 0.5);
    ts.setColor(SK_ColorWHITE);

    auto fcp = new FontCollection();
    fcp->setDefaultFontManager(SkFontMgr::RefDefault());

    auto fc = sk_sp(fcp);
    ParagraphStyle ps;
    ps.setTextAlign(TextAlign::kCenter);
    ps.setTextHeightBehavior(TextHeightBehavior::kAll);

    char timeStr[64];
    const std::string am = "AM";
    const std::string pm = "PM";
    while (!glfwWindowShouldClose(window)) {
        glfwWaitEvents();

        SkPaint paint;
        paint.setColor(SK_ColorBLACK);
        canvas->drawPaint(paint);

        paint.setColor(SK_ColorWHITE);

        auto t = time(nullptr);
        auto lt = localtime(&t);

        auto pb = ParagraphBuilderImpl(ps, fc);
        pb.pushStyle(ts);
        auto period = lt->tm_hour >= 12 ? pm : am;
        snprintf(timeStr, 64, "%d:%02d:%02d %s", lt->tm_hour % 12, lt->tm_min, lt->tm_sec, period.c_str());
        pb.addText(timeStr);

        auto p = pb.Build();
        p->layout(kWidth);

        int yRender = kHeight / 2 - (p->getHeight() / 2);
        p->paint(canvas, 0, yRender);

        sContext->flush();

        glfwSwapBuffers(window);
    }

    cleanup_skia();

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
