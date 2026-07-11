#include <gst/gst.h>
#include "Pipeline.h"

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

int pipeline_main(int argc, char *argv[])
{
    Pipeline pipeline(argc, argv);
    return pipeline.run();
}

int main(int argc, char *argv[])
{
#if defined(__APPLE__) && TARGET_OS_MAC && !TARGET_OS_IPHONE
    return gst_macos_main((GstMainFunc)pipeline_main, argc, argv, NULL);
#else
    return pipeline_main(argc, argv);
#endif
}
