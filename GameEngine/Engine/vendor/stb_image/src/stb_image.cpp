// Vendored stb_image.h v2.30 (public domain) from
// https://github.com/nothings/stb - fetched once and committed, the same
// "generate/fetch once, vendor like any other dependency" treatment as
// GLAD, since it is a single header with no build system of its own to
// integrate via FetchContent.
//
// This is the ONLY translation unit that may define
// STB_IMAGE_IMPLEMENTATION: doing so pulls in the actual function bodies,
// and doing that in more than one translation unit would violate the One
// Definition Rule.
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>
