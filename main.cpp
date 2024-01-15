#include <gfd.h>
#include <string.h>
#include <stdio.h>
#include <whb/file.h>
#include <whb/proc.h>
#include <whb/sdcard.h>
#include <whb/gfx.h>
#include <whb/log.h>
#include <whb/log_cafe.h>
#include <cstdlib>

#include <coreinit/systeminfo.h>
#include <coreinit/thread.h>
#include <coreinit/time.h>
#include <coreinit/filesystem.h>
#include <coreinit/memheap.h>

#include <whb/log_udp.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "CafeGLSLCompiler.h"

extern "C" void __init_wut_malloc();
extern "C" void __preinit_user(MEMHeapHandle *outMem1,
                               MEMHeapHandle *outFG,
                               MEMHeapHandle *outMem2)
{
   __init_wut_malloc();
}

extern "C" bool MEMCheckHeap(void *heap, uint32_t unknown);

WHBGfxShaderGroup *GLSL_CompileShader(const char *vsSrc, const char *psSrc)
{
   char infoLog[1024];
   GX2VertexShader *vs = GLSL_CompileVertexShader(vsSrc, infoLog, sizeof(infoLog), GLSL_COMPILER_FLAG_NONE);
   if (!vs)
   {
      WHBLogPrintf("Failed to compile vertex shader. Infolog: %s", infoLog);
      return NULL;
   }
   GX2PixelShader *ps = GLSL_CompilePixelShader(psSrc, infoLog, sizeof(infoLog), GLSL_COMPILER_FLAG_NONE);
   if (!ps)
   {
      WHBLogPrintf("Failed to compile pixel shader. Infolog: %s", infoLog);
      return NULL;
   }
   WHBGfxShaderGroup *shaderGroup = (WHBGfxShaderGroup *)malloc(sizeof(WHBGfxShaderGroup));
   memset(shaderGroup, 0, sizeof(*shaderGroup));
   shaderGroup->vertexShader = vs;
   shaderGroup->pixelShader = ps;
   return shaderGroup;
}

void loadShader(const char *filename, std::string &destination)
{
   // This way, using the deprecated WHB functions, doesn't crash on exit:

   // char *sdRootPath = WHBGetSdCardMountPath();
   // char path[256];
   // sprintf(path, "%s/%s", sdRootPath, filename);
   // WHBLogPrintf("Loading shader %s", path);
   // char *text = WHBReadWholeFile(path, NULL);
   // destination = text;


   // This way, using C++ ifstream, crashes on exit:
   // (also using C standard lib does the same)

   std::ifstream inVert;
   std::ostringstream inStreamVert;

   WHBLogPrintf("Loading shader %s",filename);
   inVert.open(filename);
   inStreamVert << inVert.rdbuf();
   inVert.close();
   destination.assign(inStreamVert.str());
}

int main(int argc, char **argv)
{
   WHBLogCafeInit();
   WHBLogUdpInit();
   WHBProcInit();
   WHBMountSdCard();
   WHBGfxInit();
   GLSL_Init();
   WHBLogPrint("Hello World! Logging initialised.");
   {
      WHBGfxShaderGroup *group;
      std::string inStringVert;
      loadShader("shaders/projected.vert", inStringVert);
      const char *vertexShader = inStringVert.c_str();

      std::string inStringFrag;
      loadShader("shaders/textured.frag", inStringFrag);
      const char *fragmentShader = inStringFrag.c_str();

      group = GLSL_CompileShader(vertexShader, fragmentShader);
      
      while (WHBProcIsRunning())
      {
         WHBGfxBeginRender();
         WHBGfxBeginRenderTV();
         WHBGfxClearColor(1.0f, 0.0f, 1.0f, 1.0f);
         WHBGfxFinishRenderTV();

         WHBGfxBeginRenderDRC();
         WHBGfxClearColor(1.0f, 0.0f, 1.0f, 1.0f);
         WHBGfxFinishRenderDRC();
         WHBGfxFinishRender();
      }

      // I don't think this is related, it behaves the same either way:
      WHBGfxFreeShaderGroup(group);
      // or:
      // GLSL_FreeVertexShader(group->vertexShader);
      // GLSL_FreePixelShader(group->pixelShader);
      // free(group);
   }
   WHBLogPrintf("Done. Quitting...");
   WHBGfxShutdown();
   WHBUnmountSdCard();

   // Check heap on exit
   MEMHeapHandle heap = MEMGetBaseHeapHandle(MEM_BASE_HEAP_MEM2);
   bool check = MEMCheckHeap(heap, 1);
   WHBLogPrintf("Heap check %d", check);

   WHBProcShutdown();
   WHBLogUdpDeinit();
   WHBLogCafeDeinit();
   return 0;
}
